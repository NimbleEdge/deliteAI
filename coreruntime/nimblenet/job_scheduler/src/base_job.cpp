/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "base_job.hpp"

#include <cassert>
#include <memory>
#include <mutex>

#include "core_utils/fmt.hpp"
#include "logger.hpp"

BaseJob::BaseJob(const std::string& name) : _name(name) {}

// Somehow need to provide an implementation even if the destructor is pure virtual
// https://stackoverflow.com/questions/630950/pure-virtual-destructor-in-c
BaseJob::~BaseJob() {}

void BaseJob::add_child_job(std::shared_ptr<BaseJob> job) {
  std::scoped_lock lock(_mutex, job->_mutex);

  switch (_state) {
    case State::PENDING:
      // Fine, can add dependencies at this state
      break;
    case State::WAITING_FOR_DEPENDENCIES_TO_FINISH:
    case State::SCHEDULED:
    case State::FINISHED:
      THROW("Cannot add child jobs after job is added to scheduler");
  }

  LOG_VERBOSE("Adding %s (state %d) as child of %s (state %d)", job->_name.c_str(), job->_state,
              _name.c_str(), _state);

  // Child job is allowed to be in any state for ease of use.
  // So here we check if the child job is completed or not
  // If the job has finished, we don't need to update any dependencies
  if (job->_state != State::FINISHED) {
    assert(job->_parentJob == nullptr && "There should be no existing parent job");
    job->_parentJob = get_shared_ptr();
    _numPendingChildJobs++;
  }
}
