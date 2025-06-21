/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <exception>
#include <future>
#include <memory>

#include "job.hpp"

namespace ne {
template <typename T>
class Future {
  std::shared_future<T> _sharedFuture;

 public:
  Future(std::future<T> future) { _sharedFuture = std::shared_future<T>(future.share()); }

  Future(){};

  bool is_ready() {
    auto waitResult = _sharedFuture.wait_for(std::chrono::nanoseconds(0));
    if (waitResult == std::future_status::ready) return true;
    return false;
  }

  T produce_value() { return _sharedFuture.get(); }

  // virtual T produce_value() = 0;
  // // Should only be called once, will throw if called again
  // std::future<T> get_future();
};
}  // namespace ne
