/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>

#include "client.h"
#include "core_utils/fmt.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "logger_constants.hpp"
#include "native_interface.hpp"
#include "time_manager.hpp"
#include "user_events_constants.hpp"
#include "util.hpp"

/**
 * @brief Configuration for sending logs
 */
struct LogConfig {
  int maxLogFileSizeKB = loggerconstants::MaxLogFileSizeKB; /**< Maximum log file size in KB. */
  bool toSend = true; /**< Whether logs should be sent. */
  int timeWindowToSave = 0; /**< Time window for saving logs. */
};

#define FIRST_FILE_NAME "latest.txt"

/**
 * @brief Metadata for a log file in the file store.
 */
struct FileData {
  std::string fileName; /**< Name of the file. */
  double lastTimestamp = std::numeric_limits<double>::max(); /**< Timestamp of last event. */
  int totalEvents = 0; /**< Total number of events in the file. */
  bool valid = false; /**< Whether the file data is valid. */

  /**
   * @brief Constructs FileData from a file name string.
   * 
   * @param fName File name string.
   */
  FileData(const std::string& fName) {
    fileName = fName;
    std::istringstream iss(fileName);
    iss >> lastTimestamp >> totalEvents;
    if (iss.fail()) {
      valid = false;
      return;
    }
    valid = true;
  }

  /**
   * @brief Generates a file name for saving based on current time and event count.
   * 
   * @return File name string.
   */
  std::string get_filename_to_save() {
    std::string currentTime = Time::get_time_for_event_store_file();
    return currentTime + " " + std::to_string(totalEvents);
  }

  /**
   * @brief Default constructor for FileData.
   */
  FileData() {
    fileName = FIRST_FILE_NAME;
    valid = true;
  }

  /**
   * @brief Comparison operator for sorting FileData by file name.
   * 
   * @param other Another FileData instance.
   * @return True if this file name is less than other's.
   */
  bool operator<(const FileData& other) const { return fileName < other.fileName; }
};

/**
 * @brief Manages log file compression, storage, rotation and retrieval.
 */
class FileStore {
  std::string _logDirectory; /**< Directory where log files are stored. */
  FILE* _writeFilePtr = nullptr; /**< File pointer for writing logs. */
  std::mutex _logMutex; /**< Mutex for thread-safe log writing. */
  LogConfig _logConfig; /**< Log configuration. */
  FileData _currentFileData; /**< Metadata for the current log file. */

  /**
   * @brief Retrieves metadata for all log files in the directory.
   * 
   * @return Vector of FileData for each file.
   */
  std::vector<FileData> get_all_files_data() const {
    DIR* dir;
    struct dirent* entry;
    std::vector<FileData> filesData;
    if ((dir = opendir(_logDirectory.c_str())) != NULL) {
      while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // If it's a regular file
          std::string fileName = entry->d_name;
          if (fileName == FIRST_FILE_NAME) {
            filesData.push_back(_currentFileData);
          } else
            filesData.emplace_back(FileData(fileName));
        }
      }
      closedir(dir);
    }
    return filesData;
  }

  /**
   * @brief Reads all events from a file and appends them to a vector.
   * 
   * @param filePath Path to the file.
   * @param eventsVector Vector to append parsed events to.
   */
  void read_lines_from_file(const std::string& filePath,
                            std::vector<nlohmann::json>& eventsVector) const {
    auto [readSuccess, fileData] =
        nativeinterface::read_potentially_compressed_file(filePath, true);
    if (!readSuccess) {
      return;
    }
    std::istringstream file(fileData);
    // release memory ASAP. Have to do this since there's no move constructor for istringstream
    fileData.clear();

    std::string line;
    while (std::getline(file, line)) {
      try {
        std::istringstream iss(line);
        std::string prefix;
        std::string date;
        std::string time;
        std::string eventType;
        std::string eventJsonString;
        iss >> prefix >> date >> time >> prefix >> eventType >> prefix >> eventJsonString;
        auto eventJson = nlohmann::json::parse(eventJsonString);
        auto t = Time::get_epoch_time_from_timestamp(date + " " + time);
        if (iss.fail() || t == -1) {
          // skip corrupted event
          continue;
        }
        eventJson[usereventconstants::TimestampField] = t;
        eventsVector.push_back(eventJson);
      } catch (...) {
        continue;
      }
    }
  }

 public:
  /**
   * @brief Constructs a FileStore for a given directory and log configuration.
   * 
   * @param directory Directory to store log files.
   * @param logConfig Log configuration.
   */
  FileStore(const std::string& directory, const LogConfig& logConfig) : _currentFileData() {
    _logDirectory = directory;
    mkdir(_logDirectory.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    auto fileName = _logDirectory + "/" + _currentFileData.fileName;
    {
      // just to count number of events
      // NOTE: Using fstream here, since FIRST_FILE_NAME (default file) is not compressed
      std::fstream file;
      file.open(fileName, std::ios::in | std::ios::app);
      std::string line;
      // Count the number of lines
      while (std::getline(file, line)) {
        ++_currentFileData.totalEvents;
      }
      file.close();
    }
    _writeFilePtr = fopen(fileName.c_str(), "a+");
    _logConfig = logConfig;
  }

  /**
   * @brief Destructor, closes the log file pointer.
   */
  ~FileStore() { fclose(_writeFilePtr); }

  /**
   * @brief Writes a log message to the current file, rotating if needed.
   * 
   * @param message Log message to write.
   */
  void write(const char* message) {
    std::lock_guard<std::mutex> locker(_logMutex);
    fputs(message, _writeFilePtr);
    fflush(_writeFilePtr);
    long size = ftell(_writeFilePtr);
    _currentFileData.totalEvents++;
    if ((float)size / (loggerconstants::MaxBytesInKB) > _logConfig.maxLogFileSizeKB) {
      fclose(_writeFilePtr);
      std::string newFileName = _logDirectory + "/" + _currentFileData.get_filename_to_save();
      auto fileName = _logDirectory + "/" + _currentFileData.fileName;
      if (!nativeinterface::compress_file(fileName.c_str(), newFileName.c_str())) {
        LOG_TO_ERROR("FileStore: Compressing file %s to %s failed, saving uncompressed",
                     fileName.c_str(), newFileName.c_str());
        rename(fileName.c_str(), newFileName.c_str());
      }
      remove(fileName.c_str());

      _writeFilePtr = fopen(fileName.c_str(), "a+");
      _currentFileData = FileData();
    }
  }

  /**
   * @brief Reads all events from all log files.
   * 
   * @return Vector of parsed event JSON objects.
   */
  std::vector<nlohmann::json> read() const {
    DIR* dir;
    struct dirent* entry;
    std::vector<nlohmann::json> eventsVector;
    std::vector<FileData> filesData = get_all_files_data();
    sort(filesData.begin(), filesData.end());
    for (auto& fileData : filesData) {
      std::string filePath = _logDirectory + "/" + fileData.fileName;
      read_lines_from_file(filePath, eventsVector);
    }
    return eventsVector;
  }

  /**
   * @brief Deletes log files with last event timestamp older than expiryTime.
   * 
   * @param expiryTime Expiry timestamp; files older than this are deleted.
   * @return True if operation succeeded.
   */
  bool delete_old_events(int64_t expiryTime) const {
    std::vector<FileData> filesData = get_all_files_data();
    for (auto& fileData : filesData) {
      if (fileData.valid && fileData.lastTimestamp < expiryTime) {
        // later remove according to sent as well
        std::string filePath = _logDirectory + "/" + fileData.fileName;
        remove(filePath.c_str());
      }
    }
    return true;
  }

  /**
   * @brief Deletes oldest log files to keep total event count under maxEvents.
   * 
   * @param maxEvents Maximum allowed number of events.
   * @return True if operation succeeded.
   */
  bool delete_old_events_by_count(int maxEvents) const {
    if (maxEvents < 0) return false;
    std::vector<FileData> filesData = get_all_files_data();
    sort(filesData.rbegin(), filesData.rend());
    int numEvents = 0;
    for (auto& fileData : filesData) {
      if (!fileData.valid) continue;
      if (numEvents > maxEvents) {
        // we have enough files that contain required number of events
        std::string filePath = _logDirectory + "/" + fileData.fileName;
        remove(filePath.c_str());
      }
      numEvents += fileData.totalEvents;
    }
    return true;
  }

  /**
   * @brief Calculates total size of all log files in bytes.
   * 
   * @return Total size in bytes.
   */
  int size_in_bytes() const {
    std::vector<FileData> filesData = get_all_files_data();
    int size = 0;
    for (auto& fileData : filesData) {
      if (fileData.valid) {
        // later remove according to sent as well
        std::string filePath = _logDirectory + "/" + fileData.fileName;
        struct stat st;

        // Get information about the file
        if (stat(filePath.c_str(), &st) == 0) {
          if (S_ISREG(st.st_mode)) {
            size += st.st_size;
          }
        }
      }
    }
    return size;
  }

  /**
   * @brief Gets the total number of events across all log files.
   * 
   * @return Total event count.
   */
  int get_num_events() const {
    std::vector<FileData> filesData = get_all_files_data();
    int totalEvents = 0;
    for (auto& fileData : filesData) {
      if (fileData.valid) {
        totalEvents += fileData.totalEvents;
      }
    }
    return totalEvents;
  }
};

/**
 * @brief Type of store - LOGS or METRICS.
 */
enum class StoreType { LOGS, METRICS };

/**
 * @brief Template class for managing multiple FileStores by type (e.g., logs, metrics).
 *
 * @tparam storeType The type of store (LOGS or METRICS).
 */
template <StoreType storeType>
class Store {
  std::string _directory; /**< Directory for storing types. */
  std::map<std::string, FileStore> _type2FileStoreMap; /**< Map from type to FileStore. */
  LogConfig _defaultConfig; /**< Default log configuration. */

 public:
  /**
   * @brief Formats a log line for writing.
   * 
   * @param type Log type string.
   * @param timestamp Timestamp string.
   * @param log Log message.
   * @return Formatted log line.
   */
  static char* format(const char* type, const char* timestamp, const char* log);

  /**
   * @brief Initializes the store by scanning the directory for existing types.
   * 
   * @param directory Directory to scan and initialize.
   */
  void init(const std::string& directory) {
    _directory = directory;
    mkdir(_directory.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    DIR* dir = opendir(_directory.c_str());
    if (dir == nullptr) {
      return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      std::string path = _directory + "/" + entry->d_name;
      struct stat statbuf;
      if (stat(path.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
          add_type(entry->d_name);
        }
      }
    }

    closedir(dir);
  }

  /**
   * @brief Adds a new type to the store if not already present.
   * 
   * @param type Type string to add.
   */
  void add_type(const std::string& type) {
    auto it = _type2FileStoreMap.find(type);
    if (it == _type2FileStoreMap.end()) {
      std::string filePath = _directory + "/" + type;
      _type2FileStoreMap.emplace(std::piecewise_construct, std::forward_as_tuple(type),
                                 std::forward_as_tuple(filePath, _defaultConfig));
    }
  }

  /**
   * @brief Writes a log or metric entry for a given type.
   * 
   * @param type Type string.
   * @param log Log or metric message.
   */
  void write(const char* type, const char* log) {
    add_type(type);
    std::string currentDate = Time::get_date_UTC();
    auto line = format(type, currentDate.c_str(), log);
    _type2FileStoreMap.at(type).write(line);
    free(line);
  }

  /**
   * @brief Reads all events for a given type.
   * 
   * @param type Type string.
   * @return Vector of parsed event JSON objects.
   */
  std::vector<nlohmann::json> read(const char* type) const {
    auto it = _type2FileStoreMap.find(type);
    if (it == _type2FileStoreMap.end()) {
      return {};
    }
    return it->second.read();
  }

  /**
   * @brief Deletes old events for a given type based on expiry time.
   * 
   * @param type Type string.
   * @param expiryTime Expiry timestamp.
   * @return True if operation succeeded.
   */
  bool delete_old_events(const char* type, int64_t expiryTime) const {
    auto it = _type2FileStoreMap.find(type);
    if (it == _type2FileStoreMap.end()) {
      return false;
    }
    return it->second.delete_old_events(expiryTime);
  }

  /**
   * @brief Deletes old events for a given type to keep event count under maxEvents.
   * 
   * @param type Type string.
   * @param maxEvents Maximum allowed number of events.
   * @return True if operation succeeded.
   */
  bool delete_old_events_by_count(const char* type, int maxEvents) const {
    auto it = _type2FileStoreMap.find(type);
    if (it == _type2FileStoreMap.end()) {
      return false;
    }
    return it->second.delete_old_events_by_count(maxEvents);
  }

  /**
   * @brief Calculates total size in bytes for all types.
   * 
   * @return Total size in bytes.
   */
  int size_in_bytes() const {
    int size = 0;
    for (const auto& pair : _type2FileStoreMap) {
      size += pair.second.size_in_bytes();
    }
    return size;
  }

  /**
   * @brief Gets the number of events for a given type.
   * 
   * @param type Type string.
   * @return Number of events for the type.
   */
  int get_num_events(const char* type) const {
    auto it = _type2FileStoreMap.find(type);
    if (it == _type2FileStoreMap.end()) {
      return 0;
    }
    return it->second.get_num_events();
  }

  /**
   * @brief Gets the total number of events across all types.
   * 
   * @return Total event count.
   */
  int get_num_events() const {
    int total = 0;
    for (auto& [key, value] : _type2FileStoreMap) {
      total += value.get_num_events();
    }
    return total;
  }

  /**
   * @brief Gets a set of all types currently in the store.
   * 
   * @return Set of type strings.
   */
  std::set<std::string> get_all_types() const {
    std::set<std::string> set;
    for (const auto& pair : _type2FileStoreMap) {
      set.insert(pair.first);
    }
    return set;
  }

  /**
   * @brief Deletes all data for a given type.
   * 
   * @param type Type string.
   */
  void delete_type(const char* type) {
    auto it = _type2FileStoreMap.find(type);
    if (it != _type2FileStoreMap.end()) {
      _type2FileStoreMap.erase(it);
      std::string filePath = _directory + "/" + type;
      util::delete_folder_recursively(filePath);
    }
  }
};

/**
 * @brief Aggregates log and metric stores and provides logging interface.
 */
struct Monitor {
  Store<StoreType::LOGS> logstore; /**< Store for log events. */
  Store<StoreType::METRICS> metricStore; /**< Store for metric events. */

  /**
   * @brief Logs an info message to the log store and optionally to the debug logger.
   * 
   * @param format Format string (printf-style).
   * @param ... Arguments for the format string.
   */
  void LOGINFO(const char* format, ...) {
    NE_VARIADIC_FMT(format);
    logstore.write("INFO", buf.str);
#ifdef NDEBUG
    // non debug
#else
    log_info(buf.str);
#endif
  }
};
