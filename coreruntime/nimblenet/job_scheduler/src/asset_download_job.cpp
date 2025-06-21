/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asset_download_job.hpp"

#include "asset_manager.hpp"
#include "command_center.hpp"
#include "internet_job.hpp"
#include "resource_downloader.hpp"
#include "resource_loader.hpp"

AssetDownloadJob::AssetDownloadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter)
    : InternetJob(ne::fmt("AssetDownloadJob[%s:%d]", asset->name.c_str(), asset->type).str,
                  ResourceDownloader::LoadResourceRetries) {
  _commandCenter = commandCenter;
  _asset = asset;
  LOG_TO_DEBUG("Creating asset download job for %s, type %d", _asset->name.c_str(), _asset->type);
}

ne::Future<Location> AssetDownloadJob::init() {
#ifdef GEMINI
  if (_asset->osProvided) {
    nativeinterface::initialize_os_llm();
  }
#endif  // GEMINI
  auto future = _commandCenter->job_scheduler()->add_job(shared_from_this());
  return ne::Future<Location>(std::move(future));
}

InternetJob<Location>::Status AssetDownloadJob::process_with_internet() {
  LOG_VERBOSE("Starting Asset Download Job for %s, type %d", _asset->name.c_str(), _asset->type);
  auto& downloader = _commandCenter->get_resource_downloader();
  const auto status = downloader.enqueue_download_asset(_asset);
  if (status == InternetJob::Status::COMPLETE) {
    _jobPromise.set_value(_asset->get_file_name_on_device());
  }
  return status;
};

Job<Location>::Status AssetDownloadJob::process_offline() {
  LOG_VERBOSE("Starting offline Asset Download Job for %s, type %d", _asset->name.c_str(),
              _asset->type);
  auto& downloader = _commandCenter->get_resource_downloader();
  auto assetLocation = downloader.get_asset_offline(_asset);
  if (assetLocation.has_value()) {
    _jobPromise.set_value(assetLocation.value());
    return Job::Status::COMPLETE;
  } else {
    return Job::Status::RETRY;
  }
}
