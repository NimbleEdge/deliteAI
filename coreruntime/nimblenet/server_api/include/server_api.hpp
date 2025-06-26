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

/**
 * @brief Handles all server API interactions, including registration, cloud config and asset retrieval.
 *
 * This class manages communication with the backend server, including device registration, asset downloads,
 * cloud configuration retrieval, and event registration. It maintains state such as headers, hosts,
 * and retry counters, and logs network metrics.
 */
class ServerAPI {
  MetricsAgent* _metricsAgent; /**< Pointer to the metrics agent for logging network metrics. */
  nlohmann::json HEADERS = nlohmann::json::array(); /**< HTTP headers for requests. */
  std::string QUERY = ""; /**< Query string for requests. */
  std::string HOST = ""; /**< Main server host. */
  std::string CDN_HOST = ""; /**< CDN host for asset downloads. */
  std::string ADS_HOST = ""; /**< ADS host for private asset downloads. */
  std::shared_ptr<const Config> _config; /**< Configuration object. */
  std::map<std::string, std::string> requestToHostMap; /**< Maps request types to hosts. */
  std::atomic<int> registerRetries = serverconstants::MaxRegisterRetries; /**< Registration retry counter. */
  std::map<std::string, FileDownloadStatus> _currentStatusMap; /**< Tracks current download status by URL. */
  std::atomic<bool> registerDone = false; /**< Indicates if registration is complete. */

  /**
   * @brief Generates a unique request ID for tracking server API calls.
   *
   * The request ID is constructed by concatenating the deviceId and the current timestamp in milliseconds,
   * ensuring uniqueness for each request.
   *
   * @return Unique request ID string containing deviceId and timestamp.
   */
  std::string get_requestId() const;

  /**
   * @brief Gets the appropriate host for a given request type.
   *
   * @param reqType The type of request (e.g., asset, register_event).
   * @param defaultHost The default host to use if not mapped.
   * @return The resolved host string.
   */
  std::string get_host(const std::string& reqType, const std::string& defaultHost);

  /**
   * @brief Constructs the asset URL for a given asset.
   *
   * @param asset Shared pointer to the asset.
   * @param defaultHost The default host to use.
   * @return The asset URL string.
   */
  std::string get_asset_url(std::shared_ptr<Asset> asset, const std::string& defaultHost);

  /**
   * @brief Sends a network request to the server.
   *
   * @param body Request body as a string.
   * @param headers HTTP headers as JSON.
   * @param url Target URL.
   * @param method HTTP method (e.g., "GET", "POST").
   * @param length Optional content length (default -1).
   * @return Shared pointer to the network response.
   */
  const std::shared_ptr<NetworkResponse> send_request(const std::string& body,
                                                      nlohmann::json headers,
                                                      const std::string& url,
                                                      const std::string& method, int length = -1);

  /**
   * @brief Initiates an asynchronous file download.
   *
   * @param url The URL to download from.
   * @param fileName The local file name to save to.
   * @return The download status.
   */
  FileDownloadStatus download_file_async(const std::string& url, const std::string& fileName);

  /**
   * @brief Constructs the cloud config URL from a config object.
   *
   * @param config Shared pointer to the config object.
   * @return The cloud config URL string.
   */
  std::string get_cloudconfig_url(const std::shared_ptr<const Config> config);

 public:
  /**
   * @brief Constructs a ServerAPI instance.
   *
   * @param metricsAgent Pointer to the metrics agent.
   * @param config Shared pointer to the configuration object.
   */
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

  /**
   * @brief Destructor for ServerAPI.
   */
  ~ServerAPI() {}

  /**
   * @brief Checks if the server API is initialized (registration complete).
   *
   * @return True if initialized, false otherwise.
   */
  bool is_init() const { return registerDone.load(); }

  /**
   * @brief Retrieves an asset from the server synchronously.
   *
   * @param asset Shared pointer to the asset.
   * @return Optional string containing the asset data if successful.
   */
  std::optional<std::string> get_asset(std::shared_ptr<Asset> asset);

  /**
   * @brief Initiates an asynchronous asset download.
   *
   * @param asset Shared pointer to the asset.
   * @return The download status.
   */
  FileDownloadStatus get_asset_async(std::shared_ptr<Asset> asset);

#ifdef GENAI
  /**
   * @brief Downloads and prepares an LLM asset (GENAI only).
   *
   * @param asset Shared pointer to the asset.
   * @return The download status.
   */
  FileDownloadStatus get_llm(std::shared_ptr<Asset> asset);
#endif

  /**
   * @brief Registers the device with the server.
   *
   * @return True if registration succeeded, false otherwise.
   */
  bool device_register();

  /**
   * @brief Uploads logs to the server.
   *
   * @param logrequest Log request body.
   * @return True if upload succeeded, false otherwise.
   */
  bool upload_logs(const LogRequestBody& logrequest);

  /**
   * @brief Initializes the server API (performs registration if needed).
   *
   * @return True if initialization succeeded, false otherwise.
   */
  bool init();

  /**
   * @brief Retrieves the cloud configuration from the server.
   *
   * @param eTag ETag for cache validation.
   * @param retries Number of retries on authentication error (default: MaxAuthErrorRetries).
   * @return Pair of CloudConfigResponse and Deployment.
   */
  std::pair<CloudConfigResponse, Deployment> get_cloud_config(
      std::string eTag, int retries = serverconstants::MaxAuthErrorRetries);

  /**
   * @brief Constructs the cloud config URL from a config JSON string.
   *
   * @param configJson JSON string representing the config.
   * @return The cloud config URL string.
   */
  std::string get_cloudconfig_url(const std::string& configJson);

  /**
   * @brief Resets the registration retry counter to the maximum value.
   */
  void reset_register_retries();

  /**
   * @brief Updates the request-to-host mapping.
   *
   * @param reqMap Map from request type to host identifier.
   */
  void update_request_to_host_map(const std::map<std::string, std::string>& reqMap);

  /**
   * @brief Updates the ADS host for private asset downloads.
   *
   * @param adsHost The new ADS host string.
   */
  void update_ads_host(const std::string& adsHost);

  /**
   * @brief Registers a new event with the server.
   *
   * @param eventName Name of the event to register.
   */
  void register_new_event(const std::string& eventName);
};

/**
 * @brief Job for registering a new event with the server asynchronously.
 *
 * This job attempts to register a new event with the server, retrying if the ServerAPI is not initialized.
 */
struct RegisterNewEventJob : public Job<void> {
  std::string eventName; /**< Name of the event to register. */
  std::shared_ptr<ServerAPI> serverAPI; /**< ServerAPI instance to use. */
  std::shared_ptr<JobScheduler> jobScheduler; /**< JobScheduler instance. */

  /**
   * @brief Constructs a RegisterNewEventJob.
   *
   * @param newEventName Name of the event to register.
   * @param serverAPI_ Shared pointer to the ServerAPI instance.
   * @param jobScheduler_ Shared pointer to the JobScheduler instance.
   */
  RegisterNewEventJob(const std::string& newEventName, std::shared_ptr<ServerAPI> serverAPI_,
                      std::shared_ptr<JobScheduler> jobScheduler_)
      : Job("RegisterNewEventJob") {
    eventName = newEventName;
    serverAPI = serverAPI_;
    jobScheduler = jobScheduler_;
  }

  /**
   * @brief Destructor for RegisterNewEventJob.
   */
  ~RegisterNewEventJob() override = default;

  /**
   * @brief Processes the job: attempts to register the event, retrying if ServerAPI is not initialized.
   *
   * @return Status::RETRY if not initialized, Status::COMPLETE otherwise.
   */
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
