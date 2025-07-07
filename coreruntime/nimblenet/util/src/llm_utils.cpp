/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "llm_utils.hpp"

#include <cstring>

#include "asset_manager.hpp"
#include "data_variable.hpp"
#include "native_interface.hpp"
#include "resource_manager_constants.hpp"
#include "server_api.hpp"

namespace llmutil {

// This function fetches the device tier config from the backend asset
// 0. First check if the file exists on device, if yes, parse it
// 1. Get the deployments from the command center
// 2. Loop through the deployments and check for asset type DOCUMENT
// 3. For and asset with type DOCUMENT, look at metadata and select an
//    asset with synchronous attribute
// 4. Fetch the asset using the get_asset() synchronous call
// 5. Store it in a location on device
std::string get_historical_benchmarks(CommandCenter* commandCenter) {
  const auto& deployment = commandCenter->get_deployment();
  std::shared_ptr<ServerAPI> serverAPI = commandCenter->get_serverAPI();
  std::shared_ptr<Asset> module =
      deployment.get_module(rmconstants::MobileBenchmarksAssetName, AssetType::DOCUMENT);
  if (!module) {
    return "{}";
  }
  try {
    const auto modelBufferOpt =
        nativeinterface::read_potentially_compressed_file(module->get_file_name_on_device(), false);
    const auto successfulRead = modelBufferOpt.first;
    if (successfulRead) {
      return modelBufferOpt.second;
    }
    // file not found, download
    LOG_TO_DEBUG("Benchmarks file not found, attempting to download");

    int attempt = 0;
    std::optional<std::string> result;
    while (attempt < serverconstants::MaxRegisterRetries) {
      // download the asset through a blocking call
      result = serverAPI->get_asset(module);
      if (result.has_value()) {
        break;
      }
      attempt++;
    }
    if (!result.has_value()) {
      LOG_TO_ERROR("Failed to download benchmarks file");
      THROW("Failed to download benchmarks file");
    }
    std::string value = result.value();
    // store the result on device
    nativeinterface::write_compressed_data_on_file(std::move(value),
                                                   module->get_file_name_on_device());

    return result.value();
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Error processing module metadata: %s", e.what());
    return "{}";
  }

  LOG_TO_ERROR("No suitable benchmarks document found in deployment");
  return "{}";
}

DeviceTierConfig from_raw_json(const std::string& jsonStr) {
  DeviceTierConfig config;
  try {
    auto json = nlohmann::json::parse(jsonStr);
    // Parse tier1
    config.tier1.min_ram = json["tier_config"]["tier_1"]["min_ram"];
    config.tier1.min_num_cores = json["tier_config"]["tier_1"]["min_num_cores"];
    config.tier1.min_multi_core_score = json["tier_config"]["tier_1"]["min_multi_core_score"];

    // Parse tier2
    config.tier2.min_ram = json["tier_config"]["tier_2"]["min_ram"];
    config.tier2.min_num_cores = json["tier_config"]["tier_2"]["min_num_cores"];
    config.tier2.min_multi_core_score = json["tier_config"]["tier_2"]["min_multi_core_score"];

    // Parse historical benchmarks
    for (const auto& benchmark : json["historical_benchmarks"]) {
      DeviceTierConfig::Benchmark b;
      b.device = benchmark["device"];
      b.chipset = benchmark["chipset"];
      b.multi_core_score = benchmark["multi_core_score"];
      config.historical_benchmarks.push_back(b);
    }
  } catch (const std::exception& e) {
    THROW("Could not parse benchmarks file");
  }
  return config;
}

std::map<std::string, std::string> get_device_info() {
  std::map<std::string, std::string> device_info;
  char* raw_hw_info = get_hardware_info();
  if (!raw_hw_info) {
    return device_info;
  }
  // Get hardware info and parse into map
  std::string hardware_info = std::string(raw_hw_info);
  auto device_info_var =
      DataVariable::get_map_from_json_object(nlohmann::json::parse(hardware_info));
  for (const auto& [key, value] : device_info_var->get_map()) {
    device_info[key] = value->print();
  }
  // there was a strdup() in the get_hardware_info() call, we need to free the allocated memory
  free(raw_hw_info);
  return device_info;
}

DeviceTier get_device_tier(CommandCenter* commandCenter) {
  std::map<std::string, std::string> device_info = get_device_info();
  if (device_info.empty()) {
    return DeviceTier::UNSUPPORTED;
  }
  // Extract device info from map
  std::string device_name = device_info["deviceModel"];
  std::string device_chipset = device_info["chipset"];
  int device_num_cores = std::stoi(device_info["numCores"]);
  int device_ram = std::stoi(device_info["totalRamInMB"]) / 1024;  // Convert MB to GB

  std::string device_tier_config_string = get_historical_benchmarks(commandCenter);
  if (device_tier_config_string.empty() || device_tier_config_string == "{}") {
    // Handle empty JSON gracefully
    return DeviceTier::UNSUPPORTED;
  }

  // Get device tier config
  DeviceTierConfig config;

  config = llmutil::from_raw_json(device_tier_config_string);

  // Check historical benchmarks
  for (const auto& benchmark : config.historical_benchmarks) {
    if (strcasecmp(benchmark.device.c_str(), device_name.c_str()) == 0 ||
        strcasecmp(benchmark.chipset.c_str(), device_chipset.c_str()) == 0) {
      if (benchmark.multi_core_score >= config.tier1.min_multi_core_score) {
        return DeviceTier::ONE;
      }
      if (benchmark.multi_core_score >= config.tier2.min_multi_core_score) {
        return DeviceTier::TWO;
      }
    }
  }

  // Check RAM and CPU cores
  if (device_ram >= config.tier1.min_ram && device_num_cores >= config.tier1.min_num_cores) {
    return DeviceTier::ONE;
  }
  if (device_ram >= config.tier2.min_ram && device_num_cores >= config.tier2.min_num_cores) {
    return DeviceTier::TWO;
  }

  return DeviceTier::UNSUPPORTED;
}

std::optional<std::string> get_os_supported_llm(const std::string& device_model) {
  return nativeinterface::get_os_supported_llm();
}

bool is_supported(const std::string& llm_name, const std::string& device_model,
                  llmutil::DeviceTier tier) {
  // TODO: Update this logic based on our experience of running the LLM
  //       on a certain device / device tier
  return true;
}

std::vector<std::map<std::string, std::string>> get_all_llms(CommandCenter* commandCenter,
                                                             const std::string& device_model,
                                                             llmutil::DeviceTier tier) {
  const auto& deployment = commandCenter->get_deployment();
  std::vector<std::map<std::string, std::string>> llms;

  // get the deployment LLMs
  for (const auto& module : deployment.modules) {
    if (module->type == AssetType::LLM && llmutil::is_supported(module->name, device_model, tier)) {
      std::map<std::string, std::string> llm;
      llm["name"] = module->name;
      llm["provider"] = llmutil::Provider::CUSTOM;
      llms.push_back(llm);
    }
  }
  auto osLLM = llmutil::get_os_supported_llm(device_model);
  if (osLLM.has_value()) {
    // If OS LLM is supported, add it to the list
    std::map<std::string, std::string> llm;
    llm["name"] = osLLM.value();
    llm["provider"] = llmutil::Provider::OS;
    llms.push_back(llm);
  }
  return llms;
}

}  // namespace llmutil
