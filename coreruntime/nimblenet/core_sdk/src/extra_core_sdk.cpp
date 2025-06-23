/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <fstream>
#include <list>
#include <vector>

#include "core_sdk.hpp"
#include "core_sdk_constants.hpp"
#include "data_variable.hpp"
#include "executor_structs.h"
#include "map_data_variable.hpp"
#include "native_interface.hpp"

using namespace std;

bool CoreSDK::load_model_from_file(const char* modelFilePath, const char* inferenceConfigFilePath,
                                   const char* modelId, const char* epConfigJsonChar) {
  if (inferenceConfigFilePath == nullptr || (strcmp(inferenceConfigFilePath, "") == 0)) {
    PlanData planData = ResourceManager::get_inference_plan_data_from_device(modelFilePath);
    planData.epConfigVersion = 1;
    if (epConfigJsonChar != nullptr) {
      planData.executionProviderConfig = nlohmann::json::parse(std::string(epConfigJsonChar));
    }
    throw std::runtime_error("Plan data could not be loaded.");
  }

  PlanData planData =
      ResourceManager::get_inference_plan_data_from_device(modelFilePath, inferenceConfigFilePath);
  planData.epConfigVersion = 1;
  if (epConfigJsonChar != nullptr) {
    planData.executionProviderConfig = nlohmann::json::parse(std::string(epConfigJsonChar));
  }
  throw std::runtime_error("Plan data could not be loaded.");
}

#ifdef SIMULATION_MODE

bool CoreSDK::add_events_from_file(const char* userInputFilePath, const char* tableName) {
  string userInput;
  if (!nativeinterface::get_file_from_device_common(userInputFilePath, userInput, true)) {
    throw std::runtime_error("User input could not be loaded.");
  }

  return add_events_from_buffer(userInput.c_str(), tableName);
}

bool CoreSDK::add_events_from_buffer(const char* userInputBuffer, const char* tableName) {
  nlohmann::json userEvents;
  try {
    userEvents = nlohmann::json::parse(userInputBuffer);
  } catch (json::exception& e) {
    LOG_TO_CLIENT_ERROR("Error in parsing event for table:%s with eventMap: %s with error: %s",
                        tableName, userInputBuffer, e.what());
    return false;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Error in parsing event for table:%s with eventMap: %s.", tableName,
                        userInputBuffer);
    return false;
  }

  if ((userEvents.is_array() || userEvents.is_object()) &&
      add_simulation_user_events(userEvents, tableName)) {
    return true;
  }
  return false;
}

bool CoreSDK::run_task_upto_timestamp(const char* taskName, const char* functionName,
                                      const CTensors input, CTensors* output, int64_t timestamp) {
  // add events till timestamp and call run_task
  if (!add_simulation_user_events_upto_timestamp(timestamp)) {
    throw std::runtime_error("Fetching events upto timestamp=" + std::to_string(timestamp) +
                             " failed!");
  }
  Time::set_time(timestamp);
  auto t = run_task(taskName, functionName, input, output);
  if (t == nullptr) {
    return true;
  }
  auto error = std::string(t->message) + "\nRun Task failed!";
  deallocate_nimblenet_status(t);
  throw std::runtime_error(error);
}

bool CoreSDK::add_simulation_user_events(nlohmann::json& userEventsJson, const string& tableName) {
  try {
    if (userEventsJson.is_object() && validateUserEvent(userEventsJson)) {
      userEventsJson["TableName"] = tableName;
      userEventsJson[coresdkconstants::SimulatedInputType] = coresdkconstants::InputTypeEvent;
      _simulatedUserEvents.push_back(userEventsJson);
    } else if (userEventsJson.is_array()) {
      for (int i = 0; i < userEventsJson.size(); i++) {
        if (validateUserEvent(userEventsJson[i])) {
          userEventsJson[i]["TableName"] = tableName;
          userEventsJson[i][coresdkconstants::SimulatedInputType] =
              coresdkconstants::InputTypeEvent;
        }
      }

      for (int i = 0; i < userEventsJson.size(); i++) {
        _simulatedUserEvents.push_back(userEventsJson[i]);
      }
    }

  } catch (json::exception& e) {
    throw std::runtime_error("Exception in parsing user events: " + std::string(e.what()));
  } catch (const std::exception& e) {
    throw std::runtime_error("Exception in collecting user events: " + std::string(e.what()));
  }
  return true;
}

bool CoreSDK::add_simulation_user_events_upto_timestamp(int64_t timestamp) {
  // Sort the _simulatedUserEvents and then add them
  std::sort(_simulatedUserEvents.begin(), _simulatedUserEvents.end(),
            [](nlohmann::json a, nlohmann::json b) { return a["TIMESTAMP"] < b["TIMESTAMP"]; });

  int index = -1;
  for (int i = 0; i < _simulatedUserEvents.size(); i++) {
    if (_simulatedUserEvents[i]["TIMESTAMP"] <= timestamp) {
      if (_simulatedUserEvents[i][coresdkconstants::SimulatedInputType] ==
          coresdkconstants::InputTypeEvent) {
        Time::set_time(_simulatedUserEvents[i]["TIMESTAMP"]);
        _simulatedUserEvents[i].erase("TIMESTAMP");
        const string& tableName = _simulatedUserEvents[i]["TableName"];
        _simulatedUserEvents[i].erase("TableName");
        _simulatedUserEvents[i].erase(coresdkconstants::SimulatedInputType);

        CUserEventsData cUserEventsData;
        auto status = add_user_event(_simulatedUserEvents[i].dump(), tableName, &cUserEventsData);
        if (status != nullptr) {
          LOG_TO_CLIENT_ERROR("%s", status->message);
          deallocate_nimblenet_status(status);
          return false;
        }
        deallocate_c_userevents_data(&cUserEventsData);
      }
      index = i;
    }
  }

  // If able to add all the events upto timestamp then remove them from _simulatedUserEvents
  if (index != -1) {
    _simulatedUserEvents.erase(_simulatedUserEvents.begin(),
                               _simulatedUserEvents.begin() + index + 1);
  }

  return true;
}

bool CoreSDK::validateUserEvent(nlohmann::json& userEventJson) {
  // If isTimeSimulated is false, throw an error if TIMESTAMP is present, else set the timestamp
  if (!_config->isTimeSimulated && userEventJson.contains("TIMESTAMP")) {
    throw std::runtime_error(
        "Timestamp should not be present in user event if isTimeSimulated flag is "
        "false in simulation mode.");
  } else if (!_config->isTimeSimulated) {
    userEventJson["TIMESTAMP"] = time(NULL);
    return true;
  }
  // If isTimeSimulated flag is true TIMESTAMP should be present in user events
  if (_config->isTimeSimulated && !userEventJson.contains("TIMESTAMP")) {
    throw std::runtime_error(
        "Timestamp should be present in a user event if isTimeSimulated flag is "
        "true in simulation mode.");
  }
  return true;
}

#endif  // SIMULATION_MODE
