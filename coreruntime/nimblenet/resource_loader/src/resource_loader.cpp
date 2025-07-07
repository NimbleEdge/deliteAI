/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "resource_loader.hpp"

#include <memory>

#include "asset_manager.hpp"
#include "core_sdk_structs.hpp"
#include "model_nimble_net_variable.hpp"
#include "native_interface.hpp"

#ifdef GENAI
#include "retriever.hpp"
#endif  // GENAI

using json = nlohmann::json;

#if ONNX_EXECUTOR
#include "onnx_model.hpp"
#include "task_onnx_model.hpp"
typedef class ONNXModel Model;
typedef class TaskONNXModel ModelV2;
#endif  // ONNX_EXECUTOR

ResourceLoader::ResourceLoader(CommandCenter* commandCenter_, bool isCurrentState)
    : _commandCenter(commandCenter_), _isCurrentState(isCurrentState) {}

OpReturnType ResourceLoader::load_model(std::shared_ptr<Asset> asset) {
  nlohmann::json epConfigs = nlohmann::json::object();
  int epConfigVersion = -1;
  // By default always run dummy inference
  bool runDummyInference = true;
  if (!asset->metadata.empty()) {
    if (asset->metadata.contains("epConfigs")) {
      epConfigs = asset->metadata.at("epConfigs");
    }
    if (asset->metadata.contains("epConfigVersion")) {
      epConfigVersion = asset->metadata.at("epConfigVersion");
    }
    if (asset->metadata.contains("runDummyInference")) {
      runDummyInference = asset->metadata.at("runDummyInference");
    }
  }
  try {
    std::shared_ptr<ModelV2> newModel =
        std::make_shared<ModelV2>(asset->locationOnDisk.path, asset->version, asset->name,
                                  epConfigs, epConfigVersion, _commandCenter, runDummyInference);
    return std::make_shared<ModelNimbleNetVariable>(_commandCenter, asset->name, newModel);
  } catch (std::exception& e) {
    THROW("Exception in creating Model for modelId=%s error=%s version=%s", asset->name.c_str(),
          e.what(), asset->version.c_str());
  } catch (...) {
    THROW("Unknown exception in creating Model for modelId=%s version=%s", asset->name.c_str(),
          asset->version.c_str());
  }
};

bool ResourceLoader::load_task(std::shared_ptr<Asset> taskAsset) {
  if (_taskMap.find(taskAsset->get_Id()) != _taskMap.end()) {
    return true;
  }
  auto task = std::make_shared<Task>(_commandCenter, taskAsset);
  if (!task) {
    LOG_TO_ERROR("%s", "Could not load task");
    return false;
  }
  _taskMap[taskAsset->get_Id()] = task;
  _commandCenter->set_task(task);
  return true;
}

#ifdef GENAI
OpReturnType ResourceLoader::load_document(std::shared_ptr<Asset> asset) {
  auto fullFilePath = asset->get_file_name_on_device();
  const auto [success, jsonDocStr] =
      nativeinterface::read_potentially_compressed_file(fullFilePath);
  if (!success) {
    LOG_TO_ERROR("Could not read document %s from path %s", asset->name.c_str(),
                 fullFilePath.c_str());
    return nullptr;
  }

  nlohmann::json j = nlohmann::json::parse(jsonDocStr);
  if (j.is_array()) {
    return DataVariable::get_list_from_json_array(std::move(j));
  }
  return DataVariable::get_map_from_json_object(std::move(j));
}

OpReturnType ResourceLoader::load_retriever(std::shared_ptr<Asset> asset,
                                            const std::vector<OpReturnType>& arguments) {
  if (arguments.size() != 3) {
    THROW("Unable to create Retriever. Expected 3 dependent assets, found %d", arguments.size());
  }

  return std::make_shared<RetrieverDataVariable>(_commandCenter, arguments);
}

OpReturnType ResourceLoader::load_llm(std::shared_ptr<Asset> asset) {
  return std::make_shared<LLMDataVariable>(asset, _commandCenter);
}

#endif  // GENAI

OpReturnType ResourceLoader::load_asset(std::shared_ptr<Asset> asset,
                                        const std::vector<OpReturnType>& arguments) {
  switch (asset->type) {
    case AssetType::SCRIPT:
      THROW("%s", "Script should be loaded through load_task function");
    case AssetType::MODEL:
      return load_model(asset);
#ifdef GENAI
    case AssetType::DOCUMENT:
      return load_document(asset);
    case AssetType::RETRIEVER:
      return load_retriever(asset, arguments);
    case AssetType::LLM:
      return load_llm(asset);
#endif  // GENAI
  }
}
