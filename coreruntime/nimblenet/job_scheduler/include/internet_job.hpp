/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "job.hpp"

template <typename T>
class InternetJob : public Job<T> {
 public:
  enum class Status { COMPLETE, RETRY, POLL };

 private:
  int _retryCount = 0;
  const int _maxRetries = 0;
  bool _offlineTried = false;

 public:
  InternetJob(const std::string& name, int maxRetries);

  [[nodiscard]] virtual typename Job<T>::Status process_offline() = 0;

  virtual Status process_with_internet() = 0;

 private:
  typename Job<T>::Status process() final;
};

/************************** Template Function Definitions ****************************/

template <typename T>
InternetJob<T>::InternetJob(const std::string& name, int maxRetries)
    : Job<T>(name), _maxRetries(maxRetries), _retryCount(maxRetries) {}

template <typename T>
typename Job<T>::Status InternetJob<T>::process() {
  // Try to run internet job in offline mode only once, as the asset might already be present on
  // disk
  if (!_offlineTried) {
    _offlineTried = true;
    const auto status = process_offline();
    return status;
  }
  const auto status = process_with_internet();
  switch (status) {
    case Status::POLL:
      return Job<T>::Status::RETRY;
    case Status::RETRY:
      _retryCount--;
      if (_retryCount == 0) {
        _retryCount = _maxRetries;
        return Job<T>::Status::RETRY_WHEN_ONLINE;
      } else {
        return Job<T>::Status::RETRY;
      }
    case Status::COMPLETE:
      return Job<T>::Status::COMPLETE;
  }
}
