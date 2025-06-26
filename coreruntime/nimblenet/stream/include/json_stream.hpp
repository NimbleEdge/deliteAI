/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <charconv>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "char_stream.hpp"
#include "core_utils/fmt.hpp"
#include "ne_type_traits.hpp"
#include "nlohmann/json_fwd.hpp"

class JSONValueStream {
 public:
  // Returns true when done parsing, else returns false
  virtual bool parse() = 0;
  virtual std::string to_json_string(int nesting = 0) const = 0;
  virtual nlohmann::json to_json() const = 0;
  virtual bool finished() const = 0;
  virtual std::shared_ptr<CharStream> char_stream() = 0;
  static constexpr const char* TAB_STRING = "    ";

  // Wrapper function for when we want to pass the stream directly as a subscriber to CharStream
  void parse_ahead() { parse(); }

  // Determine value type by peeking into the char stream, return nullptr if stream exhausted
  static std::shared_ptr<JSONValueStream> get_value_stream(std::shared_ptr<CharStream> charStream);
  virtual ~JSONValueStream() = default;
};

class JSONStream : public JSONValueStream {
 private:
  enum class ParserState {
    START,
    PARSING_KEY,
    PARSED_KEY,
    CREATE_VALUE_STREAM,
    PARSING_VALUE,
    PARSED_VALUE,
    FINISH
  };

 private:
  std::unordered_map<std::string, std::shared_ptr<JSONValueStream>> _map;

  std::shared_ptr<JSONValueStream> _currentStream;
  ParserState _parserState = ParserState::START;
  std::shared_ptr<CharStream> _charStream;

 private:
  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  JSONStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  // Accessing the JSON key-values
  std::shared_ptr<JSONValueStream> get_value(const std::string& key);
  bool finished() const override;

  bool parse() override;
  std::string to_json_string(int nesting = 0) const override;
  nlohmann::json to_json() const override;
};

class JSONStringStream : public JSONValueStream {
  std::optional<int> _startIdx;  // for when we haven't found the starting quote
  std::optional<int> _endIdx;    // for when we haven't found the ending quote
  std::shared_ptr<CharStream> _charStream;

 private:
  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  JSONStringStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  JSONStringStream(JSONStringStream&) = delete;
  JSONStringStream(JSONStringStream&&) = delete;

  // Called when new data is inserted into the character stream
  bool parse() override;
  bool finished() const override;

  // returns empty string if no string has been parsed, else constructs a string out of everything
  // in [_startIdx, _endIdx]
  std::string to_string() const;
  std::string to_json_string(int nesting = 0) const override;
  nlohmann::json to_json() const override;
};

class JSONArrayStream : public JSONValueStream {
 private:
  enum class ParserState { START, CREATE_VALUE_STREAM, PARSING_VALUE, PARSED_VALUE, FINISH };

 private:
  std::shared_ptr<CharStream> _charStream;
  ParserState _parserState = ParserState::START;
  std::vector<std::shared_ptr<JSONValueStream>> _values;

 private:
  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  JSONArrayStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  bool parse() override;
  bool finished() const override;
  std::string to_json_string(int nesting = 0) const override;
  nlohmann::json to_json() const override;

  // Accessing array
  int size() const noexcept;
  std::shared_ptr<JSONValueStream> get_idx(int idx) const;
};

class JSONNumberStream : public JSONValueStream {
 private:
  std::shared_ptr<CharStream> _charStream;
  std::optional<int> _startIdx;  // for when we haven't found the starting of the number
  std::optional<int> _endIdx;    // for when we haven't found the ending of the number

 private:
  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  JSONNumberStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  bool parse() override;
  bool finished() const override;
  std::string to_json_string(int nesting = 0) const override;
  nlohmann::json to_json() const override;

  // Getting the number
  template <typename T>
  T get_number() const;
};

/************************** Template function impls **************************************/
template <typename T>
T JSONNumberStream::get_number() const {
  if (!_startIdx) {
    THROW("%s", "Haven't parsed any number");
  }
  int startIdx = _startIdx.value();
  int endIdx = _endIdx.value_or(_charStream->last_seen_idx());

  const auto sv = _charStream->get_stream_view(startIdx, endIdx).to_string_view();
  T res;
  int len = endIdx - startIdx + 1;

  if constexpr (std::is_integral_v<T>) {
    const auto fromCharsRes = std::from_chars(sv.data(), sv.data() + len, res);

    if (fromCharsRes.ptr != sv.data() + len) {
      std::string str{sv};
      THROW("Unable to parse %s as number completely", str.c_str());
    }

    if (fromCharsRes.ec != std::errc{}) {
      std::string str{sv};
      std::string errorMessage{"Unknown Error"};
      if (fromCharsRes.ec == std::errc::invalid_argument) {
        errorMessage = "Invalid Argument";
      } else if (fromCharsRes.ec == std::errc::result_out_of_range) {
        errorMessage = "Result out of range";
      }

      THROW("Error in parsing %s as number: %s", str.c_str(), errorMessage.c_str());
    }
  } else {
    // Have to do this conversion as other methods need null terminated string
    std::string str{sv};
    const char* errorMessage = nullptr;
    try {
      if constexpr (std::is_same_v<T, float>) {
        res = std::stof(str);
      } else if constexpr (std::is_same_v<T, double>) {
        res = std::stod(str);
      } else if constexpr (std::is_same_v<T, long double>) {
        res = std::stold(str);
      } else {
        static_assert(ne::always_false<T>, "This non integral type is not supported");
      }
    } catch (const std::invalid_argument& e) {
      errorMessage = "Invalid Argument";
    } catch (const std::out_of_range& e) {
      errorMessage = "Result out of range";
    } catch (...) {
      errorMessage = "Unknown Error";
    }

    if (errorMessage) {
      THROW("Error in parsing %s as number: %s", str.c_str(), errorMessage);
    }
  }

  return res;
}
