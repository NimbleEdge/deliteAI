/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "script_ready_job.hpp"

#include <stdexcept>

#include "command_center.hpp"
#include "logger.hpp"
#include "resource_loader.hpp"

std::future<void> ScriptReadyJob::init() {
  return _commandCenter->job_scheduler()->add_job(shared_from_this());
}

Job<void>::Status ScriptReadyJob::process() {
  LOG_VERBOSE("%s", "Running Script Ready Job");
  auto isReady = _commandCenter->get_task()->is_ready();
  _commandCenter->set_is_ready(isReady);

  if (!_commandCenter->is_current() && isReady) {
    util::rename_deployment_to_old_deployment(_commandCenter->get_config());

    try {
      util::save_deployment_on_device(_commandCenter->get_deployment(),
                                      _commandCenter->get_config()->compatibilityTag);
    } catch (const std::runtime_error& e) {
      LOG_TO_CLIENT_ERROR("Unable to save new deployment to device: %s", e.what());
      return Job::Status::COMPLETE;
    } catch (...) {
      LOG_TO_CLIENT_ERROR("Unknown error in saving new deployment to device");
      return Job::Status::COMPLETE;
    }

    delete_new_command_center();
    LOG_TO_CLIENT_INFO("%s", "New state is saved on device");
  }
  return Job::Status::COMPLETE;
}

void ScriptReadyJob::delete_new_command_center() noexcept {
  delete _commandCenter;
  _commandCenter = nullptr;
}
