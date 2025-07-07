/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "resource_manager.hpp"

#include <sys/stat.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <tuple>
#include <utility>

#include "command_center.hpp"
#include "logger.hpp"
#include "native_interface.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann/json_fwd.hpp"
#include "resource_manager_constants.hpp"
#include "server_api.hpp"
#include "util.hpp"

using namespace std;

using json = nlohmann::json;

#define PARTSIZE 1000000

ResourceManager::ResourceManager(CommandCenter* commandCenter) { _commandCenter = commandCenter; }

PlanData ResourceManager::get_plandata_from_device(const std::string& modelId,
                                                   const ModelMetadata& metadata) {
  PlanData planData = get_plan_metadata_from_device(modelId, metadata);
  if (!planData.valid) return PlanData();
  planData.planFileName = get_plan_from_device(modelId, metadata);
  if (planData.planFileName == "") return PlanData();
  return planData;
}

bool ResourceManager::is_resource_downloaded(const std::string& modelId) {
  if (_resourcesDownloaded.find(modelId) != _resourcesDownloaded.end()) {
    return true;
  }
  return false;
}

void ResourceManager::set_resource_downloaded(const std::string& modelId) {
  _resourcesDownloaded[modelId] = true;
}

std::string ResourceManager::get_plan_from_device(const std::string& modelId,
                                                  const ModelMetadata& metadata) {
  std::string fileName = modelId + metadata.version + rmconstants::InferenceFileName;
  if (nativeinterface::get_file_size_common(fileName)) {
    return nativeinterface::get_full_file_path_common(fileName);
  }
  return "";
}

std::string ResourceManager::save_plan_metadata_on_device(const std::string& modelId,
                                                          const PlanData& planData) {
  return nativeinterface::save_file_on_device_common(nlohmann::json(planData).dump(),
                                                     modelId + planData.version +
                                                         to_string(planData.epConfigVersion) +
                                                         rmconstants::InferenceMetadataFileName);
}

PlanData ResourceManager::get_plan_metadata_from_device(const std::string& modelId,
                                                        const ModelMetadata& metadata) {
  std::string planMetadataString;
  if (!nativeinterface::get_file_from_device_common(modelId + metadata.version +
                                                        to_string(metadata.epConfigVersion) +
                                                        rmconstants::InferenceMetadataFileName,
                                                    planMetadataString))
    return PlanData();
  return jsonparser::get<PlanData>(planMetadataString);
}

void ResourceManager::remove_plan_from_device(const std::string& modelId,
                                              const ModelMetadata& metadata) {
  std::string metadataFile = nativeinterface::get_full_file_path_common(
      modelId + metadata.version + to_string(metadata.epConfigVersion) +
      rmconstants::InferenceMetadataFileName);
  int didRemove = remove(metadataFile.c_str());
  if (didRemove) {
    LOG_TO_ERROR("%s could not be removed from the system. Failed with error %d",
                 metadataFile.c_str(), didRemove);
  }
  std::string planFile = nativeinterface::get_full_file_path_common(modelId + metadata.version +
                                                                    rmconstants::InferenceFileName);
  didRemove = remove(planFile.c_str());
  if (didRemove) {
    LOG_TO_ERROR("%s could not be removed from the system. Failed with error %d", planFile.c_str(),
                 didRemove);
  }
}

void ResourceManager::reset_model_retries(const std::string& modelId) {
  _loadResourceRetries[modelId] = rmconstants::LoadResourceRetries;
}

void ResourceManager::update_resource_retries(const std::string& resourceId) {
  if (_loadResourceRetries.find(resourceId) != _loadResourceRetries.end()) {
    _loadResourceRetries[resourceId]--;
    if (_loadResourceRetries[resourceId] < 0) {
      LOG_TO_DEBUG("No retries for resourceId=%s left.", resourceId.c_str());
    }
  }
}

bool ResourceManager::can_resource_retry(const std::string& resourceId) {
  if (_loadResourceRetries.find(resourceId) != _loadResourceRetries.end()) {
    return (_loadResourceRetries[resourceId] > 0);
  } else {
    // this is the check before first try
    _loadResourceRetries[resourceId] = rmconstants::LoadResourceRetries;
    return (_loadResourceRetries[resourceId] > 0);
  }
}
#ifdef SCRIPTING

bool ResourceManager::save_task_on_device(const std::string& taskId,
                                          const TaskResponse& taskResponse) {
  return nativeinterface::compress_and_save_file_on_device(
      nlohmann::json(taskResponse).dump(),
      taskId + taskResponse.version + rmconstants::TaskDataFileName);
}

TaskResponse ResourceManager::load_task_from_device(const std::string& taskId,
                                                    const TaskMetadata& metadata) {
  const auto [success, taskResponseString] = nativeinterface::read_potentially_compressed_file(
      taskId + metadata.version + rmconstants::TaskDataFileName);
  if (!success) {
    return TaskResponse();
  }
  TaskResponse ret = jsonparser::get<TaskResponse>(taskResponseString);
  if (!ret.valid)
    LOG_TO_ERROR("Could not parse taskResponse from file on device taskId=%s version=%s",
                 taskId.c_str(), metadata.version.c_str());
  return ret;
}

#endif

PlanData ResourceManager::get_inference_plan_data_from_device(const char* modelFilePath,
                                                              const char* inferenceConfigFilePath) {
  PlanData planData;
  string planMetadatastring;
  if (!nativeinterface::get_file_from_device_common(inferenceConfigFilePath, planMetadatastring,
                                                    true)) {
    return planData;
  }
  planData.inferenceConfig = planMetadatastring;
  planData.valid = true;
  planData.planFileName = modelFilePath;
  return planData;
}

PlanData ResourceManager::get_inference_plan_data_from_device(const char* modelFilePath) {
  PlanData planData;
  planData.planFileName = modelFilePath;
  planData.valid = true;
  return planData;
}
