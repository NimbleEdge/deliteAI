#include "stream_data_variable.hpp"

#include <memory>
#include <string>

#include "char_stream.hpp"
#include "data_variable.hpp"
#include "json_stream.hpp"
#include "single_variable.hpp"
#include "task.hpp"

OpReturnType StreamDataVariable::call_function(int memberFuncIndex,
                                               const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  // leave script lock and relock
  auto scopedUnlocker = stack.scoped_unlock();
  return execute_member_function(memberFuncIndex, arguments, stack);
}

/*******************************CharStreamIterDataVariable*********************************/

CharStreamIterDataVariable::CharStreamIterDataVariable(std::shared_ptr<CharStream> charStream)
    : _charStream(charStream) {}

void CharStreamIterDataVariable::wait_for_completion(CallStack& stack,
                                                     std::unique_lock<std::mutex>& streamPushLock) {
  auto streamClosedChecker = [charStream = this->_charStream]() {
    // Run background jobs until stream closed
    return charStream->closed();
  };
  stack.task()->run_background_jobs_until_condition(streamClosedChecker, streamPushLock);
}

OpReturnType CharStreamIterDataVariable::execute_member_function(
    int memberFuncIndex, const std::vector<OpReturnType>& arguments, CallStack& stack) {
  auto streamPushLock = stack.task()->get_stream_push_lock();
  switch (memberFuncIndex) {
    case WAIT_FOR_COMPLETION:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      wait_for_completion(stack, streamPushLock);
      return std::make_shared<NoneVariable>();
    case FINISHED:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return std::make_shared<SingleVariable<bool>>(_charStream->closed() &&
                                                    _nextIdx >= _charStream->size());
    case NEXT:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return std::make_shared<SingleVariable<std::string>>(next_string());
    case GET_BLOCKING_STR:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      wait_for_completion(stack, streamPushLock);
      return std::make_shared<SingleVariable<std::string>>(print());
    case SKIP_TEXT_AND_GET_JSON_STREAM:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return std::make_shared<JSONValueStreamDataVariable>(
          skip_text_and_get_json_stream(stack, streamPushLock));
  }
  THROW("%s not implemented for CharStream", DataVariable::get_member_func_string(memberFuncIndex));
}

std::string CharStreamIterDataVariable::next_string() {
  auto startIdx = _nextIdx;
  _nextIdx = _charStream->size();
  return _charStream->get_stream_view(startIdx).to_string();
}

std::shared_ptr<JSONStream> CharStreamIterDataVariable::skip_text_and_get_json_stream(
    CallStack& stack, std::unique_lock<std::mutex>& streamPushLock) {
  auto openingBraceChecker = [&]() {
    // Run background jobs until opening brace character doesn't appear.
    while (!_charStream->closed() && !_charStream->empty()) {
      if (_charStream->peek() == '{') {
        return true;
      }
      _charStream->pop();
    }
    return false;
  };
  stack.task()->run_background_jobs_until_condition(openingBraceChecker, streamPushLock);
  auto jsonStream = std::make_shared<JSONStream>(_charStream);
  _charStream->set_subscriber(
      std::bind(&JSONStringStream::parse_ahead, std::ref(*jsonStream.get())));
  return jsonStream;
}

/****************************JSONValueStreamDataVariable******************************/

JSONValueStreamDataVariable::JSONValueStreamDataVariable(
    std::shared_ptr<JSONValueStream> valueStream)
    : _jsonValueStream(valueStream) {
  if (std::dynamic_pointer_cast<JSONStream>(valueStream)) {
    _valueType = Type::OBJECT;
  } else if (std::dynamic_pointer_cast<JSONStringStream>(valueStream)) {
    _valueType = Type::STRING;
  } else if (std::dynamic_pointer_cast<JSONArrayStream>(valueStream)) {
    _valueType = Type::ARRAY;
  } else if (std::dynamic_pointer_cast<JSONNumberStream>(valueStream)) {
    _valueType = Type::NUMBER;
  } else {
    THROW("%s", "Cannot create JSONValueStream of unknown type");
  }
}

OpReturnType JSONValueStreamDataVariable::get_string_subscript(const std::string& key) {
  if (_valueType != Type::OBJECT) {
    THROW_UNSUPPORTED("get_string_subscript");
  }
  auto jsonStream = std::dynamic_pointer_cast<JSONStream>(_jsonValueStream);
  auto jsonValueStream = jsonStream->get_value(key);
  if (jsonValueStream) {
    return std::make_shared<JSONValueStreamDataVariable>(jsonValueStream);
  } else {
    return std::make_shared<NoneVariable>();
  }
}

void JSONValueStreamDataVariable::wait_for_completion(
    std::shared_ptr<JSONValueStream> jsonValueStream, CallStack& stack,
    std::unique_lock<std::mutex>& streamPushLock) {
  auto jsonValueChecker = [jsonValStream = jsonValueStream]() { return jsonValStream->finished(); };
  stack.task()->run_background_jobs_until_condition(jsonValueChecker, streamPushLock);
}

OpReturnType JSONValueStreamDataVariable::execute_member_function(
    int memberFuncIndex, const std::vector<OpReturnType>& arguments, CallStack& stack) {
  auto streamPushLock = stack.task()->get_stream_push_lock();
  switch (memberFuncIndex) {
    case FINISHED:
      THROW_ARGUMENTS_NOT_MATCH(0, arguments.size(), memberFuncIndex);
      return std::make_shared<SingleVariable<bool>>(_jsonValueStream->finished());
    case ITERATOR:
      return get_iterator(arguments, stack);
    case GET_BLOCKING:
      return std::make_shared<JSONValueStreamDataVariable>(
          get_json_value_stream(arguments, stack, streamPushLock, "get_blocking"));
    case GET_BLOCKING_STR: {
      auto jsonValueStream =
          get_json_value_stream(arguments, stack, streamPushLock, "get_blocking_str");
      wait_for_completion(jsonValueStream, stack, streamPushLock);
      auto jsonValueDataVariable =
          std::make_shared<JSONValueStreamDataVariable>(std::move(jsonValueStream));
      return OpReturnType(new SingleVariable<std::string>(jsonValueDataVariable->print()));
    }
    case WAIT_FOR_COMPLETION:
      THROW_ARGUMENTS_NOT_MATCH(0, arguments.size(), memberFuncIndex);
      wait_for_completion(_jsonValueStream, stack, streamPushLock);
      return std::make_shared<NoneVariable>();
  }

  THROW("%s not implemented for JSONValueStream",
        DataVariable::get_member_func_string(memberFuncIndex));
}

std::shared_ptr<JSONValueStream> JSONValueStreamDataVariable::get_json_value_stream(
    const std::vector<OpReturnType>& arguments, CallStack& stack,
    std::unique_lock<std::mutex>& streamPushLock, const std::string& funcName) {
  if (_valueType != Type::OBJECT) {
    THROW_UNSUPPORTED(funcName.c_str());
  }
  THROW_ARGUMENTS_NOT_MATCH(1, arguments.size(), GET_BLOCKING);
  const auto key = arguments[0]->get_string();

  auto jsonStream = std::dynamic_pointer_cast<JSONStream>(_jsonValueStream);
  std::shared_ptr<JSONValueStream> jsonValueStream;

  auto jsonValueChecker = [&]() {
    jsonValueStream = jsonStream->get_value(key);
    if (!jsonStream->finished() && !jsonValueStream) {
      return false;
    }
    return true;
  };
  stack.task()->run_background_jobs_until_condition(jsonValueChecker, streamPushLock);
  if (!jsonValueStream) {
    THROW("Did not find key %s in JSON", key.c_str());
  }

  return jsonValueStream;
}

OpReturnType JSONValueStreamDataVariable::get_iterator(const std::vector<OpReturnType>& arguments,
                                                       CallStack& stack) {
  if (_valueType != Type::ARRAY) {
    THROW_UNSUPPORTED("iterator");
  }
  auto arrayStream = std::dynamic_pointer_cast<JSONArrayStream>(_jsonValueStream);
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, ITERATOR);
  return std::make_shared<JSONArrayIterDataVariable>(arrayStream);
}

/****************************JSONArrayIterDataVariable******************************/

OpReturnType JSONArrayIterDataVariable::execute_member_function(
    int memberFuncIndex, const std::vector<OpReturnType>& arguments, CallStack& stack) {
  auto streamPushLock = stack.task()->get_stream_push_lock();
  switch (memberFuncIndex) {
    case NEXT:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return get_next();
    case NEXT_AVAILABLE:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return std::make_shared<SingleVariable<bool>>(is_next_available());
    case NEXT_BLOCKING:
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, memberFuncIndex);
      return next_blocking(stack, streamPushLock);
  }

  THROW("%s not implemented for JSONArrayStreamIterator",
        DataVariable::get_member_func_string(memberFuncIndex));
}

OpReturnType JSONArrayIterDataVariable::get_next() {
  if (_nextIdx >= _arrayStream->size()) {
    // FIXME: We probably want to check if the charStream is closed or not here
    // If not closed, we should wait or throw instead of returning None
    return std::make_shared<NoneVariable>();
  }

  auto valStream = _arrayStream->get_idx(_nextIdx++);
  return std::make_shared<JSONValueStreamDataVariable>(valStream);
}

bool JSONArrayIterDataVariable::is_next_available() {
  if (_arrayStream->finished()) return true;

  return _nextIdx < _arrayStream->size();
}

OpReturnType JSONArrayIterDataVariable::next_blocking(
    CallStack& stack, std::unique_lock<std::mutex>& streamPushLock) {
  auto getNextChecker = [&]() {
    return _nextIdx < _arrayStream->size() || _arrayStream->finished();
  };
  stack.task()->run_background_jobs_until_condition(getNextChecker, streamPushLock);
  if (_nextIdx >= _arrayStream->size()) {
    return std::make_shared<NoneVariable>();
  }
  return std::make_shared<JSONValueStreamDataVariable>(_arrayStream->get_idx(_nextIdx++));
}
