/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "resource_downloader.hpp"

#include <optional>

#include "asset_manager.hpp"
#include "command_center.hpp"
#include "internet_job.hpp"
#include "native_interface.hpp"
#include "nimble_net_util.hpp"
#include "server_api.hpp"

InternetJob<Location>::Status ResourceDownloader::enqueue_download_asset(
    std::shared_ptr<Asset> asset) {
  FileDownloadStatus fileDownloadStatus;
  switch (asset->type) {
    case AssetType::SCRIPT: {
      auto assetResponse = _commandCenter->get_serverAPI()->get_asset(asset);
      if (assetResponse.has_value()) {
        auto fileName = asset->get_file_name_on_device();
        auto isTrue = nativeinterface::write_compressed_data_on_file(
            std::move(assetResponse.value()), fileName);
        if (isTrue) {
          return InternetJob<Location>::Status::COMPLETE;
        }
      }
      return InternetJob<Location>::Status::RETRY;
    }
#ifdef GENAI
    case AssetType::LLM: {
#ifdef GEMINI
      if (asset->osProvided) {
        fileDownloadStatus = nativeinterface::check_os_llm_status();
        break;
      }
#endif  // GEMINI
      fileDownloadStatus = _commandCenter->get_serverAPI()->get_llm(asset);
      break;
    }
    case AssetType::DOCUMENT:
#endif  // GENAI
    case AssetType::MODEL: {
      fileDownloadStatus = _commandCenter->get_serverAPI()->get_asset_async(asset);
      break;
    }
#ifdef GENAI
    case AssetType::RETRIEVER:
      THROW("%s", "Can't download a RETRIEVER directly, this shouldn't have been called");
#endif  // GENAI
  }
  switch (fileDownloadStatus) {
    case FileDownloadStatus::DOWNLOAD_SUCCESS:
      return InternetJob<Location>::Status::COMPLETE;
    case FileDownloadStatus::DOWNLOAD_PAUSED:
    case FileDownloadStatus::DOWNLOAD_PENDING:
    case FileDownloadStatus::DOWNLOAD_RUNNING:
      return InternetJob<Location>::Status::POLL;
    case FileDownloadStatus::DOWNLOAD_FAILURE:
    case FileDownloadStatus::DOWNLOAD_UNKNOWN:
      return InternetJob<Location>::Status::RETRY;
  }
};

std::optional<Location> ResourceDownloader::get_asset_offline(std::shared_ptr<Asset> asset) {
  auto fileName = asset->get_file_name_on_device();
  switch (asset->type) {
    case AssetType::MODEL:
    case AssetType::SCRIPT:
#ifdef GENAI
    case AssetType::DOCUMENT:
#endif  // GENAI
    {
      if (nativeinterface::file_exists_common(fileName)) {
        return {fileName};
      }
      return {};
    }
#ifdef GENAI
    case AssetType::RETRIEVER:
      // How do I get this offline?
      return {};
    case AssetType::LLM: {
#ifdef GEMINI
      if (asset->name == rmconstants::GeminiModelName) {
        return {};
      }
#endif  // GEMINI
      if (nativeinterface::folder_exists_common(fileName)) {
        return {fileName};
      }
      return {};
    }
#endif  // GENAI
  }
}
