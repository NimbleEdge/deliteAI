/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <atomic>
#include <cstdarg>
#include <mutex>
#include <nlohmann/json.hpp>

#include "client.h"
#include "core_utils/atomic_ptr.hpp"
#include "core_utils/fmt.hpp"
#include "logger_constants.hpp"
#include "time_manager.hpp"

/**
 * @brief Configuration for writing logs to disk.
 */
struct LogWritingConfig {
  int maxLogFileSizeKB = loggerconstants::MaxLogFileSizeKB; /**< Maximum log file size in KB. */
  std::map<std::string, bool> logTypesToWrite; /**< Log types to write. */
  std::map<std::string, bool> eventTypesToWrite; /**< Event types to write. */
  bool scriptVerbose = false; /**< Enable verbose script logging. */
  bool collectEvents = false; /**< Enable event collection. */
};

/**
 * @brief Populates a LogWritingConfig from a JSON object.
 * 
 * @param j The JSON object.
 * @param config The LogWritingConfig to populate.
 */
static inline void from_json(const nlohmann::json& j, LogWritingConfig& config) {
  if (j.find("maxLogFileSizeKB") != j.end()) {
    j.at("maxLogFileSizeKB").get_to(config.maxLogFileSizeKB);
  }
  if (j.find("eventTypesToWrite") != j.end()) {
    j.at("eventTypesToWrite").get_to(config.eventTypesToWrite);
  }
  if (j.find("scriptVerbose") != j.end()) {
    j.at("scriptVerbose").get_to(config.scriptVerbose);
  }
  if (j.find("logTypesToWrite") != j.end()) {
    j.at("logTypesToWrite").get_to(config.logTypesToWrite);
  }
  if (j.find("collectEvents") != j.end()) {
    j.at("collectEvents").get_to(config.collectEvents);
  }
}

/**
 * @brief Serializes a LogWritingConfig to a JSON object.
 * 
 * @param j The JSON object to populate.
 * @param config The LogWritingConfig to serialize.
 */
static inline void to_json(nlohmann::json& j, const LogWritingConfig& config) {
  j = nlohmann::json{{"maxLogFileSizeKB", config.maxLogFileSizeKB},
                     {"scriptVerbose", config.scriptVerbose},
                     {"eventTypesToWrite", config.eventTypesToWrite},
                     {"logTypesToWrite", config.logTypesToWrite},
                     {"collectEvents", config.collectEvents}};
}

namespace ne {
/**
 * @brief Gets the file size for a given file path.
 * 
 * @param fullFilePath The full path to the file.
 * @return File size in bytes, or 0 if not found or is a directory.
 */
static inline int64_t get_file_size(const std::string& fullFilePath) {
  struct stat fileInfo;
  if (stat(fullFilePath.c_str(), &fileInfo) != 0) {
    return 0;
  }
  if (S_ISDIR(fileInfo.st_mode)) {
    // is a directory
    return 0;
  }

  // Extract the last access time
  return fileInfo.st_size;
};
}  // namespace ne

/**
 * @brief Provides thread-safe logging to disk with file rotation for both logs and events.
 */
class Logger {
  std::string _writeFile; /**< Current log file name. */
  std::string _logDirectory; /**< Directory for log files. */
  FILE* _writeFilePtr = NULL; /**< File pointer for writing logs. */
  ne::AtomicPtr<LogWritingConfig> _atomicLogConfig; /**< Atomic pointer to log config. */
  std::mutex _logMutex; /**< Mutex for thread-safe logging. */
  bool _isClientDebug = false; /**< Enable client debug logging. */
  std::atomic<int64_t> _dirSize = 0; /**< Current directory size in bytes. */
  int64_t _maxDirSize = loggerconstants::MaxEventsSizeKBs * 1024; /**< Max allowed directory size. */

  std::atomic<bool> _logVerbose = true; /**< Enable verbose logging. */
  std::atomic<bool> _logError = true; /**< Enable error logging. */
  std::atomic<bool> _logWarning = true; /**< Enable warning logging. */

 public:
  /**
   * @brief Constructs a Logger with a given log config.
   * 
   * @param logConfig The log writing configuration.
   */
  Logger(const LogWritingConfig& logConfig) {
    _atomicLogConfig.store(std::make_shared<LogWritingConfig>(logConfig));
  }

  /**
   * @brief Default constructor for Logger.
   */
  Logger() {};

  static ne::AtomicPtr<std::string> sessionId; /**< Session ID for logging. */

  /**
   * @brief Initializes the logger with a directory for log files.
   *
   * @param logDir Directory for log files.
   * @return True if initialization succeeded.
   */
  bool init_logger(const std::string& logDir);

  /**
   * @brief Sets the maximum size limit for the log directory.
   * 
   * @param maxSizeInKBs Maximum size in KB.
   */
  void set_max_size_limit(int64_t maxSizeInKBs) { _maxDirSize = maxSizeInKBs * 1024; }

  /**
   * @brief Recomputes the total disk size used by log files in the log directory.
   */
  void recompute_disk_size() {
    int64_t dirSize = 0;
    try {
      DIR* dir = opendir(_logDirectory.c_str());
      if (!dir) {
        return;
      } else {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
          // Get the name of the file
          std::string filename = entry->d_name;
          if (filename == "." || filename == "..")  // Skip current and parent directory entries
            continue;

          // Construct full path
          std::string fullpath = _logDirectory + filename;

          dirSize += ne::get_file_size(fullpath);
        }
        closedir(dir);
      }
    } catch (...) {
      log_fatal("Unable to check for directory to write logs for nimbleSDK with exception");
      return;
    }
    _dirSize = dirSize;
  }

  /**
   * @brief Writes a log message to the log file, with log rotation if size limit is reached.
   *
   * @param message The log message.
   * @param type The log type (e.g., INFO, ERROR).
   * @param currentDate The current date string (optional, defaults to UTC date).
   */
  void write_log(const char* message, const char* type,
                 const std::string& currentDate = Time::get_date_UTC());

  /**
   * @brief Updates the log configuration atomically.
   *
   * @param config The new log writing configuration.
   */
  void update_log_config(const LogWritingConfig& config);

  /**
   * @brief Enables or disables client debug logging.
   *
   * @param debug True to enable, false to disable.
   */
  void set_debug_flag(bool debug) { _isClientDebug = debug; }

  /**
   * @brief Breaks the current log file and returns the new file name.
   *
   * @return The new log file name, or an empty string if the file was empty and not rotated.
   */
  std::string take_lock_and_break_current_file();

  /**
   * @brief Performs cleanup on segmentation fault (no-op).
   *
   * @param sigNum Signal number.
   */
  void perform_segfault_cleanup(int sigNum) {};

  /**
   * @brief Returns the directory used for log files.
   *
   * @return The log directory path.
   */
  std::string get_directory() const { return _logDirectory; };

  /**
   * @brief Logs a verbose message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGVERBOSE(const char* format, ...) {
    NE_VARIADIC_FMT(format);
    log_verbose(buf.str);
  }

  /**
   * @brief Logs a debug message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGDEBUG(const char* format, ...) {
    if (!_logVerbose) return;
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "DEBUG");
#ifdef NDEBUG
    // non debug
#else
    log_debug(buf.str);
#endif
  }

  /**
   * @brief Logs an info message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGINFO(const char* format, ...) {
    if (!_logVerbose) return;
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "INFO");
#ifdef NDEBUG
    // non debug
#else
    log_info(buf.str);
#endif
  }

  /**
   * @brief Logs a client info message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGCLIENTINFO(const char* format, ...) {
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "INFO");
    log_info(buf.str);
  }

  /**
   * @brief Logs a warning message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGWARN(const char* format, ...) {
    if (!_logWarning) return;
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "WARN");
    log_warn(buf.str);
  }

  /**
   * @brief Logs an error message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGERROR(const char* format, ...) {
    if (!_logError) return;
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "ERROR");
#ifdef NDEBUG
    // non debug
#else
    log_error(buf.str);
#endif
  };

  /**
   * @brief Logs a client error message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void LOGCLIENTERROR(const char* format, ...) {
    NE_VARIADIC_FMT(format);
    write_log(buf.str, "ERROR");
    log_error(buf.str);
  };

  /**
   * @brief Logs a metrics event.
   *
   * @param metricType The type of metric.
   * @param metricJsonString The metric data as a JSON string.
   */
  void LOGMETRICS(const char* metricType, const char* metricJsonString) {
    auto logConfig = _atomicLogConfig.load();
    auto found = logConfig->logTypesToWrite.find(metricType);
    if (found != logConfig->logTypesToWrite.end() && found->second == false) {
      return;
    }

    auto buf = ne::fmt("%s ::: %s", metricType, metricJsonString);
    write_log(buf.str, "METRICS");
  }

  /**
   * @brief Logs a client debug message.
   *
   * @param format printf-style format string.
   * @param ... Arguments for the format string.
   */
  void CLIENTDEBUGLOG(const char* format, ...) {
    if (!_isClientDebug) {
      return;
    }
    NE_VARIADIC_FMT(format);
    log_debug(buf.str);
  }

  /**
   * @brief Logs a script event if enabled.
   *
   * @param deploymentId Deployment ID for the script.
   * @param metricType The type of metric.
   * @param metricJsonString The metric data as a JSON string.
   */
  void script_log(int deploymentId, const char* metricType, const char* metricJsonString);

  /**
   * @brief Logs an event if the event type is enabled.
   *
   * @param eventType The type of event.
   * @param rawEventJsonString The event data as a JSON string.
   * @return True if the event was logged or skipped due to config, false if the event type is not enabled.
   */
  bool event_log(const char* eventType, const char* rawEventJsonString);

  /**
   * @brief Checks if an event type is new (not previously registered).
   *
   * @param eventType The event type to check.
   * @return True if the event type is new, false otherwise.
   */
  bool is_new_event_type(const std::string& eventType) {
    auto inserted = _atomicLogConfig.load()->eventTypesToWrite.insert({eventType, false});
    return inserted.second;
  }

  /**
   * @brief Destructor for Logger. Closes the log file if open.
   */
  ~Logger() {
    if (_writeFilePtr) {
      fclose(_writeFilePtr);
    }
  }

 private:
  /**
   * @brief Breaks the current log file and returns the new file name (internal).
   * 
   * @param newFileName The new file name.
   * @param logMutexUniqueLock Unique lock for thread safety.
   * @return The new log file name.
   */
  std::string break_current_file(std::string newFileName,
                                 std::unique_lock<std::mutex>&& logMutexUniqueLock);
};

extern std::shared_ptr<Logger> logger;

#define LOG_TO_ERROR(fmt, ...) logger->LOGERROR(fmt, ##__VA_ARGS__)
#define LOG_TO_CLIENT_ERROR(fmt, ...) logger->LOGCLIENTERROR(fmt, ##__VA_ARGS__)
#define LOG_TO_INFO(fmt, ...) logger->LOGINFO(fmt, ##__VA_ARGS__)
#define LOG_TO_CLIENT_INFO(fmt, ...) logger->LOGCLIENTINFO(fmt, ##__VA_ARGS__)
#define LOG_TO_WARN(fmt, ...) logger->LOGWARN(fmt, ##__VA_ARGS__)
#define LOG_TO_DEBUG(fmt, ...) logger->LOGDEBUG(fmt, ##__VA_ARGS__)
#define LOG_TO_CLIENT_DEBUG(fmt, ...) logger->CLIENTDEBUGLOG(fmt, ##__VA_ARGS__)

#if defined(ENABLE_VERBOSE_LOGGING) && defined(ALLOW_VERBOSE_LOGGING)
#define LOG_VERBOSE(fmt, ...) logger->LOGVERBOSE(fmt, ##__VA_ARGS__)
#else  // ENABLE_VERBOSE_LOGGING
#define LOG_VERBOSE(...) \
  do {                   \
  } while (false)
#endif  // ENABLE_VERBOSE_LOGGING

/**
 * @brief Defining a custom error category
 */
class NimbleEdgeError : public std::error_category {
  int _errorcode = 0;

 public:
  const char* name() const noexcept override { return "NimbleEdgeError"; }

  std::string message(int ev) const override { return "Unknown error"; }

  NimbleEdgeError(int errorCode) { _errorcode = errorCode; }
};

/**
 * Converting throwBuffer.str to std::string explicitly just to ensure std::system_error doesn't
 * directly store the pointer we give it
 */
#define NETHROW(code, format, ...)                                                             \
  do {                                                                                         \
    auto throwBuffer = ne::fmt(format, ##__VA_ARGS__);                                         \
    auto errorCategory = NimbleEdgeError(code);                                                \
    auto systemError =                                                                         \
        std::system_error(std::error_code(code, errorCategory), std::string{throwBuffer.str}); \
    throw systemError;                                                                         \
  } while (0)
