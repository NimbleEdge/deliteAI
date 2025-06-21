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

class AssetLoadJob : public Job<OpReturnType> {
  CommandCenter* _commandCenter;

  // In case of dependencies
  std::vector<OpReturnType> _arguments;

  // In case we have no dependencies
  ne::Future<Location> _locationFuture;
  std::shared_ptr<Asset> _asset;

 public:
  AssetLoadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter_);
  [[nodiscard]] std::shared_ptr<FutureDataVariable> init();
  Job::Status process() override;

  static OpReturnType fetch(std::shared_ptr<Asset> asset, CommandCenter* commandCenter);
};
