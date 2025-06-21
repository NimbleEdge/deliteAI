/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "job.hpp"
#include "locked_mpmc_queue.hpp"

/*
 * Asynchronously perform jobs
 * Add jobs from one thread, which are stored in a SPSCQueue. Another thread can perform jobs by
 * popping them off the queue. Currently all jobs are assumed to be the highest priority and they'll
 * be performed all at once
 *
 * We can add many things to this abstraction: Having different priorities for jobs, having a
 * timeout after which we stop doing jobs, etc
 */
class JobScheduler {
  // Queue of non-priority jobs that are ready to run i.e. don't have any pending dependencies
  // NOTE: This is a MPSC queue so jobs can be added by multiple threads, but only a single thread
  // can pop jobs from this queue. This works as the job scheduler runs on a single thread
  ne::LockedMPSCQueue<std::shared_ptr<BaseJob>> _jobs;
  // Jobs that have to be added back to the _jobs queue. Used in do_jobs() function
  std::queue<std::shared_ptr<BaseJob>> _attemptedJobs;
  // Queue of priority jobs that are ready to run i.e. don't have any pending dependencies
  ne::LockedMPSCQueue<std::shared_ptr<BaseJob>> _priorityJobs;

  // Here, producer is the thread on which scheduler runs and consumer will be the
  // external thread from which frontend gives indication that internet is switched on
  std::mutex _internetJobsMutex;
  std::vector<std::shared_ptr<BaseJob>> _jobsWaitingForInternet;

 public:
  JobScheduler(std::size_t capacity);

  // Push jobs waiting for internet back into the queue
  void notify_online();

  // Push job onto the queue. Blocks if the queue is full
  template <typename T>
  [[nodiscard]] std::future<T> add_job(std::shared_ptr<Job<T>> job);

  template <typename T>
  [[nodiscard]] std::future<T> add_priority_job(std::shared_ptr<Job<T>> job);

  // Performs all jobs in the queue, exits when the queue is empty

  void do_jobs();
  void do_all_non_priority_jobs();

 private:
  void do_job(std::shared_ptr<BaseJob>&& job, bool isPriority);
  void add_job(std::shared_ptr<BaseJob>&& job, bool isPriority);
  void queue_internet_waiting_job(std::shared_ptr<BaseJob>&& job);
  void append_jobs_back_to_queue();
};

/*************************** Template function implementations ********************************/

template <typename T>
std::future<T> JobScheduler::add_job(std::shared_ptr<Job<T>> job) {
  add_job(job, false);
  return job->_jobPromise.get_future();
}

template <typename T>
std::future<T> JobScheduler::add_priority_job(std::shared_ptr<Job<T>> job) {
  add_job(job, true);
  return job->_jobPromise.get_future();
}
