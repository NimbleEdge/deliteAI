/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logger.hpp"

#include <string>

#include "client.h"
#include "native_interface.hpp"
#include "time_manager.hpp"
#include "util.hpp"

ne::AtomicPtr<std::string> Logger::sessionId = ne::AtomicPtr<std::string>("");

void Logger::write_log(const char* message, const char* type, const std::string& currentDate) {
#ifdef SIMULATION_MODE
  return;
#endif
  // Log type should always come first
  auto logLine = ne::fmt("%s::: %s ::: %s\n", type, currentDate.c_str(), message);
  util::encrypt_data(logLine.str, strlen(logLine.str));

  std::unique_lock<std::mutex> lock(_logMutex);
  if (_writeFilePtr == NULL) return;

  fprintf(_writeFilePtr, "%s", logLine.str);
  fflush(_writeFilePtr);
  long size = ftell(_writeFilePtr);

  if ((float)size / (loggerconstants::MaxBytesInKB) > _atomicLogConfig.load()->maxLogFileSizeKB) {
    std::string newFileName = _logDirectory + "/log" + currentDate + ".txt";
    break_current_file(newFileName, std::move(lock));
  }
}

bool Logger::init_logger(const std::string& logDir) {
#ifdef SIMULATION_MODE
  return true;
#endif
  std::unique_lock<std::mutex> uniqueLock(_logMutex);
  if (_writeFilePtr) return true;
  _logDirectory = logDir;
  try {
    DIR* dir = opendir(_logDirectory.c_str());
    if (!dir) {
      if (mkdir(_logDirectory.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        log_fatal("Unable to create directory to write logs for nimbleSDK");
        // Unable to create directory to push logs.
        return false;
      }
    } else {
      struct dirent* entry;
      while ((entry = readdir(dir)) != nullptr) {
        // Get the name of the file
        std::string filename = entry->d_name;
        if (filename == "." || filename == "..")  // Skip current and parent directory entries
          continue;

        // Construct full path
        std::string fullpath = _logDirectory + filename;

        _dirSize += ne::get_file_size(fullpath);
      }
      closedir(dir);
    }
  } catch (...) {
    log_fatal("Unable to check for directory to write logs for nimbleSDK with exception");
    return false;
  }

  _writeFile = _logDirectory + "/latest.txt";
  _writeFilePtr = fopen(_writeFile.c_str(), "a+");

  if (_writeFilePtr == NULL) {
    log_fatal("Unable to create file to write logs for nimbleSDK");
    // Unable to open a new file to write logs
    return false;
  }
  fseek(_writeFilePtr, 0, SEEK_END);
  return true;
}

void Logger::update_log_config(const LogWritingConfig& config) {
  _atomicLogConfig.store(std::make_shared<LogWritingConfig>(config));

  for (auto [key, value] : _atomicLogConfig.load()->logTypesToWrite) {
    if (key == "info" && value == false) {
      _logVerbose = false;
    }
    if (key == "warning" && value == false) {
      _logWarning = false;
    }
    if (key == "error" && value == false) {
      _logError = false;
    }
  }
}

std::string Logger::take_lock_and_break_current_file() {
  std::unique_lock<std::mutex> uniqueLock(_logMutex);
  std::string currentDate = Time::get_date_UTC();
  currentDate.erase(std::remove(currentDate.begin(), currentDate.end(), ' '), currentDate.end());
  return break_current_file(_logDirectory + "/log" + currentDate, std::move(uniqueLock));
}

// IMPORTANT: This assumes that logMutexUniqueLock is currently locked, and unlocks it when not
// needed. The lock will be unlocked when the function returns
std::string Logger::break_current_file(std::string newFileName,
                                       std::unique_lock<std::mutex>&& logMutexUniqueLock) {
  if (ftell(_writeFilePtr) < 10) {
    // not breaking file if file is empty
    return "";
  }

  fclose(_writeFilePtr);
  std::string tmpFileName = newFileName + ".txt";
  rename(_writeFile.c_str(), tmpFileName.c_str());
  _writeFilePtr = fopen(_writeFile.c_str(), "a+");
  fseek(_writeFilePtr, 0, SEEK_END);

  logMutexUniqueLock.unlock();

  // Compress the cut file
  if (!nativeinterface::compress_file(tmpFileName.c_str(), newFileName.c_str())) {
    // If we can't compress then we should just save the uncompressed file
    rename(tmpFileName.c_str(), newFileName.c_str());
  } else {
    remove(tmpFileName.c_str());
  }

  // Update net size of directory
  _dirSize += ne::get_file_size(newFileName);

  return newFileName;
}

void Logger::script_log(int deploymentId, const char* metricType, const char* metricJsonString) {
  if (_dirSize >= _maxDirSize) {
    LOG_TO_CLIENT_ERROR(
        "Could not send script logs as current directory size=%d is more than permited size=%d",
        _dirSize.load(), _maxDirSize);
    return;
  }
  auto logConfig = _atomicLogConfig.load();
  if (!logConfig->scriptVerbose) return;
  if (!logConfig->collectEvents) return;
  auto buf = ne::fmt("%d::: %s ::: %s", deploymentId, metricType, metricJsonString);

  write_log(buf.str, "SCRIPTLOGS", Time::get_date_UTC());
}

bool Logger::event_log(const char* eventType, const char* rawEventJsonString) {
  auto logConfig = _atomicLogConfig.load();
  auto found = logConfig->eventTypesToWrite.find(eventType);
  if (found == logConfig->eventTypesToWrite.end() || found->second == false) {
    return false;
  }
  if (!logConfig->collectEvents) {
    return true;
  }
  if (_dirSize >= _maxDirSize) {
    LOG_TO_CLIENT_ERROR(
        "Could not send events as current directory size=%d is more than permited size=%d",
        _dirSize.load(), _maxDirSize);
    return true;
  }

  auto buf = ne::fmt("%s ::: %s ::: %s", sessionId.load()->c_str(), eventType, rawEventJsonString);
  write_log(buf.str, "EVENTS");
  return true;
}
