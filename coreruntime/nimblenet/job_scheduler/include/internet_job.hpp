/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "job.hpp"

/**
 * @brief A job that may depend on internet connectivity.
 *
 * This class defines a two-phase job:
 * -  Offline pre-check or fallback (`process_offline`)
 * - A main task that may require internet connectivity (`process_with_internet`)
 *
 * The job retries a configurable number of times before deferring until internet is available.
 *
 * @tparam T The result type of the job.
 */
template <typename T>
class InternetJob : public Job<T> {
 public:
  /**
   * @brief Possible statuses returned by `process_with_internet`.
   */
  enum class Status {
    COMPLETE, /**< Job finished successfully */
    RETRY,    /**< Retry soon (connectivity or transient failure) */
    POLL      /**< Poll for result repeatedly without full retry */
  };

 private:
  int _retryCount = 0;        /**< Current retry attempt count remaining. */
  const int _maxRetries = 0;  /**< Max number of retries before deferring to when-online. */
  bool _offlineTried = false; /**< Whether `process_offline()` has been attempted. */

 public:
  /**
   * @brief Constructor for an internet-capable job.
   *
   * @param name        Name of the job.
   * @param maxRetries  Maximum number of retry attempts allowed before deferring.
   */
  InternetJob(const std::string& name, int maxRetries);

  /**
   * @brief Logic to attempt the job without internet, i.e. check if the asset is already present on
   * disk.
   *
   * @note Called once before any online attempt.
   *
   * @return Job status.
   */
  [[nodiscard]] virtual typename Job<T>::Status process_offline() = 0;

  /**
   * @brief Logic to perform the job with internet access.
   *
   * Called repeatedly if offline mode fails and until maxRetries is hit.
   *
   * @return InternetJob::Status indicating result or what to do next.
   */
  virtual Status process_with_internet() = 0;

 private:
  /**
   * @brief Internal dispatch that wraps offline and online logic.
   *
   * First attempts offline logic once, then online logic repeatedly with retry/poll behavior.
   *
   * @return Final Job status to guide the scheduler.
   */
  typename Job<T>::Status process() final;
};

/************************** Template Function Definitions ****************************/

template <typename T>
InternetJob<T>::InternetJob(const std::string& name, int maxRetries)
    : Job<T>(name), _maxRetries(maxRetries), _retryCount(maxRetries) {}

template <typename T>
typename Job<T>::Status InternetJob<T>::process() {
  // Try offline method once before switching to online attempts
  if (!_offlineTried) {
    _offlineTried = true;
    const auto status = process_offline();
    return status;
  }
  const auto status = process_with_internet();
  switch (status) {
    case Status::POLL:
      // Re-poll the job without decrementing retry counter
      return Job<T>::Status::RETRY;
    case Status::RETRY:
      // Decrement retry counter, failover to "RETRY_WHEN_ONLINE" if exhausted
      _retryCount--;
      if (_retryCount == 0) {
        _retryCount = _maxRetries;
        return Job<T>::Status::RETRY_WHEN_ONLINE;
      } else {
        return Job<T>::Status::RETRY;
      }

    case Status::COMPLETE:
      // Task completed successfully
      return Job<T>::Status::COMPLETE;
  }
}
