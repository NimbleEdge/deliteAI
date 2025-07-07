/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "job_scheduler.hpp"

#include <cstddef>
#include <memory>
#include <mutex>

#include "base_job.hpp"
#include "logger.hpp"

JobScheduler::JobScheduler(std::size_t capacity)
    : _jobs(capacity), _priorityJobs(capacity), _jobsWaitingForInternet() {}

void JobScheduler::notify_online() {
  std::lock_guard lockGuard(_internetJobsMutex);
  for (auto& job : _jobsWaitingForInternet) {
    _jobs.push(std::move(job));
  }
  _jobsWaitingForInternet.clear();
}

// Runs the jobs which are initially present in the queue
// Useful when called from the background thread
void JobScheduler::do_jobs() {
  // NOTE: SPSCQueue returns nullptr when the queue is empty. This is unlike std::queue which would
  // segfault
  while (_priorityJobs.front()) {
    auto job = std::move(*_priorityJobs.front());
    _priorityJobs.pop();
    do_job(std::move(job), true);
  }

  auto jobsCount = _jobs.size();
  while (jobsCount > 0) {
    auto job = std::move(*_jobs.front());
    _jobs.pop();
    do_job(std::move(job), false);
    jobsCount--;
  }

  append_jobs_back_to_queue();
}

// Runs all the jobs till the queue is empty
// Useful when trying to load the assets from main thread
void JobScheduler::do_all_non_priority_jobs() {
  while (_jobs.size() > 0) {
    auto job = std::move(*_jobs.front());
    _jobs.pop();
    do_job(std::move(job), false);
  }

  append_jobs_back_to_queue();
}

void JobScheduler::do_job(std::shared_ptr<BaseJob>&& job, bool isPriority) {
  std::shared_ptr<BaseJob> parentJob;
  {
    std::lock_guard lock{job->_mutex};
    assert(job->_numPendingChildJobs == 0);
    BaseJob::Status status;

    LOG_VERBOSE("Starting to do job %s", job->_name.c_str());
    status = job->process_base_job();
    LOG_VERBOSE("Got result %d from job %s", static_cast<int>(status), job->_name.c_str());

    switch (status) {
      case BaseJob::Status::RETRY:
        _attemptedJobs.push(job);
        return;
      case BaseJob::Status::COMPLETE:
        job->_state = BaseJob::State::FINISHED;
        break;
      case BaseJob::Status::RETRY_WHEN_ONLINE:
        assert(!isPriority && "Priority internet jobs not supported");
        queue_internet_waiting_job(std::move(job));
        return;
    }

    parentJob = job->_parentJob;
  }

  if (parentJob) {
    {
      std::lock_guard lock{parentJob->_mutex};
      LOG_VERBOSE("BaseJob %s has parent job %s with pending child jobs %d", job->_name.c_str(),
                  parentJob->_name.c_str(), parentJob->_numPendingChildJobs);
      parentJob->_numPendingChildJobs--;
      assert(parentJob->_numPendingChildJobs >= 0);

      switch (parentJob->_state) {
        case BaseJob::State::PENDING:
          // Parent job is in pending state, do nothing
          return;
        case BaseJob::State::WAITING_FOR_DEPENDENCIES_TO_FINISH:
          if (parentJob->_numPendingChildJobs == 0) {
            parentJob->_state = BaseJob::State::SCHEDULED;
            LOG_VERBOSE("Updated state of parent job %s to SCHEDULED (%d)",
                        parentJob->_name.c_str(), static_cast<int>(parentJob->_state));
            // Call do_job after the lock is released
          } else {
            LOG_VERBOSE(
                "Parent job %s, in WAITING_FOR_DEPENDENCIES (%d) state, still has %d pending child "
                "jobs",
                parentJob->_name.c_str(), static_cast<int>(parentJob->_state),
                parentJob->_numPendingChildJobs);
            return;
          }
          break;
        case BaseJob::State::SCHEDULED:
        case BaseJob::State::FINISHED:
          THROW(
              "ILLEGAL STATE: Parent job was in state %d before its pending child job had "
              "completed",
              static_cast<int>(parentJob->_state));
      }
    }
    // Release lock before calling do_job
    do_job(std::move(parentJob), isPriority);
  }
}

void JobScheduler::add_job(std::shared_ptr<BaseJob>&& job, bool isPriority) {
  std::lock_guard lock{job->_mutex};
  LOG_VERBOSE("Adding job %s, isPriority %d, parent job %s, state %d", job->_name.c_str(),
              isPriority, (job->_parentJob ? job->_parentJob->_name.c_str() : "NA"),
              static_cast<int>(job->_state));

  if (job->_state == BaseJob::State::PENDING) {
    if (job->_numPendingChildJobs > 0) {
      job->_state = BaseJob::State::WAITING_FOR_DEPENDENCIES_TO_FINISH;
      LOG_VERBOSE("Updated state of job %s to WAITING_FOR_DEPENDENCIES (%d)", job->_name.c_str(),
                  static_cast<int>(job->_state));
      return;
    }
    job->_state = BaseJob::State::SCHEDULED;
    LOG_VERBOSE("Updated state of job %s to SCHEDULED (%d)", job->_name.c_str(),
                static_cast<int>(job->_state));
  } else {
    LOG_VERBOSE("BaseJob %s already in state %d, probably getting re-added to the queue",
                job->_name.c_str(), static_cast<int>(job->_state));
  }

  if (isPriority) {
    _priorityJobs.push(std::move(job));
  } else {
    _jobs.push(std::move(job));
  }
}

void JobScheduler::append_jobs_back_to_queue() {
  while (_attemptedJobs.size() != 0) {
    auto job = _attemptedJobs.front();
    _attemptedJobs.pop();
    add_job(std::move(job), false);
  }
}

void JobScheduler::queue_internet_waiting_job(std::shared_ptr<BaseJob>&& job) {
  LOG_VERBOSE("BaseJob %s queued waiting for internet", job->_name.c_str());
  std::lock_guard lockGuard{_internetJobsMutex};
  _jobsWaitingForInternet.push_back(std::move(job));
}
