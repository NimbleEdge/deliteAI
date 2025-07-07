/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "nlohmann_json.hpp"

struct PlanData {
  bool valid = false;
  bool isTrainable = false;
  bool personalize = false;
  std::string version;
  std::string planFileName;
  std::string inferenceConfig;
  std::string personalizeConfig;
  int planLength;
  std::vector<nlohmann::json> executionProviderConfig;
  int epConfigVersion;
  PlanData(const PlanData& other) = delete;
  PlanData(PlanData&& other) = default;
  PlanData& operator=(PlanData&& other) = default;

  PlanData() {}
};

inline const void from_json(const nlohmann::json& j, PlanData& planData) {
  if (j.find("extras") != j.end())
    j.at("extras").get_to(planData.inferenceConfig);
  else
    j.at("inferenceConfig").get_to(planData.inferenceConfig);

  if (j.find("executionProviderConfig") != j.end()) {
    if (j.at("executionProviderConfig").type() == nlohmann::json::value_t::object)
      planData.executionProviderConfig.push_back(j.at("executionProviderConfig"));
    else
      j.at("executionProviderConfig").get_to(planData.executionProviderConfig);
  }
  if (j.find("version") != j.end()) {
    j.at("version").get_to(planData.version);
  }
  if (j.find("epConfigVersion") != j.end()) {
    j.at("epConfigVersion").get_to(planData.epConfigVersion);
  }
  planData.valid = true;
}

inline const void to_json(nlohmann::json& j, const PlanData& planData) {
  j = nlohmann::json{{"version", planData.version},
                     {"inferenceConfig", planData.inferenceConfig},
                     {"planLength", planData.planLength},
                     {"executionProviderConfig", planData.executionProviderConfig},
                     {"epConfigVersion", planData.epConfigVersion}};
}
