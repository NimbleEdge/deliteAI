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

class ResourceDownloader {
 public:
  static inline constexpr int LoadResourceRetries = 3;

 private:
  CommandCenter* _commandCenter;
  // This map does not take any lock coz right now everything is downloaded on background thread. If
  // system is made multi threaded we need to take lock on this
  std::map<AssetId, bool> _download_queued_map;

  // Maybe not needed. Asset Manager itself should specify the naming convention for every module or
  // file type, so that its easier
  std::string get_file_name_from_asset(std::shared_ptr<Asset> asset) {
    // return std::to_string(asset.type) + asset.name + asset.version + ".txt";
    return "";
  }

  bool is_download_queued(std::shared_ptr<Asset> asset) const {
    auto mapFind = _download_queued_map.find(asset->get_Id());
    if (mapFind != _download_queued_map.end()) {
      return mapFind->second;
    }
    return false;
  };

 public:
  ResourceDownloader(CommandCenter* commandCenter_) { _commandCenter = commandCenter_; };

  std::optional<Location> get_asset_offline(std::shared_ptr<Asset> asset);

  InternetJob<Location>::Status enqueue_download_asset(std::shared_ptr<Asset> asset);
};
