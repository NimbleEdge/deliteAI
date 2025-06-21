/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <exception>
#include <future>
#include <memory>
#include <type_traits>

#include "base_job.hpp"

template <typename T>
class Job : public BaseJob, public std::enable_shared_from_this<Job<T>> {
 protected:
  std::promise<T> _jobPromise;
  friend class JobScheduler;

 public:
  using Status = BaseJob::Status;

  Job(const std::string& name) : BaseJob(name) {}

  [[nodiscard]] virtual Status process_base_job() override;
  [[nodiscard]] virtual Status process() = 0;
  [[nodiscard]] std::shared_ptr<BaseJob> get_shared_ptr() override;
};

// ********************* Function implementations **************************

template <typename T>
typename Job<T>::Status Job<T>::process_base_job() {
  Status status;
  try {
    status = process();
    if constexpr (std::is_same_v<T, void>) {
      if (status == Status::COMPLETE) {
        _jobPromise.set_value();
      }
    }
  } catch (...) {
    _jobPromise.set_exception(std::current_exception());
    status = Status::COMPLETE;
  }
  return status;
}

template <typename T>
std::shared_ptr<BaseJob> Job<T>::get_shared_ptr() {
  return Job<T>::shared_from_this();
}
