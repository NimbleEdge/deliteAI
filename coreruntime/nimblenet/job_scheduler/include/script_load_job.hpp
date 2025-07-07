/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "asset_manager.hpp"
#include "future.hpp"
#include "job.hpp"
#include "ne_fwd.hpp"

/**
 * @brief A job responsible for loading a delitepy script asset.
 *
 * It inherits from `Job<void>` as it does not produce a result value.
 */
class ScriptLoadJob : public Job<void> {
  CommandCenter* _commandCenter;
  std::shared_ptr<Asset> _scriptAsset;
  ne::Future<Location> _scriptLocationFuture;

 public:
  ScriptLoadJob(std::shared_ptr<Asset> scriptAsset, CommandCenter* commandCenter_)
      : Job("ScriptLoadJob") {
    _scriptAsset = scriptAsset;
    _commandCenter = commandCenter_;
  };

  void init();

  Job::Status process() override;
};
