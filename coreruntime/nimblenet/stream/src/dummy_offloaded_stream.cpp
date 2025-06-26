/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dummy_offloaded_stream.hpp"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "char_stream.hpp"
#include "task.hpp"
#include "time_manager.hpp"

void DummyOffloadedStream::ProducerThread::operator()(const std::atomic<bool>& keepProcessing) {
  while (keepProcessing.load(std::memory_order_acquire) && _nextIdx < _sourceString.size()) {
    std::this_thread::sleep_for(std::chrono::microseconds(_sleepAfterCharMicros));

    _internalQueue->push(_sourceString[_nextIdx]);
    _nextIdx++;
  }

  // Signal that the generation is finished
  _internalQueue->push('\0');
}

DummyOffloadedStream::DummyOffloadedStream(const std::string& str, std::size_t charsPerSec,
                                           std::size_t bufferSize, std::shared_ptr<Task> task)
    : _internalQueue{std::make_shared<DummyOffloadedStream::Queue>(bufferSize)},
      _runProducerThread(true),
      _streamPushThread{ProducerThread{._sourceString = str,
                                       ._nextIdx = 0,
                                       ._sleepAfterCharMicros = Time::MICROS_IN_SECS / charsPerSec,
                                       ._internalQueue = _internalQueue},
                        std::cref(_runProducerThread)},
      _charStream{CharStream::construct()} {
  if (!task) {
    THROW("%s", "Task pointer not set");
  }
  auto job = std::make_shared<FillCharStreamJob>(_charStream, _internalQueue);
  task->add_stream_push_job(job);
}

DummyOffloadedStream::~DummyOffloadedStream() {
  _runProducerThread.store(false, std::memory_order_release);
  _streamPushThread.join();
}
