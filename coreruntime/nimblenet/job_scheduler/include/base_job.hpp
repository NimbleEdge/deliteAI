/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>

/**
 * @brief Jobs form a tree, with parent-child relationships indicating dependencies.
 *
 * A parent job can only be scheduled once all its child jobs have completed.
 * Each job maintains a count of pending child jobs and an optional pointer to its parent.
 *
 * @note In the future, the job structure could be extended into a DAG (directed acyclic graph),
 *       in which case a job may hold multiple parent references.
 */

class BaseJob {
 public:
  /**
   * @brief Status codes returned after processing a job.
   */
  enum class Status {
    COMPLETE,         /**< Job successfully completed. */
    RETRY,            /**< Job failed and should be retried. */
    RETRY_WHEN_ONLINE /**< Retry only when network or online state is restored. */
  };

  /**
   * @brief States a job can be in throughout its lifecycle.
   *
   * - PENDING: Not yet scheduled; can add dependencies.
   * - WAITING_FOR_DEPENDENCIES_TO_FINISH: Scheduled but waiting for child jobs.
   * - SCHEDULED: Ready to execute; all dependencies resolved.
   * - FINISHED: Execution complete (status COMPLETE).
   */
  enum class State { PENDING, WAITING_FOR_DEPENDENCIES_TO_FINISH, SCHEDULED, FINISHED };

 private:
  std::mutex _mutex;                   /**< Mutex to protect internal state transitions. */
  State _state = State::PENDING;       /**< Current state of the job. */
  int _numPendingChildJobs = 0;        /**< Counter for unfinished child jobs. */
  std::shared_ptr<BaseJob> _parentJob; /**< Pointer to the parent job (if any). */
  const std::string _name;             /**< Job name for identification/debugging. */

  friend class JobScheduler; /**< Grants scheduler internal access to state. */

 protected:
  /**
   * @brief Constructor to be called by derived classes.
   *
   * @param name Name of the job for tracking and debugging.
   *
   * @note Must always be constructed as a `shared_ptr` to function correctly.
   */
  BaseJob(const std::string& name);

  /**
   * @brief Returns a shared pointer to the current job instance.
   *
   * @return shared_ptr of the job (typically `shared_from_this()`).
   */
  [[nodiscard]] virtual std::shared_ptr<BaseJob> get_shared_ptr() = 0;

 public:
  /**
   * @brief Processes the job logic. Called by the scheduler.
   *
   * @return Job status after execution.
   */
  [[nodiscard]] virtual Status process_base_job() = 0;

  /**
   * @brief Virtual destructor for safe cleanup of derived types.
   */
  virtual ~BaseJob() = 0;

  /**
   * @brief Registers another job as a child/dependent of this job.
   *
   * @param job The dependent job to be added.
   */
  void add_child_job(std::shared_ptr<BaseJob> job);
};