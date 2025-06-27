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

class StreamDataVariable : public DataVariable {
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) final;
  virtual OpReturnType execute_member_function(int memberFuncIndex,
                                               const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) = 0;
};

class CharStreamIterDataVariable : public StreamDataVariable {
  std::shared_ptr<CharStream> _charStream;
  int _nextIdx = 0;

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

class JSONValueStreamDataVariable : public StreamDataVariable {
  enum class Type {
    OBJECT,
    ARRAY,
    STRING,
    NUMBER,
  };

  std::shared_ptr<JSONValueStream> _jsonValueStream;
  Type _valueType;

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

class JSONArrayIterDataVariable : public StreamDataVariable {
  std::shared_ptr<JSONArrayStream> _arrayStream;
  int _nextIdx = 0;

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
