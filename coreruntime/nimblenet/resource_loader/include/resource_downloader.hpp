/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <optional>

#include "asset_manager.hpp"
#include "internet_job.hpp"
#include "ne_fwd.hpp"

/**
 * @class ResourceDownloader
 * @brief Manages the downloading and local retrieval of assets.
 * files.
 *
 * This class coordinates background downloading of assets and provides synchronous or asynchronous
 * access to those resources. It interacts with the AssetManager to determine what needs to be
 * fetched and maintains internal state to prevent redundant downloads.
 */
class ResourceDownloader {
 public:
  // Maximum number of retries to attempt when loading/downloading a resource
  static inline constexpr int LoadResourceRetries = 3;

 private:
  CommandCenter* _commandCenter;

  /**
   * @brief Tracks which assets are currently queued for download to avoid duplication.
   *
   * NOTE: This map is accessed only from a background thread. If the system evolves into a
   * multi-threaded download system, a mutex will be needed to protect this state.
   */
  std::map<AssetId, bool> _download_queued_map;

  /**
   * @brief Checks whether a download for the given asset has already been queued.
   *
   * @param asset Pointer to the Asset object.
   * @return true if already queued, false otherwise.
   */
  bool is_download_queued(std::shared_ptr<Asset> asset) const {
    auto mapFind = _download_queued_map.find(asset->get_Id());
    if (mapFind != _download_queued_map.end()) {
      return mapFind->second;
    }
    return false;
  };

 public:
  /**
   * @brief Constructs a ResourceDownloader instance.
   *
   * @param commandCenter_
   */
  ResourceDownloader(CommandCenter* commandCenter_) { _commandCenter = commandCenter_; };

  /**
   * @brief Attempts to retrieve an asset from local storage (offline).
   *
   * @param asset The asset to retrieve.
   * @return A Location if the asset is found locally; std::nullopt otherwise.
   */
  std::optional<Location> get_asset_offline(std::shared_ptr<Asset> asset);

  /**
   * @brief Queues the download of a given asset if not already in progress.
   *
   * @param asset The asset to download.
   * @return The status of the enqueued InternetJob (e.g., AlreadyExists, Enqueued).
   */
  InternetJob<Location>::Status enqueue_download_asset(std::shared_ptr<Asset> asset);
};
