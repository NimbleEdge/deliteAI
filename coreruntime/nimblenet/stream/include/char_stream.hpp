/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "job.hpp"

class CharStreamView;
class JSONNumberStream;

/**
 * @brief CharStream manages lifecycle of a character stream.
 *
 * It handles asynchronous production of characters and aids in parsing the generated stream
 * into an organised data structure. Data can be pushed explicitly or produced asynchronously.
 * Subscribers can be notified when new data is available.
 */
class CharStream : public std::enable_shared_from_this<CharStream> {
  // The subscriber will get called whenever new stuff is added to the stream
  using SubscriberFunction = std::function<void(void)>;

  std::string _stream;                  /**< Internal buffer holding the stream data. */
  SubscriberFunction _subscriberFunction; /**< Function called when new data is pushed; empty by default. */
  int _unparsedIdx = 0;                 /**< Index of the next unparsed character in the stream. */
  bool _closed = false;                 /**< Flag indicating whether the stream is closed. */

 public:
  /**
   * @brief Construct a new CharStream instance.
   *
   * @return Shared pointer to the constructed CharStream.
   */
  static std::shared_ptr<CharStream> construct();

  /**
   * @brief Push a string into the stream.
   *
   * @param str The string view to push into the stream.
   */
  void push(std::string_view str);

  /**
   * @brief Push a single character into the stream.
   *
   * @param c The character to push into the stream.
   */
  void push(char c);

  /**
   * @brief Close the stream after all data has been pushed.
   */
  void close();

  /**
   * @brief Set a subscriber function to be called when new data is pushed.
   *
   * @param subscriberFunction The function to call on new data.
   */
  void set_subscriber(SubscriberFunction subscriberFunction);

  /**
   * @brief Get a view of the stream from startIdx to endIdx.
   *
   * @param startIdx The starting index of the view.
   * @param endIdx The ending index of the view (default -1 for end of stream).
   * @return CharStreamView representing the requested interval.
   */
  CharStreamView get_stream_view(int startIdx, int endIdx = -1) const;

  /**
   * @brief Get the total size of the stream.
   *
   * @return The number of characters in the stream.
   */
  int size() const;

  /**
   * @brief Check if the stream is empty (all data consumed).
   *
   * @return True if empty, false otherwise.
   */
  bool empty() const;

  /**
   * @brief Pop the next character from the stream.
   *
   * @return The next character.
   */
  char pop();

  /**
   * @brief Peek at the next character in the stream without consuming it.
   *
   * @return The next character.
   */
  char peek();

  /**
   * @brief Check if the stream has been closed.
   *
   * @return True if closed, false otherwise.
   */
  bool closed() const;

  /**
   * @brief Pop and return the next non-whitespace character, skipping whitespace.
   *
   * @return The next non-whitespace character, or std::nullopt if none.
   */
  std::optional<char> pop_next_non_ws();

  /**
   * @brief Skip whitespace and peek at the next character.
   *
   * @return The next non-whitespace character, or std::nullopt if none.
   */
  std::optional<char> pop_ws_and_peek();

  /**
   * @brief Get the index of the last seen character.
   *
   * @return The last seen index.
   */
  int last_seen_idx();

 private:
  friend class CharStreamView;

  CharStream() = default;

  /**
   * @brief Get a string from the stream between startIdx and endIdx.
   *
   * @param startIdx The starting index.
   * @param endIdx The optional ending index.
   * @return The substring from startIdx to endIdx.
   */
  std::string get_string(int startIdx, std::optional<int> endIdx) const;

  /**
   * @brief Get a string_view from the stream between startIdx and endIdx.
   *
   * @param startIdx The starting index.
   * @param endIdx The optional ending index.
   * @return The string_view from startIdx to endIdx.
   */
  std::string_view get_string_view(int startIdx, std::optional<int> endIdx) const;
  
  /**
   * @brief Assert that a producer function is set for the stream.
   *
   * Used internally to ensure that a producer function exists before attempting to use it.
   */
  void assert_producer_function();
};

/**
 * @brief Creates a string view over the character stream.
 *
 * This class generates a new string for each view to avoid invalidation issues from stream mutation.
 */
class CharStreamView {
  std::shared_ptr<const CharStream> _stream; /**< Shared pointer to the underlying CharStream. */
  int _startIdx;                            /**< Start index of the view in the stream. */
  std::optional<int> _endIdx;               /**< Optional end index of the view in the stream. */

 public:
  /**
   * @brief Convert the view to a string.
   *
   * @return The string representation of the view.
   */
  std::string to_string() { return _stream->get_string(_startIdx, _endIdx); }

 private:
  friend class CharStream;
  friend class JSONNumberStream;

  /**
   * @brief Construct a CharStreamView from a stream and interval.
   *
   * @param stream The CharStream to view.
   * @param startIdx The starting index.
   * @param endIdx The optional ending index.
   */
  CharStreamView(std::shared_ptr<const CharStream> stream, int startIdx, std::optional<int> endIdx)
      : _stream(stream), _startIdx(startIdx), _endIdx(endIdx) {}

  /**
   * @brief Convert the view to a string_view (private, for trusted classes only).
   *
   * @return The string_view representation of the view.
   */
  std::string_view to_string_view() { return _stream->get_string_view(_startIdx, _endIdx); }
};
