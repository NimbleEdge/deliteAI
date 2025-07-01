/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "char_stream.hpp"

#include <memory>

#include "core_utils/fmt.hpp"
#include "stream_producer.hpp"

std::shared_ptr<CharStream> CharStream::construct() {
  return std::shared_ptr<CharStream>{new CharStream()};
}

void CharStream::push(std::string_view str) {
  if (_closed) {
    THROW("%s", "Unable to push data to closed char stream");
  }

  if (str.empty()) {
    return;
  }

  _stream.append(str);

  if (_subscriberFunction) {
    _subscriberFunction();
  }
}

void CharStream::push(char c) {
  if (_closed) {
    THROW("%s", "Unable to push data to closed char stream");
  }

  _stream.push_back(c);

  if (_subscriberFunction) {
    _subscriberFunction();
  }
}

void CharStream::close() {
  _closed = true;

  // Inform the subscriber upon stream closure as well
  if (_subscriberFunction) {
    _subscriberFunction();
  }
}

void CharStream::set_subscriber(CharStream::SubscriberFunction subscriberFunction) {
  _subscriberFunction = subscriberFunction;
}

bool CharStream::empty() const { return _unparsedIdx >= _stream.size(); }

char CharStream::pop() { return _stream[_unparsedIdx++]; }

char CharStream::peek() { return _stream[_unparsedIdx]; }

bool CharStream::closed() const { return _closed; }

std::optional<char> CharStream::pop_next_non_ws() {
  auto ret = pop_ws_and_peek();
  if (ret) {
    // pop the character that we're returning
    _unparsedIdx++;
  }
  return ret;
}

std::optional<char> CharStream::pop_ws_and_peek() {
  while (_unparsedIdx < _stream.size()) {
    if (char c = _stream[_unparsedIdx]; !std::isspace(c)) {
      return c;
    }
    _unparsedIdx++;
  }
  return {};
}

int CharStream::last_seen_idx() { return _unparsedIdx - 1; }

int CharStream::size() const { return _stream.size(); }

CharStreamView CharStream::get_stream_view(int startIdx, int endIdx) const {
  std::optional<int> endIdxOpt;
  if (endIdx != -1) endIdxOpt = endIdx;
  return CharStreamView{shared_from_this(), startIdx, endIdxOpt};
}

std::string CharStream::get_string(int startIdx, std::optional<int> endIdx) const {
  return std::string{get_string_view(startIdx, endIdx)};
}

std::string_view CharStream::get_string_view(int startIdx, std::optional<int> endIdx) const {
  std::string_view sv_stream{_stream};
  if (endIdx) {
    return sv_stream.substr(startIdx, endIdx.value() - startIdx + 1);
  } else {
    return sv_stream.substr(startIdx);
  }
}
