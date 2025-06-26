/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <thread>

#include "char_stream.hpp"
#include "rigtorp/SPSCQueue.h"
#include "stream_producer.hpp"

class Task;

// This class will write a given string to a character stream at a particular rate
// Writing part is offloaded to another thread which writes to an internal buffer. This is because
// CharStream is not thread safe. To pull characters out of the internal buffer and into the
// character stream, process() function needs to be called
class DummyOffloadedStream {
  using Queue = rigtorp::SPSCQueue<char>;

  class ProducerThread {
   public:
    std::string _sourceString;
    std::size_t _nextIdx = 0;
    std::size_t _sleepAfterCharMicros;  // micros to sleep after inserting a char

    std::shared_ptr<Queue> _internalQueue;

    void operator()(const std::atomic<bool>& keepProcessing);
  };

  // NOTE: Order is important here, as _streamPushThread needs to be constructed
  // after _internalQueue is constructed
  std::shared_ptr<Queue> _internalQueue;
  std::atomic<bool> _runProducerThread = true;
  std::thread _streamPushThread;
  std::shared_ptr<CharStream> _charStream;

 public:
  DummyOffloadedStream(const std::string& str, std::size_t charsPerSec, std::size_t bufferSize,
                       std::shared_ptr<Task> task);
  ~DummyOffloadedStream();

  // Extract the character stream to attach subscribers into
  std::shared_ptr<CharStream> char_stream() const noexcept { return _charStream; }
};
