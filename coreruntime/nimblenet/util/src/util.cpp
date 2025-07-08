/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util.hpp"

#include <cassert>
#include <exception>
#include <map>
#include <string>

#include "core_utils/ne_md5.hpp"
#include "data_variable.hpp"
#include "logger.hpp"
#include "nimble_net_util.hpp"

namespace util {
const char* get_string_from_enum(int dataType) {
  switch (dataType) {
    case DATATYPE::NONE:
      return "None";
    case DATATYPE::NIMBLENET:
      return "nimblenet";
    case DATATYPE::NIMBLENET_INTERNAL:
      return "nimblenet_internal";
    case DATATYPE::EMPTY:
      return "";
    case DATATYPE::UNKNOWN:
      return "None";
    case DATATYPE::FLOAT:
      return "float";
    case DATATYPE::BOOLEAN:
      return "bool";
    case DATATYPE::INT32:
      return "int32";
    case DATATYPE::INT64:
      return "int64";
    case DATATYPE::STRING:
      return "string";
    case DATATYPE::JSON:
      return "json";
    case DATATYPE::UNICODE_STRING:
      return "unicode_string";
    case DATATYPE::JSON_ARRAY:
      return "json_array";
    case DATATYPE::DOUBLE:
      return "double";
    case DATATYPE::INT32_ARRAY:
      return "int32[]";
    case DATATYPE::INT64_ARRAY:
      return "int64[]";
    case DATATYPE::DOUBLE_ARRAY:
      return "double[]";
    case DATATYPE::FLOAT_ARRAY:
      return "float[]";
    case DATATYPE::STRING_ARRAY:
      return "string[]";
    case DATATYPE::RAW_EVENTS_STORE:
      return "RawEventStore";
    case DATATYPE::DATAFRAME:
      return "DataFrame";
    case DATATYPE::TABLE_EVENT:
      return "TableEvent";
    case DATATYPE::NIMBLENET_REGEX:
      return "NimblenetRegex";
    case DATATYPE::NIMBLENET_REGEX_MATCHOBJECT:
      return "NimblenetRegexMatchObject";
    case DATATYPE::CHAR_STREAM:
      return "CharStream";
    case DATATYPE::FE_OBJ:
      return "FrontendObj";
    case DATATYPE::EXCEPTION:
      return "Exception";
    default:
      return "UNKNOWN";
  }
}

int get_enum_from_string(const char* type) {
  static std::map<std::string, int> typeMap = {{"float", DATATYPE::FLOAT},
                                               {"double", DATATYPE::DOUBLE},
                                               {"bool", DATATYPE::BOOLEAN},
                                               {"int32", DATATYPE::INT32},
                                               {"int64", DATATYPE::INT64},
                                               {"string", DATATYPE::STRING},
                                               {"json", DATATYPE::JSON},
                                               {"json_array", DATATYPE::JSON_ARRAY},
                                               {"int32[]", DATATYPE::INT32_ARRAY},
                                               {"int64[]", DATATYPE::INT64_ARRAY},
                                               {"float[]", DATATYPE::FLOAT_ARRAY},
                                               {"double[]", DATATYPE::DOUBLE_ARRAY},
                                               {"string[]", DATATYPE::STRING_ARRAY}};

  if (typeMap.find(type) == typeMap.end()) return -1;
  return typeMap[type];
}

bool is_dType_array(int dataType) {
  switch (dataType) {
    case DATATYPE::INT32_ARRAY:
    case DATATYPE::INT64_ARRAY:
    case DATATYPE::DOUBLE_ARRAY:
    case DATATYPE::FLOAT_ARRAY:
    case DATATYPE::STRING_ARRAY:
      return true;
    default:
      return false;
  }
}

int get_primitive_dType(int dataType) {
  switch (dataType) {
    case DATATYPE::INT32_ARRAY:
      return DATATYPE::INT32;
    case DATATYPE::INT64_ARRAY:
      return DATATYPE::INT64;
    case DATATYPE::DOUBLE_ARRAY:
      return DATATYPE::DOUBLE;
    case DATATYPE::FLOAT_ARRAY:
      return DATATYPE::FLOAT;
    case DATATYPE::STRING_ARRAY:
      return DATATYPE::STRING;
    default:
      return DATATYPE::UNKNOWN;
  }
}

int get_array_dataType(int dataType) {
  switch (dataType) {
    case DATATYPE::INT32:
      return DATATYPE::INT32_ARRAY;
    case DATATYPE::INT64:
      return DATATYPE::INT64_ARRAY;
    case DATATYPE::DOUBLE:
      return DATATYPE::DOUBLE_ARRAY;
    case DATATYPE::FLOAT:
      return DATATYPE::FLOAT_ARRAY;
    case DATATYPE::STRING:
      return DATATYPE::STRING_ARRAY;
    default:
      return DATATYPE::UNKNOWN;
  }
}

int get_containerType_from_dataType(int dataType) {
  switch (dataType) {
    case DATATYPE::NONE:
    case DATATYPE::JSON:
    case DATATYPE::BOOLEAN:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
    case DATATYPE::STRING:
    case DATATYPE::DOUBLE:
    case DATATYPE::FLOAT:
      return CONTAINERTYPE::SINGLE;
    case DATATYPE::INT32_ARRAY:
    case DATATYPE::INT64_ARRAY:
    case DATATYPE::FLOAT_ARRAY:
    case DATATYPE::DOUBLE_ARRAY:
    case DATATYPE::STRING_ARRAY:
      return CONTAINERTYPE::LIST;
    default:
      THROW("Trying to get container for invalid dataType=%s.",
            util::get_string_from_enum(dataType));
  }
}

void encrypt_data(char* data, int length) {
#ifdef NDEBUG
  for (int i = 0; i < length; ++i) {
    data[i] = (data[i] + 15) % 256;
  }
#endif
}

void decrypt_data(char* data, int length) {
#ifdef NDEBUG
  for (int i = 0; i < length; ++i) {
    data[i] = (data[i] - 15 + 256) % 256;  // adding 256 to prevent overflow
  }
#endif
}

void delete_extra_files(const std::string& directory, float fileTimeDeleteInDays) {
  DIR* dir;
  struct dirent* entry;
  struct stat fileInfo;

  // Open the directory
  if ((dir = opendir(directory.c_str())) != nullptr) {
    // Iterate over each entry in the directory
    while ((entry = readdir(dir)) != nullptr) {
      // Get the name of the file
      std::string filename = entry->d_name;
      if (filename == "." || filename == "..")  // Skip current and parent directory entries
        continue;

      // Construct full path
      std::string fullpath = directory + filename;

      // Get information about the file
      if (stat(fullpath.c_str(), &fileInfo) != 0) {
        // TODO: this throws an error while doing stat for symlinks in ios
#ifndef IOS
        LOG_TO_ERROR("Error getting file information for %s", fullpath.c_str());
#endif  // IOS
        continue;
      }
      if (S_ISDIR(fileInfo.st_mode)) {
        // is a directory
        continue;
      }

      // Extract the last access time
      std::time_t lastAccessTime = fileInfo.st_atime;
      if (Time::get_time() - lastAccessTime > fileTimeDeleteInDays * 60 * 60 * 24) {
        // LOG_TO_DEBUG("Deleting file %s", fullpath.c_str());
        remove(fullpath.c_str());
      }
    }
    // Close the directory
    closedir(dir);
  } else {
    // Error opening directory
    LOG_TO_ERROR("%s", "cannot open directory homeDir");
  }
}

}  // namespace util
