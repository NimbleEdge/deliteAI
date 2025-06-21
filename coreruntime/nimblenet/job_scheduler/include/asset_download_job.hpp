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

/**
 * @brief Job responsible for downloading an asset, with offline fallback and retry support.
 *
 * This class wraps the logic to download an `Asset` and return its `Location`.
 * It inherits from `InternetJob<Location>`, enabling it to:
 *  - Attempt offline resolution of the asset first.
 *  - Download the asset from the internet with automatic retries
 *
 * The result of the job is a `Location` object representing where the asset is stored on disk.
 */
class AssetDownloadJob : public InternetJob<Location> {
  CommandCenter* _commandCenter;
  std::shared_ptr<Asset> _asset;

 public:
  AssetDownloadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter);
  [[nodiscard]] ne::Future<Location> init();
  InternetJob::Status process_with_internet() override;
  Job::Status process_offline() override;
};
