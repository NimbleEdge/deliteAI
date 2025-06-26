/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "json_stream.hpp"

#include <cctype>
#include <memory>

#include "char_stream.hpp"
#include "core_utils/fmt.hpp"
#include "nlohmann/json.hpp"

/**************************JSONValueStream*******************************/

std::shared_ptr<JSONValueStream> JSONValueStream::get_value_stream(
    std::shared_ptr<CharStream> charStream) {
  std::optional<char> optChar = charStream->pop_ws_and_peek();
  if (!optChar) return {};
  char c = optChar.value();
  switch (c) {
    case '"':
      return std::make_shared<JSONStringStream>(charStream);
    case '{':
      return std::make_shared<JSONStream>(charStream);
    case '[':
      return std::make_shared<JSONArrayStream>(charStream);
  }
  if ((c >= '0' && c <= '9') || c == '-') {
    return std::make_shared<JSONNumberStream>(charStream);
  }
  if (c == 't' || c == 'f') {
    THROW("%s", "bool value type not supported yet");
  }
  if (c == 'n') {
    THROW("%s", "null value not supported yet");
  }
  THROW("Unkown value type that begins with character %c", c);
}

/****************************JSONStream**********************************/

std::shared_ptr<JSONValueStream> JSONStream::get_value(const std::string& key) {
  auto it = _map.find(key);
  if (it == _map.end()) {
    return {};
  }
  return it->second;
}

bool JSONStream::finished() const {
  // Either parser is finished, or the charStream source has closed AND we have parsed everything
  // off of it
  return _parserState == ParserState::FINISH || (_charStream->closed() && _charStream->empty());
}

bool JSONStream::parse() {
  if (_parserState == ParserState::FINISH) return true;
  if (_charStream->empty()) return false;
  switch (_parserState) {
    case ParserState::START: {
      if (_charStream->empty()) return false;
      std::optional<char> c = _charStream->pop_next_non_ws();
      if (!c) return false;

      // We have a non whitespace character, has to be an opening brace
      if (c != '{') {
        THROW("Expect JSON to start with '{', instead started with %c", c);
      }
      _currentStream = std::make_shared<JSONStringStream>(_charStream);
      _parserState = ParserState::PARSING_KEY;
      break;
    }
    case ParserState::PARSING_KEY:
      if (_currentStream->parse()) {
        // current stream i.e. the key stream finished parsing, change state
        _parserState = ParserState::PARSED_KEY;
      }
      break;
    case ParserState::PARSED_KEY: {
      std::optional<char> c = _charStream->pop_next_non_ws();
      if (!c) return false;

      if (c != ':') {
        THROW("JSON Key value should be separated with ':', instead found %c", c.value());
      }

      _parserState = ParserState::CREATE_VALUE_STREAM;
      break;
    }
    case ParserState::CREATE_VALUE_STREAM: {
      std::shared_ptr<JSONValueStream> valueStream = JSONValueStream::get_value_stream(_charStream);
      if (!valueStream) {
        // Stream was exhausted and couldn't determine value type
        break;
      }

      std::string key = std::dynamic_pointer_cast<JSONStringStream>(_currentStream)->to_string();
      _currentStream = std::move(valueStream);
      _parserState = ParserState::PARSING_VALUE;
      _map[key] = _currentStream;
      break;
    }
    case ParserState::PARSING_VALUE:
      if (_currentStream->parse()) {
        _parserState = ParserState::PARSED_VALUE;
      }
      break;
    case ParserState::PARSED_VALUE: {
      std::optional<char> c = _charStream->pop_next_non_ws();
      if (!c) return false;

      switch (c.value()) {
        case ',':
          _parserState = ParserState::PARSING_KEY;
          _currentStream = std::make_shared<JSONStringStream>(_charStream);
          break;
        case '}':
          _parserState = ParserState::FINISH;
          _currentStream.reset();
          break;
        default:
          THROW("After value, we should get , or }, instead got %c", c.value());
      }
      break;
    }
    case ParserState::FINISH:
      return true;
  }

  // Parse remaining characters in stream after state change
  return parse();
}

std::string JSONStream::to_json_string(int nesting) const {
  std::string ret = "{\n";
  for (const auto& [key, valueStream] : _map) {
    for (int i = 0; i < nesting + 1; i++) ret += JSONValueStream::TAB_STRING;
    const auto valueString = valueStream->to_json_string(nesting + 1);
    ret += ne::fmt("\"%s\": %s,\n", key.c_str(), valueString.c_str()).str;
  }
  for (int i = 0; i < nesting; i++) ret += JSONValueStream::TAB_STRING;
  ret += "}";
  return ret;
}

nlohmann::json JSONStream::to_json() const {
  nlohmann::json::object_t obj;
  for (const auto& [key, valueStream] : _map) {
    obj.insert({key, valueStream->to_json()});
  }
  return obj;
}

/************************JSONStringStream**********************************/

bool JSONStringStream::finished() const { return _endIdx.has_value() || _charStream->closed(); }

bool JSONStringStream::parse() {
  // TODO: JSON strings can have escape characters as well:
  // \" \b \\ \/ \b \f \n \r \t \u0000 (any 4 hex digits: unicode escape)
  if (!_startIdx) {
    // Looking for the starting quote
    while (!_charStream->empty()) {
      char c = _charStream->pop();
      if (std::isspace(c)) {
        continue;
      }

      if (c != '"') {
        // String must start with a double quote
        THROW("JSON String should start with '\"' but starts with '%c' instead", c);
      }

      // NOTE: This won't cause problems even if _charStream ends with quote since one after the
      // end is a valid position for substring positions and return empty string
      _startIdx = _charStream->last_seen_idx() + 1;
      break;
    }
  }

  // Continue till we haven't hit closing quotes
  while (!_charStream->empty()) {
    char c = _charStream->pop();
    if (c == '"') {
      _endIdx = _charStream->last_seen_idx() - 1;  // one before the quote
      return true;                                 // done parsing the string
    }
  }

  return false;
}

std::string JSONStringStream::to_string() const {
  if (!_startIdx) {
    return "";
  }

  if (!_endIdx) {
    return _charStream->get_stream_view(_startIdx.value()).to_string();
  } else {
    return _charStream->get_stream_view(_startIdx.value(), _endIdx.value()).to_string();
  }
}

std::string JSONStringStream::to_json_string(int nesting) const {
  std::string str = to_string();
  return std::string{ne::fmt("\"%s\"", str.c_str()).str};
}

nlohmann::json JSONStringStream::to_json() const {
  nlohmann::json::string_t jsonStr = to_string();
  return jsonStr;
}

/**************************************JSONArrayStream**************************************/

bool JSONArrayStream::parse() {
  if (_parserState == ParserState::FINISH) return true;
  if (_charStream->empty()) return false;

  switch (_parserState) {
    case ParserState::START: {
      std::optional<char> optionalChar = _charStream->pop_next_non_ws();
      if (!optionalChar) return false;
      char c = optionalChar.value();

      if (c != '[') {
        THROW("JSON Array must begin with '[', begins with %c instead", c);
      }
      _parserState = ParserState::CREATE_VALUE_STREAM;
      break;
    }
    case ParserState::CREATE_VALUE_STREAM: {
      auto valueStream = JSONValueStream::get_value_stream(_charStream);
      if (!valueStream) break;
      _values.emplace_back(std::move(valueStream));
      _parserState = ParserState::PARSING_VALUE;
      break;
    }
    case ParserState::PARSING_VALUE: {
      if (_values.back()->parse()) {
        _parserState = ParserState::PARSED_VALUE;
      }
      break;
    }
    case ParserState::PARSED_VALUE: {
      std::optional<char> optionalChar = _charStream->pop_next_non_ws();
      if (!optionalChar) return false;
      char c = optionalChar.value();

      if (c == ',') {
        _parserState = ParserState::CREATE_VALUE_STREAM;
      } else if (c == ']') {
        _parserState = ParserState::FINISH;
      } else {
        THROW("Expected ',' or ']' after JSON array element, found %c instead", c);
      }
      break;
    }
    case ParserState::FINISH:
      return true;
  }

  return parse();
}

bool JSONArrayStream::finished() const {
  return _parserState == ParserState::FINISH || _charStream->closed();
}

std::string JSONArrayStream::to_json_string(int nesting) const {
  std::string ret = "";
  for (int i = 0; i < nesting; i++) ret += JSONValueStream::TAB_STRING;
  ret += "[ ";
  for (const auto& valueStream : _values) {
    ret += valueStream->to_json_string(nesting);
    ret += ", ";
  }
  ret += "]";
  return ret;
}

nlohmann::json JSONArrayStream::to_json() const {
  nlohmann::json::array_t arr;
  arr.reserve(_values.size());
  for (const auto& valueStream : _values) {
    arr.push_back(valueStream->to_json());
  }
  return arr;
}

int JSONArrayStream::size() const noexcept { return _values.size(); }

std::shared_ptr<JSONValueStream> JSONArrayStream::get_idx(int idx) const { return _values.at(idx); }

/*********************************JSONNumberStream*******************************************/

bool JSONNumberStream::parse() {
  if (!_startIdx) {
    const auto charOpt = _charStream->pop_next_non_ws();
    if (!charOpt) {
      // Char stream only had whitespace
      return false;
    }
    const auto c = charOpt.value();
    if (c != '-' && !(c >= '0' && c <= '9')) {
      THROW("Number should start with - or 0-9 digits, started with %c instead", c);
    }
    _startIdx = _charStream->last_seen_idx();
  }

  // Continue reading characters until we hit comma
  // NOTE: We are deliberately not doing in depth parsing and leaving that to checking using
  // from_chars function at the end. Since a partial number might not be parseable, e.g. 3E, we have
  // to check the result after the number is finished
  while (!_charStream->empty()) {
    char c = _charStream->peek();
    if (c == ',' || c == '}' || c == ']') {
      _endIdx = _charStream->last_seen_idx();
      break;
    }
    _charStream->pop();
  }

  if (_endIdx) {
    // Check the validity of the found number by getting a long double out of it
    static_cast<void>(get_number<long double>());
    return true;
  }

  return false;
}

bool JSONNumberStream::finished() const { return _endIdx.has_value() || _charStream->closed(); }

std::string JSONNumberStream::to_json_string(int nesting) const { return to_json().dump(); }

nlohmann::json JSONNumberStream::to_json() const { return get_number<long double>(); }
