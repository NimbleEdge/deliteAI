/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "script_load_job.hpp"

#include <stdexcept>

#include "asset_download_job.hpp"
#include "command_center.hpp"
#include "logger.hpp"
#include "resource_loader.hpp"

Job<Location>::Status ScriptLoadJob::process() {
  LOG_TO_DEBUG("%s", "Script Load happening");
  auto assetLocation = _scriptLocationFuture.produce_value();
  _scriptAsset->locationOnDisk = assetLocation;

  try {
    if (!_commandCenter->get_resource_loader().load_task(_scriptAsset)) {
      LOG_TO_CLIENT_ERROR("Unable to load script");
    }
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Error in loading script: %s", e.what());
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Unknown error in loading script");
  }

  return Job::Status::COMPLETE;
}

void ScriptLoadJob::init() {
  LOG_TO_DEBUG("%s", "Inside init of Script Load Job");
  auto childJob = std::make_shared<AssetDownloadJob>(_scriptAsset, _commandCenter);
  this->add_child_job(childJob);
  _scriptLocationFuture = childJob->init();

  // Ignoring the future returned by this job, as it doesn't throw
  static_cast<void>(_commandCenter->job_scheduler()->add_job(shared_from_this()));
}
