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
 * Jobs form a tree, with parent-child relationship indicating dependency
 * A parent Job can only be scheduled once all its children Jobs are completed
 * Each job maintains a count of number of pending child jobs as well as a pointer
 * to parent job
 *
 * NOTE: We might want to make Job's tree into a DAG in the future. In that case,
 * every job will just hold a vector of its parent jobs
 */

class BaseJob {
 public:
  enum class Status { COMPLETE, RETRY, RETRY_WHEN_ONLINE };

  /*
   * Job has a state, in which certain operations are allowed or disallowed:
   * - PENDING: Job is not scheduled. Can add dependencies at this time
   * - WAITING_FOR_DEPENDENCIES_TO_FINISH: Job added to the scheduler, but not queued as it is
   *   waiting for child jobs to finish. More dependencies cannot be added
   * - SCHEDULED: Job added to scheduler queue, no pending dependencies. More dependencies
   *   cannot be added
   * - FINISHED: Job is finished, i.e. it returned COMPLETE status on a run() call
   */
  enum class State { PENDING, WAITING_FOR_DEPENDENCIES_TO_FINISH, SCHEDULED, FINISHED };

 private:
  std::mutex _mutex;  // Protect job state variables
  State _state = State::PENDING;
  int _numPendingChildJobs = 0;
  std::shared_ptr<BaseJob> _parentJob;
  const std::string _name;

  friend class JobScheduler;

 protected:
  // Inheriting classes should ensure this is always constructed as a shared_ptr
  BaseJob(const std::string& name);
  [[nodiscard]] virtual std::shared_ptr<BaseJob> get_shared_ptr() = 0;

 public:
  [[nodiscard]] virtual Status process_base_job() = 0;

  virtual ~BaseJob() = 0;

  void add_child_job(std::shared_ptr<BaseJob> job);
};