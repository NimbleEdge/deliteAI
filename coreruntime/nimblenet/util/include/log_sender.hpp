/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <chrono>
#include <fstream>
#include <vector>

#include "client.h"
#include "config_manager.hpp"
#include "logger_constants.hpp"
#include "util.hpp"

/**
 * @brief Configuration for sending logs to logger service.
 */
struct LogSendingConfig {
  std::string _host;                  /**< Host URL for log server. */
  std::string _secretKey;             /**< Secret key for authentication. */
  std::string _defaultSecretKey;      /**< Default secret key if none provided. */
  int maxConcurrentLogFailures = loggerconstants::MaxConcurrentLogFailures; /**< Max allowed failures before stopping. */
  float sendLogsProbability = loggerconstants::LogSendProbability; /**< Probability to send logs. */
  bool sendFirstLog = false;          /**< Whether to always send the first log. */
  int maxFilesToSend = loggerconstants::MaxFilesToSend; /**< Max number of files to send in one batch. */
  int64_t _timer_interval = loggerconstants::LogTimeIntervalSecs; /**< Interval for sending logs. */
  int64_t background_timer_interval = loggerconstants::LogTimeIntervalSecs * 10; /**< Background send interval. */
  bool valid = false;                 /**< Whether the config is valid. */

  /**
   * @brief Constructs a default LogSendingConfig.
   */
  LogSendingConfig() {
    // required for probability generation
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto seed = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    std::srand(static_cast<unsigned int>(seed));
    std::vector<int64_t> secVec = {3617574009957856822, 7161680211933160759, 3834033765364414521,
                                   7378366457403629875};
    _defaultSecretKey = std::string(secVec.size() * 8, 'a');
    int64_t* d1 = (int64_t*)(&_defaultSecretKey[0]);
    for (int i = 0; i < secVec.size(); i++) {
      *d1 = secVec[i];
      ++d1;
    }
  }
};

/**
 * @brief Populates a LogSendingConfig from a JSON object.
 *
 * @param j The JSON object.
 * @param config The LogSendingConfig to populate.
 */
inline static void from_json(const nlohmann::json& j, LogSendingConfig& config) {
  j.at("url").get_to(config._host);
  // If key present in logSendingconfig either from cloud or from disk then take it, else set
  // _defaultSecretkey
  if (j.find("key") != j.end()) {
    j.at("key").get_to(config._secretKey);
  } else {
    config._secretKey = config._defaultSecretKey;
  }
  j.at("interval").get_to(config._timer_interval);
  if (j.find("maxConcurrentLogFailures") != j.end())
    j.at("maxConcurrentLogFailures").get_to(config.maxConcurrentLogFailures);
  if (j.find("sendLogsProbability") != j.end())
    j.at("sendLogsProbability").get_to(config.sendLogsProbability);
  if (j.find("maxFilesToSend") != j.end()) {
    j.at("maxFilesToSend").get_to(config.maxFilesToSend);
  }
  if (j.find("sendFirstLog") != j.end()) {
    j.at("sendFirstLog").get_to(config.sendFirstLog);
  }
  if (j.find("backgroundInterval") != j.end()) {
    j.at("backgroundInterval").get_to(config.background_timer_interval);
  }
  config.valid = true;
};

/**
 * @brief Serializes a LogSendingConfig to a JSON object.
 *
 * @param j The JSON object to populate.
 * @param config The LogSendingConfig to serialize.
 */
inline static void to_json(json& j, const LogSendingConfig& config) {
  j = json{{"url", config._host},
           {"interval", config._timer_interval},
           {"maxConcurrentLogFailures", config.maxConcurrentLogFailures},
           {"sendLogsProbability", config.sendLogsProbability},
           {"maxFilesToSend", config.maxFilesToSend},
           {"sendFirstLog", config.sendFirstLog},
           {"key", config._secretKey}};
};

class ServerAPI;

/**
 * @brief Handles sending logs to a remote logger service, including batching, retries, and log file management.
 */
class LogSender {
  std::mutex _senderMutex; /**< Mutex for thread-safe sending. */
  const std::string _service = "nimbleSDK"; /**< Service name. */
  const std::string _source = PLATFORM;      /**< Source platform. */
  const std::string _sdkVersion = SDKVERSION " (" NIMBLE_GIT_REV ")"; /**< SDK version string. */
  std::shared_ptr<ServerAPI> _serverAPI;     /**< Server API for sending logs. */
  std::shared_ptr<Logger> _mappedLogger;     /**< Logger instance for local logging. */
  LogSendingConfig _sender_config;           /**< Log sending configuration. */
  std::chrono::high_resolution_clock::time_point _lastSendTime =
      Time::get_high_resolution_clock_time() - std::chrono::hours(24); /**< Last time logs were sent. */
  std::shared_ptr<const Config> _config = nullptr; /**< Configuration pointer. */
  std::atomic<int> concurrentLogFailures;    /**< Number of consecutive log send failures. */
  bool _breakingFileForFirstTime = true;     /**< Whether breaking file for the first time. */

  /**
   * @brief Determines if logs should be sent based on the configured probability.
   *
   * @return True if logs should be sent, false otherwise.
   */
  bool should_send_logs();

  /**
   * @brief Gets the list of log files to send, sorted in order.
   *
   * @return Vector of file paths to log files ready for sending.
   */
  std::vector<std::string> get_log_files_to_send();

  /**
   * @brief Gets files in sorted order for sending from given logger instance.
   *
   * @param logger Logger instance to use for retrieving the log directory.
   * @return Vector of file paths to log files, sorted by name.
   */
  std::vector<std::string> get_files_in_sorted_order(std::shared_ptr<Logger> logger);

 public:
  /**
   * @brief Constructs a LogSender instance.
   *
   * @param serverAPI Shared pointer to the ServerAPI.
   * @param config Shared pointer to the configuration object.
   * @param mappedLogger Shared pointer to the Logger instance.
   * @param senderConfig Log sending configuration parameters.
   */
  LogSender(std::shared_ptr<ServerAPI> serverAPI, std::shared_ptr<const Config> config,
            std::shared_ptr<Logger> mappedLogger, const LogSendingConfig& senderConfig);

  /**
   * @brief Resets the sender's retry counter for consecutive log send failures.
   */
  void reset_sender_retries() { concurrentLogFailures = 0; }

  /**
   * @brief Sends any pending logs to the server if enough time has elapsed and failure count is within limits.
   */
  void send_pending_logs();

  /**
   * @brief Updates the sender configuration at runtime.
   *
   * @param config New log sending configuration to apply.
   */
  void update_sender_config(const LogSendingConfig& config);

  /**
   * @brief Sends all available logs, regardless of probability or timer interval. This is a blocking call.
   *
   * @return True if all logs were sent successfully, false if a send failed or configuration is invalid.
   */
  bool send_all_logs();

  /**
   * @brief Sends the specified log files to the server.
   *
   * Reads the contents of each file, uploads them as a batch, and removes files that were sent successfully.
   * If a file cannot be read or is empty, it is removed. If the upload fails, files are not removed.
   *
   * @param logfilePath Vector of log file paths to send.
   * @return True if logs were sent successfully, false otherwise.
   */
  bool send_logs(const std::vector<std::string>& logfilePath);
};
