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

/**
 * @brief This class writes a given string to a character stream at a specified rate.
 *
 * Writing is offloaded to another thread which writes to an internal buffer, since CharStream is not thread safe.
 * To pull characters out of the internal buffer and into the character stream, the process() function needs to be called.
 */
class DummyOffloadedStream {
  using Queue = rigtorp::SPSCQueue<char>;

  /**
   * @brief Producer thread that pushes characters into the internal queue at a fixed rate.
   */
  class ProducerThread {
   public:
    std::string _sourceString;              /**< The source string to be streamed. */
    std::size_t _nextIdx = 0;               /**< Index of the next character to push. */
    std::size_t _sleepAfterCharMicros;      /**< Microseconds to sleep after inserting a character. */
    std::shared_ptr<Queue> _internalQueue;  /**< Shared pointer to the internal character queue. */

    /**
     * @brief Function call operator to run the producer thread.
     *
     * @param keepProcessing Atomic flag to control thread execution.
     */
    void operator()(const std::atomic<bool>& keepProcessing);
  };

  /**
   * NOTE: Order is important here, as _streamPushThread needs to be constructed after _internalQueue is constructed.
   */
  
  std::shared_ptr<Queue> _internalQueue;      /**< Internal single-producer single-consumer queue. */
  std::atomic<bool> _runProducerThread = true;/**< Flag to control the producer thread's execution. */
  std::thread _streamPushThread;              /**< Thread that pushes characters into the internal queue. */
  std::shared_ptr<CharStream> _charStream;    /**< The character stream to which data is ultimately written. */

 public:
  /**
   * @brief Construct a DummyOffloadedStream.
   *
   * @param str The string to stream.
   * @param charsPerSec The rate (characters per second) at which to stream.
   * @param bufferSize The size of the internal buffer.
   * @param task Shared pointer to the Task for job scheduling.
   */
  DummyOffloadedStream(const std::string& str, std::size_t charsPerSec, std::size_t bufferSize,
                       std::shared_ptr<Task> task);

  /**
   * @brief Destructor. Stops the producer thread and cleans up resources.
   */
  ~DummyOffloadedStream();

  /**
   * @brief Extract the character stream to attach subscribers into.
   *
   * @return Shared pointer to the CharStream.
   */
  std::shared_ptr<CharStream> char_stream() const noexcept { return _charStream; }
};
