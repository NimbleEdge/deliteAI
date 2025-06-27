/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// DataVariable classes from streaming related classes

#pragma once

#include <memory>

#include "char_stream.hpp"
#include "data_variable.hpp"
#include "dummy_offloaded_stream.hpp"
#include "json_stream.hpp"

class Task;

/**
 * @brief Base class for stream-based data variables
 *
 * StreamDataVariable provides the foundation for handling streaming data
 * in the NimbleNet system. It implements a template method pattern where
 * derived classes override execute_member_function to provide specific
 * streaming behavior while the base class handles common function calling logic.
 */
class StreamDataVariable : public DataVariable {
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) final;
  virtual OpReturnType execute_member_function(int memberFuncIndex,
                                               const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) = 0;
};

/**
 * @brief Iterator for character streams with JSON parsing capabilities
 *
 * CharStreamIterDataVariable provides iteration over character streams with
 * support for extracting JSON data. It maintains a position index and can
 * skip text to find JSON content, making it useful for parsing mixed text/JSON streams.
 */
class CharStreamIterDataVariable : public StreamDataVariable {
  std::shared_ptr<CharStream> _charStream; /**< The underlying character stream to iterate over */
  int _nextIdx = 0;                        /**< Current position index in the stream */

 private:
  // TODO: Maybe we should create CONTAINERTYPE::STREAM?
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::CHAR_STREAM; }

  std::string print() override { return "CharStreamIter"; }

  nlohmann::json to_json() const override { return "CharStreamIter"; }

  OpReturnType execute_member_function(int memberFuncIndex,
                                       const std::vector<OpReturnType>& arguments,
                                       CallStack& stack) override;

 public:
  CharStreamIterDataVariable(std::shared_ptr<CharStream> charStream);

 private:
  std::string next_string();
  std::shared_ptr<JSONStream> skip_text_and_get_json_stream(
      CallStack& stack, std::unique_lock<std::mutex>& streamPushLock);
  void wait_for_completion(CallStack& stack, std::unique_lock<std::mutex>& streamPushLock);
};

/**
 * @brief Data variable for JSON value streams with type-specific behavior
 *
 * JSONValueStreamDataVariable handles different types of JSON streams (objects,
 * arrays, strings, numbers) and provides appropriate access patterns for each.
 * It supports both blocking and non-blocking operations and can extract nested
 * values from JSON object streams.
 */
class JSONValueStreamDataVariable : public StreamDataVariable {
  enum class Type {
    OBJECT,  /**< JSON object stream type */
    ARRAY,   /**< JSON array stream type */
    STRING,  /**< JSON string stream type */
    NUMBER,  /**< JSON number stream type */
  };

  std::shared_ptr<JSONValueStream> _jsonValueStream; /**< The underlying JSON value stream */
  Type _valueType;                                   /**< The specific type of JSON stream */

 private:
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  // TODO: Create new data types maybe? Or just use JSON_VALUE_STREAM?
  int get_dataType_enum() const override { return DATATYPE::JSON_STREAM; }

  std::string print() override {
    if (_valueType == Type::STRING) {
      return _jsonValueStream->to_json().get<std::string>();
    }
    return _jsonValueStream->to_json().dump();
  }

  nlohmann::json to_json() const override { return _jsonValueStream->to_json(); }

  OpReturnType get_string_subscript(const std::string& key) override;
  OpReturnType execute_member_function(int memberFuncIndex,
                                       const std::vector<OpReturnType>& arguments,
                                       CallStack& stack) override;
  OpReturnType get_iterator(const std::vector<OpReturnType>& arguments, CallStack& stack);

  std::shared_ptr<JSONValueStream> get_json_value_stream(
      const std::vector<OpReturnType>& arguments, CallStack& stack,
      std::unique_lock<std::mutex>& streamPushLock, const std::string& funcName);
  OpReturnType get_blocking_str(const std::vector<OpReturnType>& arguments, CallStack& stack);

  static void wait_for_completion(std::shared_ptr<JSONValueStream> jsonValueStream,
                                  CallStack& stack, std::unique_lock<std::mutex>& streamPushLock);

 public:
  JSONValueStreamDataVariable(std::shared_ptr<JSONValueStream> valueStream);
};

/**
 * @brief Iterator for JSON array streams
 *
 * JSONArrayIterDataVariable provides iteration over JSON array streams,
 * allowing sequential access to array elements. It supports both blocking
 * and non-blocking iteration patterns and can check for element availability.
 */
class JSONArrayIterDataVariable : public StreamDataVariable {
  std::shared_ptr<JSONArrayStream> _arrayStream; /**< The underlying JSON array stream */
  int _nextIdx = 0;                              /**< Current position index in the array */

 private:
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  // TODO: Create new data types maybe? Or just use JSON_VALUE_STREAM?
  int get_dataType_enum() const override { return DATATYPE::JSON_STREAM; }

  std::string print() override { return "[JSONArrayIterator]"; }

  nlohmann::json to_json() const override { return "[JSONArrayIterator]"; }

 public:
  JSONArrayIterDataVariable(std::shared_ptr<JSONArrayStream> arrayStream)
      : _arrayStream(arrayStream) {}

  OpReturnType execute_member_function(int memberFuncIndex,
                                       const std::vector<OpReturnType>& arguments,
                                       CallStack& stack) override;
  OpReturnType get_next();
  bool is_next_available();
  OpReturnType next_blocking(CallStack& stack, std::unique_lock<std::mutex>& streamPushLock);
};
