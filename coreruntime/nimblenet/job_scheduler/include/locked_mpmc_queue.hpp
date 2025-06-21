/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <mutex>

#include "rigtorp/SPSCQueue.h"

namespace ne {

template <typename T, bool MultiProducer, bool MultiConsumer>
class LockedGenericQueue {
  rigtorp::SPSCQueue<T> _queue;
  std::mutex _producerMutex;
  std::mutex _consumerMutex;

 public:
  template <typename... Ts>
  explicit LockedGenericQueue(Ts...);
  bool empty() const noexcept;
  [[nodiscard]] T* front() noexcept;

  size_t size() const noexcept { return _queue.size(); };

  void pop() noexcept;
  template <typename P>
  void push(P&& v);
};

template <typename T>
using LockedMPSCQueue = LockedGenericQueue<T, true, false>;
template <typename T>
using LockedMPMCQueue = LockedGenericQueue<T, true, true>;
template <typename T>
using LockedSPMCQueue = LockedGenericQueue<T, false, true>;
template <typename T>
using SPSCQueue = LockedGenericQueue<T, false, false>;
// ^This will have no performance overhead over rigtorp::SPSCQueue, except for storing two mutexes
// which will never get used

}  // namespace ne

/*****************************Function impls*********************************/

#define DO_WITH_PRODUCER_LOCK(S)                             \
  do {                                                       \
    if constexpr (MultiProducer) {                           \
      std::lock_guard<std::mutex> lockGuard{_producerMutex}; \
      S;                                                     \
    } else {                                                 \
      S;                                                     \
    }                                                        \
  } while (0)

#define DO_WITH_CONSUMER_LOCK(S)                             \
  do {                                                       \
    if constexpr (MultiConsumer) {                           \
      std::lock_guard<std::mutex> lockGuard{_consumerMutex}; \
      S;                                                     \
    } else {                                                 \
      S;                                                     \
    }                                                        \
  } while (0)

namespace ne {

template <typename T, bool MultiProducer, bool MultiConsumer>
template <typename... Ts>
LockedGenericQueue<T, MultiProducer, MultiConsumer>::LockedGenericQueue(Ts... ts)
    : _queue(std::forward<Ts>(ts)...) {}

template <typename T, bool MultiProducer, bool MultiConsumer>
bool LockedGenericQueue<T, MultiProducer, MultiConsumer>::empty() const noexcept {
  return _queue.empty();
}

template <typename T, bool MultiProducer, bool MultiConsumer>
T* LockedGenericQueue<T, MultiProducer, MultiConsumer>::front() noexcept {
  DO_WITH_CONSUMER_LOCK(return _queue.front());
}

template <typename T, bool MultiProducer, bool MultiConsumer>
void LockedGenericQueue<T, MultiProducer, MultiConsumer>::pop() noexcept {
  DO_WITH_CONSUMER_LOCK(return _queue.pop());
}

template <typename T, bool MultiProducer, bool MultiConsumer>
template <typename P>
void LockedGenericQueue<T, MultiProducer, MultiConsumer>::push(P&& v) {
  DO_WITH_PRODUCER_LOCK(return _queue.push(std::forward<P>(v)));
}

}  // namespace ne