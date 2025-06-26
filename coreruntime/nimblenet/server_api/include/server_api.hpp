/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <optional>
#include <stdexcept>

#include "config_manager.hpp"
#include "core_sdk_structs.hpp"
#include "job_scheduler.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "native_interface_structs.hpp"
#include "nimble_net_util.hpp"
#include "server_api_constants.hpp"
#include "server_api_structs.hpp"

class ServerAPI {
  MetricsAgent* _metricsAgent;
  nlohmann::json HEADERS = nlohmann::json::array();
  std::string QUERY = "";
  std::string HOST = "";
  std::string CDN_HOST = "";
  std::string ADS_HOST = "";
  std::shared_ptr<const Config> _config;
  std::map<std::string, std::string> requestToHostMap;
  std::atomic<int> registerRetries = serverconstants::MaxRegisterRetries;
  std::map<std::string, FileDownloadStatus> _currentStatusMap;
  std::atomic<bool> registerDone = false;
  std::string get_requestId() const;
  std::string get_host(const std::string& reqType, const std::string& defaultHost);
  std::string get_asset_url(std::shared_ptr<Asset> asset, const std::string& defaultHost);
  const std::shared_ptr<NetworkResponse> send_request(const std::string& body,
                                                      nlohmann::json headers,
                                                      const std::string& url,
                                                      const std::string& method, int length = -1);
  FileDownloadStatus download_file_async(const std::string& url, const std::string& fileName);
  std::string get_cloudconfig_url(const std::shared_ptr<const Config> config);

 public:
  ServerAPI(MetricsAgent* metricsAgent, std::shared_ptr<const Config> config) {
    _metricsAgent = metricsAgent;
    HOST = config->host;
    CDN_HOST = config->host;
    auto pos = CDN_HOST.find("://");
    if (pos != std::string::npos) {
      CDN_HOST.insert(pos + 3, "cdn-");
    }
    _config = config;
  }

  ~ServerAPI() {}

  bool is_init() const { return registerDone.load(); }

  std::optional<std::string> get_asset(std::shared_ptr<Asset> asset);
  FileDownloadStatus get_asset_async(std::shared_ptr<Asset> asset);
#ifdef GENAI
  FileDownloadStatus get_llm(std::shared_ptr<Asset> asset);
#endif

  bool device_register();
  bool upload_logs(const LogRequestBody& logrequest);
  bool init();
  std::pair<CloudConfigResponse, Deployment> get_cloud_config(
      std::string eTag, int retries = serverconstants::MaxAuthErrorRetries);

  std::string get_cloudconfig_url(const std::string& configJson);

  void reset_register_retries();
  void update_request_to_host_map(const std::map<std::string, std::string>& reqMap);
  void update_ads_host(const std::string& adsHost);
  void register_new_event(const std::string& eventName);
};

struct RegisterNewEventJob : public Job<void> {
  std::string eventName;
  std::shared_ptr<ServerAPI> serverAPI;
  std::shared_ptr<JobScheduler> jobScheduler;

  RegisterNewEventJob(const std::string& newEventName, std::shared_ptr<ServerAPI> serverAPI_,
                      std::shared_ptr<JobScheduler> jobScheduler_)
      : Job("RegisterNewEventJob") {
    eventName = newEventName;
    serverAPI = serverAPI_;
    jobScheduler = jobScheduler_;
  }

  ~RegisterNewEventJob() override = default;

  Job::Status process() override {
    try {
      if (!serverAPI->is_init()) {
        return Status::RETRY;
      }
      serverAPI->register_new_event(eventName);
    } catch (const std::runtime_error& e) {
      LOG_TO_ERROR("Got error throw in RegisterNewEventJob that will be ignored: %s", e.what());
    } catch (...) {
      LOG_TO_ERROR("Got unknown error thrown in RegisterNewEventJob that will be ignored");
    }
    return Status::COMPLETE;
  }
};
