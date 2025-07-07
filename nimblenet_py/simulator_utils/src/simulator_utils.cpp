/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "simulator_utils.hpp"

#include "asset_manager.hpp"
#include "core_sdk_structs.hpp"
#include "native_interface.hpp"
#include "server_api_structs.hpp"

void SimulatorUtils::create_symlink(const fs::path& target, const std::string& link) {
  std::string targetStr = target.string();
  try {
    // Ignoring the return value, since we don't care if link existed or not
    static_cast<void>(fs::remove(link));
    fs::create_symlink(fs::absolute(target), link);
  } catch (const fs::filesystem_error& e) {
    THROW("Could not create symlink from %s to %s with error: %s", targetStr.c_str(), link.c_str(),
          e.what());
  } catch (...) {
    THROW("Could not create symlink from %s to %s", targetStr.c_str(), link.c_str());
  }
}

void SimulatorUtils::copy_module(const std::shared_ptr<Asset> asset, Deployment& deployment,
                                 bool addToDeployment) {
  switch (asset->type) {
    case AssetType::SCRIPT: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      std::string taskCodeString = parseScriptToAST(asset->location.path);
      nativeinterface::write_data_to_file(std::move(taskCodeString), outputFilePath);
      if (addToDeployment) deployment.script = asset;
      break;
    }
    case AssetType::MODEL: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      SimulatorUtils::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
#ifdef GENAI
    case AssetType::RETRIEVER: {
      for (const auto& asset : asset->arguments) {
        copy_module(asset, deployment, false);
      }
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
    case AssetType::DOCUMENT: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      SimulatorUtils::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
    case AssetType::LLM: {
      std::string outputFilePath =
          nativeinterface::get_full_file_path_common(asset->get_file_name_on_device());
      SimulatorUtils::create_symlink(fs::absolute(asset->location.path), outputFilePath);
      if (addToDeployment) deployment.modules.push_back(asset);
      break;
    }
#endif
    default:
      THROW("AssetType %s not supported in simulator.",
            assetmanager::get_string_from_asset_type(asset->type).c_str());
  }
}

void SimulatorUtils::copy_modules(nlohmann::json&& moduleConfig) {
  nativeinterface::HOMEDIR = "./NimbleSDK/";
  if (!nativeinterface::create_folder(nativeinterface::HOMEDIR)) {
    THROW("Could not create directory %s", nativeinterface::HOMEDIR.c_str());
  }
  Deployment deployment = {.Id = 1};
  for (const auto& it : moduleConfig) {
    std::shared_ptr<Asset> asset = assetmanager::parse_module_info(it);
    copy_module(asset, deployment, true);
  }
  // Write deployment_config to disk
  util::save_deployment_on_device(deployment, coresdkconstants::DefaultCompatibilityTag);
}
