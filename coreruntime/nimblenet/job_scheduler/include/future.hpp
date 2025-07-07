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

/**
 * @brief A wrapper around std::shared_future<T> to provide basic future handling.
 *
 * This class is designed to be used in the job system to check readiness and retrieve results
 * from asynchronous computations.
 *
 * @tparam T The type of the result produced by the future.
 */
template <typename T>
class Future {
  std::shared_future<T> _sharedFuture; /**< Internal shared future to allow multiple consumers. */

 public:
  /**
   * @brief Construct a Future from a std::future<T>.
   *
   * Converts a standard future to a shared future so it can be accessed multiple times.
   *
   * @param future The std::future to wrap.
   */
  Future(std::future<T> future) { _sharedFuture = std::shared_future<T>(future.share()); }

  /**
   * @brief Default constructor. Initializes an empty future.
   */
  Future(){};

  /**
   * @brief Check if the future's result is ready.
   *
   * @return true if the result is available without blocking, false otherwise.
   */
  bool is_ready() {
    auto waitResult = _sharedFuture.wait_for(std::chrono::nanoseconds(0));
    if (waitResult == std::future_status::ready) return true;
    return false;
  }

  /**
   * @brief Retrieve the value from the future.
   *
   * This call blocks if the value is not yet available. It will throw if the future has
   * already been consumed or is invalid.
   *
   * @return The value produced by the future.
   */
  T produce_value() { return _sharedFuture.get(); }

  // virtual T produce_value() = 0;
  // // Should only be called once, will throw if called again
  // std::future<T> get_future();
};
}  // namespace ne
