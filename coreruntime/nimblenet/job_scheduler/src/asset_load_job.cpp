/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asset_load_job.hpp"

#include <memory>

#include "asset_download_job.hpp"
#include "core_sdk_structs.hpp"
#include "future_data_variable.hpp"
#include "logger.hpp"
#include "model_nimble_net_variable.hpp"

AssetLoadJob::AssetLoadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter_)
    : Job<OpReturnType>(ne::fmt("AssetLoadJob[%s:%d]", asset->name.c_str(), asset->type).str) {
  _asset = asset;
  _commandCenter = commandCenter_;
}

Job<OpReturnType>::Status AssetLoadJob::process() {
  LOG_TO_DEBUG(
      "Started processing Asset Load since Asset Download is completed for asset %s, type %d",
      _asset->name.c_str(), _asset->type);
  if (_arguments.size() == 0) {
    // This is a single asset with no dependencies
    if (!_locationFuture.is_ready()) {
      // ideally this should never happen, process function should have been called if future was
      // true;
      return Job::Status::RETRY;
    }
    auto location = _locationFuture.produce_value();
    _asset->locationOnDisk = location;
  } else {
    for (int i = 0; i < _arguments.size(); i++) {
      auto future = std::dynamic_pointer_cast<FutureDataVariable>(_arguments[i]);
      if (!future) {
        THROW("%s", "Expected argument of Asset to be a future");
      }
      if (!future->is_available()) {
        THROW("%s", "Process called of AssetLoadJob but argument future is not available yet");
      }
      _arguments[i] = future->get();
    }
  }
  // Do NOT load assets when new commandCenter is being constructed, just set the promise to
  // NoneVariable, so that isReady still returns true and we save the newCommandCenter deployment on
  // device
  if (!_commandCenter->is_current()) {
    _jobPromise.set_value(std::make_shared<NoneVariable>());
    return Job::Status::COMPLETE;
  }

  auto dataVariable = _commandCenter->get_resource_loader().load_asset(_asset, _arguments);
  if (dataVariable == nullptr) {
    return Job::Status::RETRY;
  }
  _jobPromise.set_value(dataVariable);
  LOG_TO_DEBUG("Loaded Asset %s, type %d via new flow ", _asset->name.c_str(), _asset->type);
  return Job::Status::COMPLETE;
}

std::shared_ptr<FutureDataVariable> AssetLoadJob::init() {
  LOG_TO_DEBUG("Init Asset load Job for asset %s, type %d", _asset->name.c_str(), _asset->type);
  auto deployment = _commandCenter->get_deployment();

  if (_asset->arguments.size() > 0) {
    // This has dependencies, first need to load them
    for (auto childAsset : _asset->arguments) {
      auto childLoadJob = std::make_shared<AssetLoadJob>(childAsset, _commandCenter);
      this->add_child_job(childLoadJob);
      _arguments.push_back(childLoadJob->init());
    }
  } else {
    auto childJob = std::make_shared<AssetDownloadJob>(_asset, _commandCenter);
    this->add_child_job(childJob);
    _locationFuture = childJob->init();
  }

  auto fut = _commandCenter->job_scheduler()->add_job(shared_from_this());
  return std::make_shared<FutureDataVariable>(std::move(fut), _asset->name, shared_from_this(),
                                              _commandCenter->is_task_initializing());
}

OpReturnType AssetLoadJob::fetch(std::shared_ptr<Asset> asset, CommandCenter* commandCenter) {
  auto loadJob = std::make_shared<AssetLoadJob>(asset, commandCenter);
  return loadJob->init();
}
