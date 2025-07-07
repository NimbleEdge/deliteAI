/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "log_sender.hpp"

#include <dirent.h>

#include <cstdio>

#include "native_interface.hpp"
#include "server_api.hpp"

LogSender::LogSender(std::shared_ptr<ServerAPI> serverAPI, std::shared_ptr<const Config> config,
                     std::shared_ptr<Logger> loggerToControl,
                     const LogSendingConfig& senderConfig) {
  if (senderConfig.valid == true) {
    _sender_config = senderConfig;
  }
  _serverAPI = serverAPI;
  _config = config;
  _mappedLogger = loggerToControl;
  concurrentLogFailures = 0;
}

std::vector<std::string> LogSender::get_files_in_sorted_order(std::shared_ptr<Logger> loggerInUse) {
  std::string logDirectory = loggerInUse->get_directory();
  std::string logFileName = "";
  std::vector<std::string> logFiles;
  DIR* logDir;
  struct dirent* dirContent;
  logDir = opendir(logDirectory.c_str());
  if (logDir != NULL) {
    while ((dirContent = readdir(logDir)) != NULL) {
      if (dirContent->d_type == DT_REG && strcmp(dirContent->d_name, "latest.txt")) {
        logFileName = std::string(dirContent->d_name);
        logFiles.push_back(logDirectory + "/" + logFileName);
      }
    }
    closedir(logDir);

    if (logFiles.empty() && _sender_config.sendFirstLog &&
        logDirectory.find("logs") != std::string::npos) {
      // no other file apart from latest.txt is present (excluding filesToIgnore)
      if (!_breakingFileForFirstTime) {
        return {};
      }
      auto newFileName = loggerInUse->take_lock_and_break_current_file();
      if (newFileName == "") {
        return {};
      }
      _breakingFileForFirstTime = false;
      return {newFileName};
    }
  } else {
    LOG_TO_ERROR("Cannot open the directory %s to read logs", logDirectory.c_str());
  }

  std::sort(logFiles.begin(), logFiles.end());
  return logFiles;
}

void LogSender::update_sender_config(const LogSendingConfig& sendConfig) {
  _sender_config = sendConfig;
}

bool LogSender::send_logs(const std::vector<std::string>& logfiles) {
  if (!_sender_config.valid) return false;
  std::string logBody = "";
  std::vector<bool> logfileRemove(logfiles.size(), false);
  for (int i = 0; i < logfiles.size(); i++) {
    const std::string& logfilePath = logfiles[i];
    auto ret = nativeinterface::read_log_file(logfilePath);
    if (!ret.first) {
      LOG_TO_ERROR("%s Logfile could not be read from the device.", logfilePath.c_str());
      continue;
    }
    std::string logs = ret.second;
    if (logs.empty()) {
      try {
        int didRemove = remove(logfilePath.c_str());
        if (didRemove) {
          LOG_TO_ERROR("%s could not be removed from the system. Failed with error %d",
                       logfilePath.c_str(), didRemove);
          logfileRemove[i] = true;
        }
      } catch (...) {
        LOG_TO_ERROR("%s Logfile could not be removed from the device.", logfilePath.c_str());
      }
      continue;
    }
    logBody.append(logs);
    logfileRemove[i] = true;
  }
  // DO NOT REMOVE OR CHANGE THE FIELDS OF HEADER WITHOUT CONSULTING
  json header = json::array({json{{"Content-Type", "text/plain"},
                                  {"Secret-Key", _sender_config._secretKey},
                                  {"Accept", "application/json"},
                                  {"service", _service},
                                  {"clientId", _config->clientId},
                                  {"ddsource", _source},
                                  {"ddtags", _sdkVersion},
                                  {"deviceID", _config->deviceId},
                                  {"compatibilityTag", _config->compatibilityTag},
                                  {"internalDeviceId", _config->internalDeviceId}}});

  // removing unreadable characters from file to be uploaded
  for (int i = 0; i < logBody.size(); i++) {
    if ((logBody[i] < 32 || logBody[i] > 126) && logBody[i] != '\n') {
      logBody[i] = '?';
    }
  }
  bool didSend = _serverAPI->upload_logs(LogRequestBody(header, logBody, _sender_config._host));

  if (didSend) {
    concurrentLogFailures = 0;
    for (int i = 0; i < logfiles.size(); i++) {
      if (logfileRemove[i]) {
        std::string logfilePath = logfiles[i];
        try {
          int didRemove = remove(logfilePath.c_str());
          if (didRemove) {
            LOG_TO_ERROR("%s could not be removed from the system. Failed with error %d",
                         logfilePath.c_str(), didRemove);
          }
        } catch (...) {
          LOG_TO_ERROR("%s Logfile could not be removed from the device.", logfilePath.c_str());
        }
      }
    }
  } else {
    concurrentLogFailures++;
  }

  _mappedLogger->recompute_disk_size();
  return didSend;
}

bool LogSender::should_send_logs() {
  auto probabilityToSend = _sender_config.sendLogsProbability;
  float randomNumber = static_cast<float>(std::rand()) / float(RAND_MAX);
  if (randomNumber < probabilityToSend)
    return true;
  else
    return false;
}

std::vector<std::string> LogSender::get_log_files_to_send() {
  auto logFilesInOrder = get_files_in_sorted_order(_mappedLogger);

  logFilesInOrder.resize(std::min((int)logFilesInOrder.size(), _sender_config.maxFilesToSend));

  return logFilesInOrder;
}

bool LogSender::send_all_logs() {
  if (!_sender_config.valid) return false;
  if (!_senderMutex.try_lock()) {
    return false;
  }
  std::lock_guard<std::mutex> locker(_senderMutex, std::adopt_lock);
  while (true) {
    std::vector<std::string> logfiles = get_log_files_to_send();
    // Case when there is no log file apart from latest.txt in the logs directory
    if (logfiles.empty()) {
      // all files sent
      return true;
    }
    if (!send_logs(logfiles)) {
      // log sending failed means there is some network issue, should not keep retrying.
      return false;
    }
  }
}

void LogSender::send_pending_logs() {
#ifndef SENDLOGS
  return;
#endif
#ifdef SIMULATION_MODE
  return;
#endif
  if (!_sender_config.valid) return;
  if (!_senderMutex.try_lock()) {
    return;
  }
  std::lock_guard<std::mutex> locker(_senderMutex, std::adopt_lock);
  auto elapsedTimeSecs = Time::get_elapsed_time_in_sec(_lastSendTime);
  if (elapsedTimeSecs > _sender_config._timer_interval &&
      concurrentLogFailures <= _sender_config.maxConcurrentLogFailures) {
    if (should_send_logs()) {
      std::vector<std::string> logfiles = get_log_files_to_send();
      if (!logfiles.empty()) {
        send_logs(logfiles);
      }
    }
    _lastSendTime = Time::get_high_resolution_clock_time();
  }
}
