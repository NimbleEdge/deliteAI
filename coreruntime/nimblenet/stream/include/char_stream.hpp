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

/*
 * CharStream is a stream of characters, AND MORE.
 * It handles asynchronous production of characters and aids in parsing the generated stream
 * into an organised data structure
 *
 * Asynchronous Production:
 * Data into a CharStream can be pushed explicitly via the push() function or CharStream can poll
 * for data via the _producerFunction. Setting the latter enables functionality like the CharStream
 * being able to wait for more data.
 *
 * CharStream Subscribers:
 * A subscriber function can be set in CharStream which is called whenever new data is pushed into
 * the stream. The subscriber function can use pop()/peek() functions to consume the stream.
 * Currently, only one subscriber is there. Hence, only one index is maintained for the pop() and
 * peek() functions to return.
 */

class CharStream : public std::enable_shared_from_this<CharStream> {
  // The subscriber will get called whenever new stuff is added to the stream
  using SubscriberFunction = std::function<void(void)>;

  std::string _stream;
  SubscriberFunction _subscriberFunction;  // empty by default i.e. no subscriber
  int _unparsedIdx = 0;
  bool _closed = false;

 public:
  static std::shared_ptr<CharStream> construct();
  void push(std::string_view str);
  void push(char c);
  void close();  // close the stream, after having pushed all data into it
  void set_subscriber(SubscriberFunction subscriberFunction);
  CharStreamView get_stream_view(int startIdx, int endIdx = -1) const;
  int size() const;

  // functions for getting new data from the stream
  bool empty() const;
  char pop();
  char peek();
  bool closed() const;
  std::optional<char> pop_next_non_ws();
  std::optional<char> pop_ws_and_peek();
  int last_seen_idx();

 private:
  friend class CharStreamView;

  CharStream() = default;
  std::string get_string(int startIdx, std::optional<int> endIdx) const;
  std::string_view get_string_view(int startIdx, std::optional<int> endIdx) const;
  void assert_producer_function();
};

// Denotes a potentially open ended interval over the stream. Reason why we can't do it with just a
// string_view is because every push can potentially invalidate the string inside CharStream That's
// why this will always generate a new string instead of giving a view since that can be invalidated
// very easily and cause problems later
class CharStreamView {
  std::shared_ptr<const CharStream> _stream;
  int _startIdx;
  std::optional<int> _endIdx;

 public:
  std::string to_string() { return _stream->get_string(_startIdx, _endIdx); }

 private:
  friend class CharStream;
  friend class JSONNumberStream;

  CharStreamView(std::shared_ptr<const CharStream> stream, int startIdx, std::optional<int> endIdx)
      : _stream(stream), _startIdx(startIdx), _endIdx(endIdx) {}

  // Getting a string_view out of a string that will be pushed to is a thing that needs to be done
  // very responsibly, so this method is private and only accessible by trusted friend classes
  std::string_view to_string_view() { return _stream->get_string_view(_startIdx, _endIdx); }
};
