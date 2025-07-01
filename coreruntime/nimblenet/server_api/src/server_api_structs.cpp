/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "server_api_structs.hpp"

#include <algorithm>

#include "json.hpp"
#ifndef MINIMAL_BUILD
#include "thread_pool.hpp"
#endif  // MINIMAL_BUILD

void from_json(const json& j, RegisterResponse& registerResponse) {
  j.at("headers").get_to(registerResponse.headers);
  j.at("queryParams").get_to(registerResponse.queryParams);
}

void from_json(const json& j, Deployment& dep) {
  j.at("id").get_to(dep.Id);
  if (j.find("eTag") != j.end()) {
    j.at("eTag").get_to(dep.eTag);
  }

  if (j.find("forceUpdate") != j.end()) {
    j.at("forceUpdate").get_to(dep.forceUpdate);
  }
  dep.script = assetmanager::parse_module_info(j.at("script"));

  if (j.find("modules") != j.end() && j.at("modules").is_array()) {
    int size = j.at("modules").size();
    auto mod = j.at("modules");
    for (int i = 0; i < size; i++) {
      dep.modules.push_back(assetmanager::parse_module_info(mod[i]));
    }
  }
};

void to_json(json& j, const Deployment& dep) {
  j = json{{"id", dep.Id},
           {"forceUpdate", dep.forceUpdate},
           {"eTag", dep.eTag},
           {"script", assetmanager::module_to_json(dep.script)}};
  nlohmann::json moduleArray = json::array();
  for (auto module : dep.modules) {
    moduleArray.push_back(assetmanager::module_to_json(module));
  }
  j["modules"] = moduleArray;
};

void from_json(const json& j, CloudConfigResponse& cloudConfigResponse) {
  cloudConfigResponse.state = CloudConfigState::Invalid;

  if (j.find("inferMetricLogInterval") != j.end()) {
    j.at("inferMetricLogInterval").get_to(cloudConfigResponse.inferenceMetricLogInterval);
  }
  if (j.find("threadSleepTimeUSecs") != j.end()) {
    j.at("threadSleepTimeUSecs").get_to(cloudConfigResponse.threadSleepTimeUSecs);
    cloudConfigResponse.threadSleepTimeUSecs =
        std::max(cloudConfigResponse.threadSleepTimeUSecs,
                 (long long)coresdkconstants::LongRunningThreadSleepUTime);
  }
  if (j.find("requestToHostMap") != j.end()) {
    j.at("requestToHostMap").get_to(cloudConfigResponse.requestToHostMap);
  }
  if (j.find("fileDeleteTimeInDays") != j.end()) {
    j.at("fileDeleteTimeInDays").get_to(cloudConfigResponse.fileDeleteTimeInDays);
  }

  if (j.find("time") != j.end()) {
    j.at("time").get_to(cloudConfigResponse.serverTimeMicros);
  }

  if (j.find("nimbleLogger") != j.end()) {
    j.at("nimbleLogger").get_to(cloudConfigResponse.nimbleLoggerConfig);
  } else {
    LOG_TO_ERROR("%s", "nimbleLogger key not found in Cloud Config.");
  }
  // Necessary to assign default value for NimbleEdge LogSender URL so that in case of any issues,
  // logs keep on coming via default setup.
  if (cloudConfigResponse.nimbleLoggerConfig.senderConfig._host.empty()) {
    cloudConfigResponse.nimbleLoggerConfig.senderConfig._host =
        loggerconstants::DefaultLogUploadURL;
  }

  if (j.find("externalLogger") != j.end()) {
    j.at("externalLogger").get_to(cloudConfigResponse.externalLoggerConfig);
  }
  if (j.find("adsHost") != j.end()) {
    j.at("adsHost").get_to(cloudConfigResponse.adsHost);
  }

#ifdef GENAI
  if (auto it = j.find("LLMExecutor"); it != j.end()) {
    it.value().get_to(cloudConfigResponse.llmExecutorConfig);
  }
#endif  // GENAI

#ifndef MINIMAL_BUILD
  if (j.find("threadSpinTimeInMs") != j.end()) {
    int spinTime;
    j.at("threadSpinTimeInMs").get_to(spinTime);
    ThreadPool::spinTimeInMs = spinTime;
  }
#endif  // MINIMAL_BUILD
  cloudConfigResponse.state = CloudConfigState::Valid;
}

void from_json(const json j, LoggerConfig& loggerConfig) {
  j.at("sender").get_to(loggerConfig.senderConfig);
  j.at("writer").get_to(loggerConfig.writerConfig);
}

void to_json(json& j, const LoggerConfig& loggerConfig) {
  j = json{{"sender", nlohmann::json(loggerConfig.senderConfig)},
           {"writer", nlohmann::json(loggerConfig.writerConfig)}};
}

void to_json(json& j, const CloudConfigResponse& cloudConfig) {
  j = json{
      {"inferMetricLogInterval", cloudConfig.inferenceMetricLogInterval},
      {"threadSleepTimeUSecs", cloudConfig.threadSleepTimeUSecs},
      {"requestToHostMap", cloudConfig.requestToHostMap},
      {"fileDeleteTimeInDays", cloudConfig.fileDeleteTimeInDays},
      {"time", cloudConfig.serverTimeMicros},
      {"nimbleLogger", cloudConfig.nimbleLoggerConfig},
      {"externalLogger", cloudConfig.externalLoggerConfig},
      {"adsHost", cloudConfig.adsHost},
#ifndef MINIMAL_BUILD
      {"threadSpinTimeInMs", ThreadPool::spinTimeInMs.load()},
#endif
  };
}

// Auth Info
void from_json(const json& j, AuthenticationInfo& authInfo) {
  j.at("apiHeaders").get_to(authInfo.apiHeaders);
  j.at("apiQuery").get_to(authInfo.apiQuery);
  authInfo.valid = true;
}

void to_json(json& j, const AuthenticationInfo& authInfo) {
  j = json{{"apiHeaders", authInfo.apiHeaders}, {"apiQuery", authInfo.apiQuery}};
}

void from_json(const json& j, ModelMetadata& metadata) {
  j.at("version").get_to(metadata.version);
  j.at("epConfigVersion").get_to(metadata.epConfigVersion);
  metadata.valid = true;
}

void to_json(json& j, const ModelMetadata& metadata) {
  j = json{{"version", metadata.version}, {"epConfigVersion", metadata.epConfigVersion}};
}

void from_json(const json& j, TaskMetadata& metadata) {
  j.at("version").get_to(metadata.version);
  metadata.valid = true;
}

void to_json(json& j, const TaskMetadata& metadata) { j = json{{"version", metadata.version}}; }

std::pair<CloudConfigResponse, Deployment> get_config_and_deployment_from_json(
    const nlohmann::json& j) {
  CloudConfigResponse cloudConfigResponse;

  cloudConfigResponse = jsonparser::get_from_json<CloudConfigResponse>(j);

  Deployment deployment;
  deployment = jsonparser::get_from_json<Deployment>(j.at("deployment"));

  return std::make_pair(cloudConfigResponse, deployment);
}
