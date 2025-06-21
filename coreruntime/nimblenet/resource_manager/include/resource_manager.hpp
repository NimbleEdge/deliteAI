/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>
#include <string>

#include "config_manager.hpp"
#include "resource_manager_structs.hpp"
#include "server_api_structs.hpp"

class CommandCenter;

class ResourceManager {
  CommandCenter* _commandCenter;

  std::map<std::string, int> _loadResourceRetries;
  std::map<std::string, bool> _resourcesDownloaded;
  std::string get_plan_from_device(const std::string& modelId, const ModelMetadata& metadata);
  PlanData download_plan_metadata(const std::string& modelId, const ModelMetadata& metadata);
  std::string download_plan(const std::string& modelId, const ModelMetadata& metadata);
  std::string save_plan_metadata_on_device(const std::string& modelId, const PlanData& planData);

  PlanData get_plan_metadata_from_device(const std::string& modelId, const ModelMetadata& metadata);
  void set_resource_downloaded(const std::string& modelId);
#ifdef SCRIPTING
  int load_task(const std::string& taskId);
#endif

 public:
  bool can_resource_retry(const std::string& modelId);
  void update_resource_retries(const std::string& resourceId);

  bool is_resource_downloaded(const std::string& modelId);

  PlanData get_plandata_from_device(const std::string& modelId, const ModelMetadata& metadata);
  void reset_model_retries(const std::string& modelId);
  PlanData update_plan_if_required(const std::string& modelId, const ModelMetadata& metadata);
  void remove_plan_from_device(const std::string& modelId, const ModelMetadata& metadata);

  ResourceManager(CommandCenter* commandCenter);

#ifdef SCRIPTING
  TaskResponse update_task_if_required(const std::string& taskId, const TaskMetadata& metadata);
  bool save_task_on_device(const std::string& taskId, const TaskResponse& taskResponse);
  TaskResponse load_task_from_device(const std::string& taskId, const TaskMetadata& metadata);

#endif

  // #ifdef SIMULATION_MODE
  static PlanData get_inference_plan_data_from_device(const char* modelFilePath,
                                                      const char* inferenceConfigFilePath);

  static PlanData get_inference_plan_data_from_device(const char* modelFilePath);

  // #endif
};
