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

/**
 * @brief Abstract base class for streaming and parsing JSON values.
 *
 * Provides an interface for parsing, serializing, and accessing JSON values from a character stream.
 */
class JSONValueStream {
 public:
  /**
   * @brief Parse the value from the stream.
   *
   * @return True if parsing is complete, false otherwise.
   */
  virtual bool parse() = 0;

  /**
   * @brief Serialize the value to a JSON string.
   *
   * @param nesting Indentation level for pretty printing.
   * @return JSON string representation.
   */
  virtual std::string to_json_string(int nesting = 0) const = 0;

  /**
   * @brief Convert the value to a nlohmann::json object.
   *
   * @return JSON object representation.
   */
  virtual nlohmann::json to_json() const = 0;

  /**
   * @brief Check if parsing is finished.
   *
   * @return True if finished, false otherwise.
   */
  virtual bool finished() const = 0;

  /**
   * @brief Get the underlying character stream.
   *
   * @return Shared pointer to the CharStream.
   */
  virtual std::shared_ptr<CharStream> char_stream() = 0;

  static constexpr const char* TAB_STRING = "    ";

  /**
   * @brief Wrapper function to parse ahead, useful as a CharStream subscriber.
   */
  void parse_ahead() { parse(); }

  /**
   * @brief Factory function to create a JSONValueStream for the next value in the stream.
   *
   * @param charStream The character stream to parse from.
   * @return Shared pointer to a JSONValueStream, or nullptr if stream exhausted.
   */
  static std::shared_ptr<JSONValueStream> get_value_stream(std::shared_ptr<CharStream> charStream);

  /**
   * @brief Virtual destructor.
   */
  virtual ~JSONValueStream() = default;
};

/**
 * @brief Stream parser for JSON objects (dictionaries).
 *
 * Parses and provides access to key-value pairs in a JSON object from a character stream.
 */
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

  std::unordered_map<std::string, std::shared_ptr<JSONValueStream>> _map; /**< Map of parsed key-value pairs. */
  std::shared_ptr<JSONValueStream> _currentStream; /**< Current key/value stream being parsed. */
  ParserState _parserState = ParserState::START;   /**< Current parser state. */
  std::shared_ptr<CharStream> _charStream;         /**< Underlying character stream. */

  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  /**
   * @brief Construct a JSONStream from a character stream.
   *
   * @param charStream The character stream to parse.
   */
  JSONStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  /**
   * @brief Get the value stream for a given key.
   *
   * @param key The key to look up.
   * @return Shared pointer to the value stream, or nullptr if not found.
   */
  std::shared_ptr<JSONValueStream> get_value(const std::string& key);

  /**
   * @brief Check if parsing is finished.
   *
   * @return True if finished, false otherwise.
   */
  bool finished() const override;

  /**
   * @brief Parse the JSON object from the stream.
   *
   * @return True if parsing is complete, false otherwise.
   */
  bool parse() override;

  /**
   * @brief Serialize the object to a JSON string.
   *
   * @param nesting Indentation level for pretty printing.
   * @return JSON string representation.
   */
  std::string to_json_string(int nesting = 0) const override;

  /**
   * @brief Convert the object to a nlohmann::json object.
   *
   * @return JSON object representation.
   */
  nlohmann::json to_json() const override;
};

/**
 * @brief Stream parser for JSON strings.
 *
 * Parses and provides access to a JSON string value from a character stream.
 */
class JSONStringStream : public JSONValueStream {
  std::optional<int> _startIdx;              /**< Index of the starting quote, if found. */
  std::optional<int> _endIdx;                /**< Index of the ending quote, if found. */
  std::shared_ptr<CharStream> _charStream;   /**< Underlying character stream. */

 private:
  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  /**
   * @brief Construct a JSONStringStream from a character stream.
   *
   * @param charStream The character stream to parse.
   */
  JSONStringStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  JSONStringStream(JSONStringStream&) = delete;
  JSONStringStream(JSONStringStream&&) = delete;

  /**
   * @brief Parse the JSON string from the stream.
   *
   * @return True if parsing is complete, false otherwise.
   */
  bool parse() override;

  /**
   * @brief Check if parsing is finished.
   *
   * @return True if finished, false otherwise.
   */
  bool finished() const override;

  /**
   * @brief Get the parsed string value.
   *
   * @return The parsed string, or empty string if not parsed.
   */
  std::string to_string() const;

  /**
   * @brief Serialize the string to a JSON string.
   *
   * @param nesting Indentation level for pretty printing.
   * @return JSON string representation.
   */
  std::string to_json_string(int nesting = 0) const override;

  /**
   * @brief Convert the string to a nlohmann::json object.
   *
   * @return JSON string value.
   */
  nlohmann::json to_json() const override;
};

/**
 * @brief Stream parser for JSON arrays.
 *
 * Parses and provides access to array elements from a character stream.
 */
class JSONArrayStream : public JSONValueStream {
 private:
  enum class ParserState { START, CREATE_VALUE_STREAM, PARSING_VALUE, PARSED_VALUE, FINISH };

  std::shared_ptr<CharStream> _charStream;   /**< Underlying character stream. */
  ParserState _parserState = ParserState::START; /**< Current parser state. */
  std::vector<std::shared_ptr<JSONValueStream>> _values; /**< Parsed value streams for array elements. */

  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  /**
   * @brief Construct a JSONArrayStream from a character stream.
   *
   * @param charStream The character stream to parse.
   */
  JSONArrayStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  /**
   * @brief Parse the JSON array from the stream.
   *
   * @return True if parsing is complete, false otherwise.
   */
  bool parse() override;

  /**
   * @brief Check if parsing is finished.
   *
   * @return True if finished, false otherwise.
   */
  bool finished() const override;

  /**
   * @brief Serialize the array to a JSON string.
   *
   * @param nesting Indentation level for pretty printing.
   * @return JSON string representation.
   */
  std::string to_json_string(int nesting = 0) const override;

  /**
   * @brief Convert the array to a nlohmann::json object.
   *
   * @return JSON array representation.
   */
  nlohmann::json to_json() const override;

  /**
   * @brief Get the number of elements in the array.
   *
   * @return Number of elements.
   */
  int size() const noexcept;

  /**
   * @brief Get the value stream at a given index.
   *
   * @param idx Index of the element.
   * @return Shared pointer to the value stream at idx.
   */
  std::shared_ptr<JSONValueStream> get_idx(int idx) const;
};

/**
 * @brief Stream parser for JSON numbers.
 *
 * Parses and provides access to a JSON number value from a character stream.
 */
class JSONNumberStream : public JSONValueStream {
 private:
  std::shared_ptr<CharStream> _charStream;   /**< Underlying character stream. */
  std::optional<int> _startIdx;              /**< Index of the start of the number, if found. */
  std::optional<int> _endIdx;                /**< Index of the end of the number, if found. */

  std::shared_ptr<CharStream> char_stream() override { return _charStream; }

 public:
  /**
   * @brief Construct a JSONNumberStream from a character stream.
   *
   * @param charStream The character stream to parse.
   */
  JSONNumberStream(std::shared_ptr<CharStream> charStream) : _charStream(charStream) {}

  /**
   * @brief Parse the JSON number from the stream.
   *
   * @return True if parsing is complete, false otherwise.
   */
  bool parse() override;

  /**
   * @brief Check if parsing is finished.
   *
   * @return True if finished, false otherwise.
   */
  bool finished() const override;

  /**
   * @brief Serialize the number to a JSON string.
   *
   * @param nesting Indentation level for pretty printing.
   * @return JSON string representation.
   */
  std::string to_json_string(int nesting = 0) const override;

  /**
   * @brief Convert the number to a nlohmann::json object.
   *
   * @return JSON number value.
   */
  nlohmann::json to_json() const override;

  /**
   * @brief Get the parsed number as a value of type T.
   *
   * @tparam T The numeric type to parse as.
   * @return The parsed number as type T.
   */
  template <typename T>
  T get_number() const;
};

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
