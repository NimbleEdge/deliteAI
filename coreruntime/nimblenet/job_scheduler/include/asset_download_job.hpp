/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "asset_manager.hpp"
#include "future.hpp"
#include "internet_job.hpp"
#include "ne_fwd.hpp"

class AssetDownloadJob : public InternetJob<Location> {
  CommandCenter* _commandCenter;
  std::shared_ptr<Asset> _asset;

 public:
  AssetDownloadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter);
  [[nodiscard]] ne::Future<Location> init();
  InternetJob::Status process_with_internet() override;
  Job::Status process_offline() override;
};
