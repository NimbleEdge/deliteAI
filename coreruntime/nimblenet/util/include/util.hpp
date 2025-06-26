/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdint>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core_utils/fmt.hpp"
#include "executor_structs.h"
#include "logger.hpp"
#include "nimble_net_util.hpp"

/**
 * @brief Platform string describing the current build target.
 */
#if defined(__ANDROID__)
#ifdef __i386__
static inline const std::string PLATFORM = "android_x86";
#elif __x86_64__
static inline const std::string PLATFORM = "android_x86_64";
#elif __arm__
static inline const std::string PLATFORM = "android_armeabi-v7a";
#elif __aarch64__
static inline const std::string PLATFORM = "android_arm64-v8a";
#else
static inline const std::string PLATFORM = "android_unknown";
#endif
#else
#if defined(__APPLE__)
#ifdef __i386__
static inline const std::string PLATFORM = std::string("ios_i386") + IOS_PLATFORM;
#elif __x86_64__
static inline const std::string PLATFORM = std::string("ios_x86_64") + IOS_PLATFORM;
#elif __arm__
static inline const std::string PLATFORM = std::string("ios_arm") + IOS_PLATFORM;
#elif __aarch64__
static inline const std::string PLATFORM = std::string("ios_aarch64") + std::string(IOS_PLATFORM);
#endif
#else
#ifdef __i386__
static inline const std::string PLATFORM = "i386";
#elif __x86_64__
static inline const std::string PLATFORM = "x86_64";
#elif __arm__
static inline const std::string PLATFORM = "arm";
#elif __aarch64__
static inline const std::string PLATFORM = "aarch64";
#else
static inline const std::string PLATFORM = "unknown";
#endif
#endif
#endif

#define DEFAULT_SQLITE_DB_NAME "nimbleDB"

/**
 * @brief Global default task name for scripts.
 */
static inline const char* GLOBALTASKNAME = "DEFAULT_SCRIPT";

/**
 * @brief Macro to wrap a function call in try-catch and return NimbleNetStatus on error.
 */
#define TRY_CATCH_RETURN_NIMBLESTATUS(func)              \
  try {                                                  \
    return func;                                         \
  } catch (const std::exception& e) {                    \
    return util::nimblestatus(1, "%s", e.what());        \
  } catch (...) {                                        \
    return util::nimblestatus(1, "%s", "unknown error"); \
  }

/**
 * @brief Macro to wrap a void function call in try-catch and log errors as NimbleNetStatus.
 */
#define TRY_CATCH_RETURN_VOID(func)                             \
  try {                                                         \
    func;                                                       \
  } catch (const std::exception& e) {                           \
    auto status = util::nimblestatus(1, "%s", e.what());        \
    deallocate_nimblenet_status(status);                        \
  } catch (...) {                                               \
    auto status = util::nimblestatus(1, "%s", "unknown error"); \
    deallocate_nimblenet_status(status);                        \
  }

/**
 * @brief Macro to wrap a function call in try-catch and return a default value on error.
 */
#define TRY_CATCH_RETURN_DEFAULT(func, defaultReturnValue)      \
  try {                                                         \
    return func;                                                \
  } catch (const std::exception& e) {                           \
    auto status = util::nimblestatus(1, "%s", e.what());        \
    deallocate_nimblenet_status(status);                        \
    return defaultReturnValue;                                  \
  } catch (...) {                                               \
    auto status = util::nimblestatus(1, "%s", "unknown error"); \
    deallocate_nimblenet_status(status);                        \
    return defaultReturnValue;                                  \
  }

/**
 * @brief Utility namespace containing helper functions for data types, error handling, and file operations.
 */
namespace util {
/**
 * @brief Get the string representation of a data type enum value.
 *
 * @param dataType Integer representing the data type.
 * @return C-string describing the data type.
 */
const char* get_string_from_enum(int dataType);

/**
 * @brief Get the enum value from a string representation of a data type.
 *
 * @param type C-string describing the data type.
 * @return Integer enum value for the data type, or -1 if not found.
 */
int get_enum_from_string(const char* type);

/**
 * @brief Check if a data type represents an array type.
 *
 * @param dataType Integer representing the data type.
 * @return True if the data type is an array type, false otherwise.
 */
bool is_dType_array(int dataType);

/**
 * @brief Get the primitive (non-array) data type for an array data type.
 *
 * @param dataType Integer representing the array data type.
 * @return Integer enum value for the primitive data type, or UNKNOWN if not applicable.
 */
int get_primitive_dType(int dataType);

/**
 * @brief Get the array data type for a primitive data type.
 *
 * @param dataType Integer representing the primitive data type.
 * @return Integer enum value for the array data type, or UNKNOWN if not applicable.
 */
int get_array_dataType(int dataType);

/**
 * @brief Get the container type for a given data type.
 *
 * @param dataType Integer representing the data type.
 * @return Integer enum value for the container type.
 * @throws Throws if the data type is invalid.
 */
int get_containerType_from_dataType(int dataType);

/**
 * @brief Set the global session ID for logging and tracking.
 *
 * @param sessionIdString Session ID string. If empty, a new session ID is generated.
 */
static inline void set_session_id(const std::string& sessionIdString) {
  if (sessionIdString.empty()) {
    auto timesession = std::to_string(Time::get_time());
    Logger::sessionId.store(std::make_shared<std::string>(timesession));
    LOG_TO_INFO("Updated session id for the session to %s", timesession.c_str());
    return;
  }
  Logger::sessionId.store(std::make_shared<std::string>(sessionIdString));
  LOG_TO_INFO("Updated session id for the session to %s", sessionIdString.c_str());
  return;
};

/**
 * @brief Convert a string to a value of type T using stringstream.
 *
 * @tparam T Target type.
 * @param s String to convert.
 * @return Value of type T.
 */
template <typename T>
T GetAs(const std::string& s) {
  std::stringstream ss{s};
  T t;
  if (!(ss >> t)) LOG_TO_ERROR("%s cannot be converted to %s", s.c_str(), typeid(T).name());
  return t;
}

/**
 * @brief Recursively convert a multi-dimensional array to a string representation.
 *
 * @tparam T Data type of the array elements.
 * @param shape Shape of the array.
 * @param shapeDepth Current depth in the shape vector.
 * @param data Pointer to the data array.
 * @param dataIndex Current index in the data array.
 * @param totalSizeOfDepth Total size at the current depth.
 * @return String representation of the array.
 */
template <typename T>
std::string recursive_string(const std::vector<int64_t>& shape, int shapeDepth, const T* data,
                             int dataIndex, int totalSizeOfDepth) {
  if (shapeDepth == shape.size()) {
    std::stringstream ss;
    if constexpr (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
      ss << (int)data[dataIndex];
    } else {
      ss << data[dataIndex];
    }
    return ss.str();
  }
  std::string output = "[";
  for (int i = 0; i < shape[shapeDepth]; i++) {
    output += recursive_string(shape, shapeDepth + 1, data,
                               dataIndex + i * totalSizeOfDepth / shape[shapeDepth],
                               totalSizeOfDepth / shape[shapeDepth]);
    if (i != shape[shapeDepth] - 1) {
      output += ",";
    }
  }
  output += "]";
  return output;
}

/**
 * @brief Recursively convert a multi-dimensional array to a JSON representation.
 *
 * @tparam T Data type of the array elements.
 * @param shape Shape of the array.
 * @param shapeDepth Current depth in the shape vector.
 * @param data Pointer to the data array.
 * @param dataIndex Current index in the data array.
 * @param totalSizeOfDepth Total size at the current depth.
 * @return JSON representation of the array.
 */
template <typename T>
nlohmann::json recursive_json(const std::vector<int64_t>& shape, int shapeDepth, const T* data,
                              int dataIndex, int totalSizeOfDepth) {
  if (shapeDepth == shape.size()) {
    std::stringstream ss;
    if constexpr (std::is_same<T, int8_t>::value || std::is_same<T, uint8_t>::value) {
      ss << (int)data[dataIndex];
    } else {
      ss << data[dataIndex];
    }
    return ss.str();
  }

  nlohmann::json array = nlohmann::json::array();
  for (int i = 0; i < shape[shapeDepth]; i++) {
    array.push_back(recursive_string(shape, shapeDepth + 1, data,
                                     dataIndex + i * totalSizeOfDepth / shape[shapeDepth],
                                     totalSizeOfDepth / shape[shapeDepth]));
  }
  return array;
}

/**
 * @brief Create a NimbleNetStatus error object with formatted message and code.
 *
 * @tparam Args Variadic arguments for formatting.
 * @param code Error code.
 * @param format Format string.
 * @param args Arguments for the format string.
 * @return Pointer to a NimbleNetStatus object (caller must free).
 */
template <typename... Args>
NimbleNetStatus* nimblestatus(int code, const char* format, Args... args) {
  char* buf = ne::fmt_to_raw(format, args...);
  LOG_TO_CLIENT_ERROR("%s", buf);
  NimbleNetStatus* error = (NimbleNetStatus*)malloc(sizeof(NimbleNetStatus));
  error->code = code;
  error->message = buf;
  return error;
}

/**
 * @brief Create a NimbleNetStatus error object from a std::system_error.
 *
 * @param e System error exception.
 * @return Pointer to a NimbleNetStatus object (caller must free).
 */
static inline NimbleNetStatus* nimblestatus(std::system_error& e) {
  NimbleNetStatus* error = (NimbleNetStatus*)malloc(sizeof(NimbleNetStatus));
  LOG_TO_CLIENT_ERROR("%s", e.what());
  error->code = e.code().value();
  asprintf(&(error->message), "%s", e.what());
  return error;
}

/**
 * @brief Get the field size (in bytes) for a given data type.
 *
 * @param dataType Integer representing the data type.
 * @return Size in bytes for the data type, or 0 if not defined.
 */
static inline int get_field_size_from_data_type(int dataType) {
  switch (dataType) {
    case DATATYPE::STRING:
      return 1;
    case DATATYPE::FLOAT:
    case DATATYPE::INT32:
      return 4;
    case DATATYPE::INT64:
    case DATATYPE::DOUBLE:
      return 8;
    default:
      LOG_TO_CLIENT_ERROR("Datatype=%d not defined", dataType);
      return 0;
  }
}

/**
 * @brief Delete extra files in a directory older than a specified number of days.
 *
 * @param directory Directory path.
 * @param fileTimeDeleteInDays Files older than this (in days) will be deleted.
 */
void delete_extra_files(const std::string& directory, float fileTimeDeleteInDays);

/**
 * @brief Encrypt data in-place using a simple algorithm (only in release builds).
 *
 * @param data Pointer to data buffer.
 * @param length Length of the data buffer.
 */
void encrypt_data(char* data, int length);

/**
 * @brief Decrypt data in-place using a simple algorithm (only in release builds).
 *
 * @param data Pointer to data buffer.
 * @param length Length of the data buffer.
 */
void decrypt_data(char* data, int length);

/**
 * @brief Apply a function to zipped elements of multiple iterators in parallel.
 *
 * @tparam F Function type.
 * @tparam Iterator Iterator type.
 * @tparam ExtraIterators Additional iterator types.
 * @param func Function to apply.
 * @param begin Begin iterator.
 * @param end End iterator.
 * @param extra_begin_iterators Additional begin iterators.
 * @return The function object after applying to all elements.
 */
template <typename F, typename Iterator, typename... ExtraIterators>
F for_each_zipped(F func, Iterator begin, Iterator end, ExtraIterators... extra_begin_iterators) {
  for (; begin != end; ++begin, detail::advance_all(extra_begin_iterators...))
    func(*begin, *(extra_begin_iterators)...);
  return func;
}

/**
 * @brief Apply a function to zipped elements of multiple containers in parallel.
 *
 * @tparam F Function type.
 * @tparam Container Container type.
 * @tparam ExtraContainers Additional container types.
 * @param func Function to apply.
 * @param container Main container.
 * @param extra_containers Additional containers.
 * @return The function object after applying to all elements.
 */
template <typename F, typename Container, typename... ExtraContainers>
F for_each_zipped_containers(F func, Container& container, ExtraContainers&... extra_containers) {
  return for_each_zipped(func, std::begin(container), std::end(container),
                         std::begin(extra_containers)...);
}

/**
 * @brief Call a function with a default-initialized C++ type corresponding to a data type enum.
 *
 * @tparam Func Function type.
 * @tparam Ts Additional argument types.
 * @param func Function to call.
 * @param dataType Data type enum value.
 * @param ts Additional arguments.
 * @return Result of the function call.
 * @throws Throws if the data type is not implemented.
 */
template <class Func, typename... Ts>
auto call_function_for_dataType(Func func, DATATYPE dataType, Ts&&... ts) {
  using Result = decltype(func(int32_t{}, std::forward<Ts>(ts)...));
  switch (dataType) {
    case DATATYPE::INT32:
      return func(int32_t{}, std::forward<Ts>(ts)...);
    case DATATYPE::DOUBLE:
      return func(double{}, std::forward<Ts>(ts)...);
    case DATATYPE::FLOAT:
      return func(float{}, std::forward<Ts>(ts)...);
    case DATATYPE::INT64:
      return func(int64_t{}, std::forward<Ts>(ts)...);
    case DATATYPE::BOOLEAN:
      return func(bool{}, std::forward<Ts>(ts)...);
    case DATATYPE::STRING:
      // TODO: This is allocating a string unnecessarily, use some other marker type
      return func(std::string{}, std::forward<Ts>(ts)...);
    default:
      THROW("Not implemented for %s", get_string_from_enum(dataType));
  }
};

/**
 * @brief Call a function with a default-initialized C++ numeric type corresponding to a data type enum.
 *
 * @tparam Func Function type.
 * @tparam Ts Additional argument types.
 * @param func Function to call.
 * @param dataType Data type enum value.
 * @param ts Additional arguments.
 * @return Result of the function call.
 * @throws Throws if the data type is not implemented.
 */
template <class Func, typename... Ts>
auto call_function_for_numeric_dataType(Func func, DATATYPE dataType, Ts&&... ts) {
  using Result = decltype(func(int32_t{}, std::forward<Ts>(ts)...));
  switch (dataType) {
    case DATATYPE::INT32:
      return func(int32_t{}, std::forward<Ts>(ts)...);
    case DATATYPE::DOUBLE:
      return func(double{}, std::forward<Ts>(ts)...);
    case DATATYPE::FLOAT:
      return func(float{}, std::forward<Ts>(ts)...);
    case DATATYPE::INT64:
      return func(int64_t{}, std::forward<Ts>(ts)...);
    default:
      THROW("Not implemented for %s", get_string_from_enum(dataType));
  }
};

/**
 * @brief Recursively delete a folder and all its contents.
 *
 * @param folderPath Path to the folder to delete.
 * @return True if deletion succeeded, false otherwise.
 */
static inline bool delete_folder_recursively(const std::string& folderPath) {
  DIR* dir = opendir(folderPath.c_str());
  if (!dir) {
    return false;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name != "." && name != "..") {
      std::string fullPath = folderPath + "/" + name;
      struct stat statbuf;
      if (stat(fullPath.c_str(), &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
          delete_folder_recursively(fullPath);
        } else {
          unlink(fullPath.c_str());
        }
      }
    }
  }
  closedir(dir);
  return rmdir(folderPath.c_str()) == 0;
}

/**
 * @brief Namespace for UTF-8 string handling utilities.
 */
namespace utf8 {
/**
 * @brief Check if a byte is a UTF-8 continuation byte (10xxxxxx).
 *
 * @param c Byte to check.
 * @return True if the byte is a continuation byte, false otherwise.
 */
inline bool is_continuation_byte(char c) { return (c & 0xC0) == 0x80; }

/**
 * @brief Get the size (in bytes) of a UTF-8 character based on its first byte.
 *
 * @param c First byte of the UTF-8 character.
 * @return Size in bytes of the UTF-8 character.
 */
inline int char_size(unsigned char c) {
  if ((c & 0x80) == 0)
    return 1;  // 0xxxxxxx (ASCII)
  else if ((c & 0xE0) == 0xC0)
    return 2;  // 110xxxxx
  else if ((c & 0xF0) == 0xE0)
    return 3;  // 1110xxxx
  else if ((c & 0xF8) == 0xF0)
    return 4;  // 11110xxx
  return 1;    // Invalid UTF-8, treat as single byte
}

/**
 * @brief Count the number of Unicode characters in a UTF-8 string.
 *
 * @param str UTF-8 encoded string.
 * @return Number of Unicode characters in the string.
 */
inline int count_chars(const std::string& str) {
  int char_count = 0;
  const char* s = str.c_str();
  while (*s) {
    if (!is_continuation_byte(*s)) char_count++;
    s++;
  }
  return char_count;
}

/**
 * @brief Extract a single UTF-8 character at a specific byte position.
 *
 * @param str UTF-8 encoded string.
 * @param byte_pos Byte position in the string.
 * @return Extracted UTF-8 character as a string, or empty string if out of bounds.
 */
inline std::string extract_char(const std::string& str, int byte_pos) {
  if (byte_pos < 0 || byte_pos >= str.size()) return "";

  const char* s = str.c_str() + byte_pos;
  int size = char_size(*s);

  // Ensure we don't read past the end of the string
  if (byte_pos + size > str.size()) size = str.size() - byte_pos;

  return std::string(s, size);
}

}  // namespace utf8
}  // namespace util
