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

/**
 * @brief Template class for typed asynchronous jobs.
 *
 * This class extends BaseJob and provides typed result handling using `std::promise<T>`.
 *
 * @tparam T The return type of the job result.
 */
template <typename T>
class Job : public BaseJob, public std::enable_shared_from_this<Job<T>> {
 protected:
  std::promise<T> _jobPromise; /**< Promise object used to store the result or exception. */
  friend class JobScheduler;   /**< Scheduler has internal access to manage execution. */

 public:
  using Status = BaseJob::Status; /**< Status type alias for convenience. */

  /**
   * @brief Constructor for Job.
   *
   * @param name Name of the job, passed to BaseJob for tracking.
   */
  Job(const std::string& name) : BaseJob(name) {}

  /**
   * @brief Entry point called by the scheduler to run the job.
   *
   * Calls the user-implemented `process()` function, captures exceptions, and
   * sets the appropriate promise state.
   *
   * @return The job's final execution status.
   */
  [[nodiscard]] virtual Status process_base_job() override;

  /**
   * @brief Pure virtual function that must be implemented by derived job classes.
   *
   * This function should contain the actual business logic of the job.
   *
   * @return Job status after execution.
   */
  [[nodiscard]] virtual Status process() = 0;

  /**
   * @brief Returns a shared pointer to this instance.
   *
   * @return A `shared_ptr<BaseJob>` pointing to this job.
   */
  [[nodiscard]] std::shared_ptr<BaseJob> get_shared_ptr() override;
};

// ********************* Function implementations **************************

/**
 * @brief Base job processing logic implementation.
 *
 * Handles error capture and promise setting. If the job type is `void`,
 * sets the promise without any value. If an exception occurs during
 * processing, sets the exception into the promise and marks the job as complete.
 */
template <typename T>
typename Job<T>::Status Job<T>::process_base_job() {
  Status status;
  try {
    status = process();
    if constexpr (std::is_same_v<T, void>) {
      if (status == Status::COMPLETE) {
        _jobPromise.set_value();  // No return value for void-type jobs
      }
    }
  } catch (...) {
    _jobPromise.set_exception(std::current_exception());
    status = Status::COMPLETE;  // Even on failure, mark as "complete" to allow progression
  }
  return status;
}

/**
 * @brief Return a shared pointer to this job instance.
 */
template <typename T>
std::shared_ptr<BaseJob> Job<T>::get_shared_ptr() {
  return Job<T>::shared_from_this();
}
