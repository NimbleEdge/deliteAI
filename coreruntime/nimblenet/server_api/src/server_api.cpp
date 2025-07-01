/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "server_api.hpp"

#include <optional>
#include <string>

#include "asset_manager.hpp"
#include "client.h"
#include "config_manager.hpp"
#include "core_utils/shard.hpp"
#include "json.hpp"
#include "logger.hpp"
#include "nimble_net_util.hpp"
#include "resource_manager_structs.hpp"
#include "time_manager.hpp"
#include "util.hpp"

using json = nlohmann::json;

#include "native_interface.hpp"

using namespace std;

#define GETPLANMETRIC "getplan"
#define REGISTERMETRIC "register"
#define GETCLOUDCONFIGMETRIC "getCloudConfig"
#define GETMODELVERSIONMETRIC "getModelVersion"
#define GETTASKMETRIC "getTask"
#define LOGMETRIC "logMetric"
static inline const std::string NETWORK = "network";
static inline const std::string ASYNCDOWNLOAD = "asyncdownload";

struct NetworkMetric {
  std::string requestId_ = "";
  std::string url_ = "";
  int statusCode_ = -1;
  long long int timeTakenInMicros_ = -1;

  NetworkMetric(const std::string& requestId, const std::string& url, int statusCode,
                long long int time) {
    requestId_ = requestId;
    url_ = url;
    statusCode_ = statusCode;
    timeTakenInMicros_ = time;
  }
};

inline const void to_json(nlohmann::json& j, const NetworkMetric& metric) {
  j["requestId"] = metric.requestId_;

  auto position = metric.url_.find('?');
  if (position != std::string::npos) {
    j["url"] = metric.url_.substr(0, position);
  } else {
    j["url"] = metric.url_;
  }

  j["statusCode"] = metric.statusCode_;
  j["timeUsecs"] = metric.timeTakenInMicros_;
}

inline const void to_json(nlohmann::json& j, const FileDownloadInfo& m) {
  j["requestId"] = m.requestId;
  j["prevStatusCode"] = m.prevStatus;
  j["currentStatusCode"] = m.currentStatus;
  j["reasonCode"] = m.currentStatusReasonCode;
  j["timeElapsedUSecs"] = m.timeElapsedInMicro;
}

nlohmann::json convertHeadersToLowercase(const nlohmann::json& headersJson) {
  nlohmann::json lowerHeadersJson;
  for (const auto& [key, value] : headersJson.items()) {
    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    lowerHeadersJson[lowerKey] = value;
  }
  return lowerHeadersJson;
}

bool is_success(const shared_ptr<NetworkResponse> response) {
  if (response->r.statusCode >= 200 && response->r.statusCode < 300) return true;
  return false;
}

bool is_failure(const shared_ptr<NetworkResponse> response) {
  if ((response->r.statusCode >= 400 && response->r.statusCode < 600) ||
      response->r.statusCode == EMPTY_ERROR_CODE)
    return true;
  return false;
}

// Generates unique Request Ids to be sent with Server API Calls
std::string ServerAPI::get_requestId() const {
  return _config->deviceId + "-" +
         to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now().time_since_epoch())
                       .count());
}

const std::shared_ptr<NetworkResponse> ServerAPI::send_request(const std::string& body,
                                                               nlohmann::json headers,
                                                               const std::string& url,
                                                               const std::string& method,
                                                               int length) {
  auto requestId = get_requestId();
  headers.push_back(nlohmann::json::object({{"Request-Id", requestId}}));
  auto start = Time::get_high_resolution_clock_time();
  auto response = nativeinterface::send_request(body, headers.dump(), url, method, length);
  auto timeTaken = Time::get_elapsed_time_in_micro(start);
  NetworkMetric m(requestId, url, response->r.statusCode, timeTaken);
  _metricsAgent->log_metrics(NETWORK.c_str(), nlohmann::json(m));
  return response;
}

std::string ServerAPI::get_host(const std::string& reqType, const std::string& defaultHost) {
#ifdef TESTING
  return HOST;
#endif
  if (requestToHostMap.find(reqType) != requestToHostMap.end()) {
    if (requestToHostMap[reqType] == serverconstants::CDNHostIdentifier)
      return CDN_HOST;
    else if (requestToHostMap[reqType] == serverconstants::ServiceHostIdentifier)
      return HOST;
  }
  return defaultHost;
}

std::string ServerAPI::get_asset_url(std::shared_ptr<Asset> asset, const std::string& defaultHost) {
  if (asset->location.isPrivate) {
    return ADS_HOST + asset->location.path;
  }
  auto reqType = assetmanager::get_string_from_asset_type(asset->type);
  std::string host = get_host(reqType, defaultHost);
  return host + serverconstants::ModelService + serverconstants::ApiVersionV4 +
         asset->location.path;
}

bool ServerAPI::init() {
  if (registerDone) return true;
  if (registerRetries == 0) return false;
  registerRetries--;
  string authInfoString;
  if (nativeinterface::get_file_from_device_common(serverconstants::AuthInfoFile, authInfoString)) {
    AuthenticationInfo info = jsonparser::get<AuthenticationInfo>(authInfoString);
    if (info.valid) {
      try {
        HEADERS = nlohmann::json::parse(info.apiHeaders);
        QUERY = info.apiQuery;
        return registerDone = true;
      } catch (...) {
        LOG_TO_ERROR("%s", "saved headers not parsed");
        // pass
      }
    }
  }
  if (device_register()) {
    return registerDone = true;
  }
  LOG_TO_ERROR("%s", "Registeration failed");
  return false;
}

void ServerAPI::update_request_to_host_map(const std::map<std::string, std::string>& reqMap) {
  requestToHostMap = reqMap;
}

void ServerAPI::update_ads_host(const std::string& adsHost) { ADS_HOST = adsHost; }

void ServerAPI::reset_register_retries() { registerRetries = serverconstants::MaxRegisterRetries; }

FileDownloadStatus ServerAPI::download_file_async(const std::string& url,
                                                  const std::string& fileName) {
  // NOTE : Request Id is now added at the outer layer for these requests.
  auto headers = HEADERS;
  const auto currentTime = Time::get_high_resolution_clock_time();
  const auto fileDownloadInfo =
      nativeinterface::download_to_file_async(url, headers.dump(), fileName);
  LOG_VERBOSE("Downloading URL %s into file %s, prev status %d, current status %d", url.c_str(),
              fileName.c_str(), fileDownloadInfo.prevStatus, fileDownloadInfo.currentStatus);

  auto it = _currentStatusMap.find(url);
  if (it != _currentStatusMap.end()) {
    if (fileDownloadInfo.currentStatus != it->second) {
      auto metricJson = nlohmann::json(fileDownloadInfo);
      metricJson["url"] = url;
      _metricsAgent->log_metrics(ASYNCDOWNLOAD.c_str(), metricJson);
    }
  }
  _currentStatusMap[url] = fileDownloadInfo.currentStatus;

  return fileDownloadInfo.currentStatus;
}

bool ServerAPI::device_register() {
  json body;
  body["deviceId"] = _config->deviceId;
  json registerHeaders = json::array();
  auto requestId = get_requestId();
  registerHeaders.push_back(
      json::object({{"ClientSecret", _config->clientSecret}, {"Request-Id", requestId}}));

  string URL = HOST + serverconstants::ModelService + serverconstants::ApiVersionV4 + "/clients/" +
               _config->clientId + "/register";

  const auto netResponse = ServerAPI::send_request(body.dump(), registerHeaders, URL, "POST");

  if (is_failure(netResponse)) {
    LOG_TO_ERROR("Device Registration Failed with status_code=%d .", netResponse->r.statusCode);
    return false;
  }
  string responseString(netResponse->r.body, netResponse->r.bodyLength);
  RegisterResponse response = jsonparser::get<RegisterResponse>(responseString);
  json headers = response.headers;
  HEADERS = headers;
  QUERY = "";
  if (response.queryParams.length() != 0) {
    QUERY = "?" + response.queryParams;
  }
  AuthenticationInfo info;
  info.apiHeaders = headers.dump();
  info.apiQuery = QUERY;
  nativeinterface::save_file_on_device_common(nlohmann::json(info).dump(),
                                              serverconstants::AuthInfoFile);
  LOG_TO_INFO("%s", "Device Registration Successful");

  return true;
}

bool ServerAPI::upload_logs(const LogRequestBody& logrequest) {
  auto requestId = get_requestId();
  nlohmann::json logMetric;
  string host = logrequest.host;
  json headers = logrequest.headers;
  const auto response = ServerAPI::send_request(logrequest.body.c_str(), headers, host, "POST");
  string responseString(response->r.body, response->r.bodyLength);
  if (is_success(response)) {
    return true;
  }
  return false;
}

std::pair<CloudConfigResponse, Deployment> ServerAPI::get_cloud_config(std::string eTag,
                                                                       int retries) {
  std::string URL = get_cloudconfig_url(_config);
  auto startTime = DeviceTime::current_time();

  auto headers = HEADERS;
  if (!eTag.empty()) {
    headers.push_back(nlohmann::json::object({{"If-None-Match", eTag}}));
  }

  const auto response = ServerAPI::send_request("", headers, URL, "GET");
  if (is_failure(response)) {
    LOG_TO_ERROR("Error in cloud config with status code %d", response->r.statusCode);
    if (retries > 0) {
      if (response->r.statusCode == AUTH_ERR) {
        if (device_register()) return get_cloud_config(eTag, retries - 1);
      }
    }

    return std::make_pair(CloudConfigResponse(), Deployment());
  }

  if (response->r.statusCode == UNMODIFIED) {
    LOG_TO_INFO("%s", "Cloud config is unmodified");
    CloudConfigResponse configResponse = CloudConfigResponse();
    configResponse.state = CloudConfigState::Unmodified;
    return std::make_pair(configResponse, Deployment());
  }

  string responseString(response->r.body, response->r.bodyLength);
  auto [configResponse, deployment] =
      get_config_and_deployment_from_json(nlohmann::json::parse(responseString));

  try {
    nlohmann::json headersJson =
        convertHeadersToLowercase(nlohmann::json::parse(response->r.headers));

    if (const auto etagIt = headersJson.find("etag"); etagIt != headersJson.end()) {
      deployment.eTag = etagIt.value();
    }

    const std::string neDate = headersJson.at("ne-date");
    auto serverTime = EpochTime::from_seconds(std::stoll(neDate));

    if (const auto ageIt = headersJson.find("age"); ageIt != headersJson.end()) {
      serverTime = serverTime + Duration::from_seconds(std::stoll(std::string{ageIt.value()}));
    }

    configResponse.peggedDeviceTime = PeggedDeviceTime{startTime, serverTime};
  } catch (const std::exception& e) {
    LOG_TO_ERROR("Unable to parse cloud config response headers as json: %s. Headers: %s", e.what(),
                 response->r.headers);
  }

  LOG_TO_DEBUG("%s", "Found Cloud Config");
  return std::make_pair(configResponse, deployment);
}

std::optional<std::string> ServerAPI::get_asset(std::shared_ptr<Asset> asset) {
  string URL = get_asset_url(asset, CDN_HOST);
  const auto response = ServerAPI::send_request("", HEADERS, URL, "GET");
  if (is_failure(response)) {
    LOG_TO_ERROR("Error in get_asset of type=%s with status code %d",
                 assetmanager::get_string_from_asset_type(asset->type).c_str(),
                 response->r.statusCode);
    if (response->r.statusCode == AUTH_ERR) {
      device_register();
    }
    return std::nullopt;
  }

  string responseString(response->r.body, response->r.bodyLength);
  return std::optional<std::string>(responseString);
}

FileDownloadStatus ServerAPI::get_asset_async(std::shared_ptr<Asset> asset) {
  string URL = get_asset_url(asset, CDN_HOST);
  auto fileName = asset->get_file_name_on_device();
  return ServerAPI::download_file_async(URL, fileName);
}

#ifdef GENAI
FileDownloadStatus ServerAPI::get_llm(std::shared_ptr<Asset> asset) {
  string URL = get_asset_url(asset, CDN_HOST);
  auto gzFileName = asset->get_file_name_on_device() + ".zip.gz";
  auto zipFileName = asset->get_file_name_on_device() + ".zip";

  // Unzipping the archive takes significant time in case of large models. In a scenario where  user
  // closes the app in the middle of unzip we want to be able to unzip from the archive present on
  // device instead of downloading again.
  if (nativeinterface::file_exists_common(zipFileName)) {
    util::delete_folder_recursively(asset->get_file_name_on_device());
    if (nativeinterface::unzip_archive(zipFileName, asset->get_file_name_on_device())) {
      if (nativeinterface::delete_file(zipFileName)) {
        return FileDownloadStatus::DOWNLOAD_SUCCESS;
      }
    }
  }

  auto fileDownloadStatus = ServerAPI::download_file_async(URL, gzFileName);
  if (fileDownloadStatus == FileDownloadStatus::DOWNLOAD_SUCCESS) {
    // Decompress .zip.gz to .zip
    if (!nativeinterface::decompress_file(gzFileName, zipFileName)) {
      LOG_TO_CLIENT_ERROR("Could not decompress file: %s", gzFileName.c_str());
      return FileDownloadStatus::DOWNLOAD_FAILURE;
    }
    // Delete .gz.zip
    if (!nativeinterface::delete_file(gzFileName, false)) {
      LOG_TO_CLIENT_ERROR("Could not delete file: %s", gzFileName.c_str());
      return FileDownloadStatus::DOWNLOAD_FAILURE;
    }
    // Unarchive .zip to folder
    if (!nativeinterface::unzip_archive(zipFileName, asset->get_file_name_on_device())) {
      LOG_TO_CLIENT_ERROR("Could not unqip archive: %s", zipFileName.c_str());
      return FileDownloadStatus::DOWNLOAD_FAILURE;
    }
    // Delete .zip
    if (!nativeinterface::delete_file(zipFileName)) {
      LOG_TO_ERROR("Could not delete file: %s", zipFileName.c_str());
      return FileDownloadStatus::DOWNLOAD_SUCCESS;
    }
  }
  return fileDownloadStatus;
}
#endif  // GENAI

void ServerAPI::register_new_event(const std::string& eventName) {
  string host = get_host("register_event", HOST);
  string URL = host + serverconstants::ModelService + serverconstants::ApiVersionV4 + "/clients/" +
               _config->clientId + "/events/" + eventName + "/register" + QUERY;
  const auto response = ServerAPI::send_request("", HEADERS, URL, "GET");
  if (is_failure(response)) {
    LOG_TO_ERROR("Register Event failed for %s", eventName.c_str());
  }
}

std::string ServerAPI::get_cloudconfig_url(const std::shared_ptr<const Config> config) {
  string host = get_host("cloudConfig", HOST);

  std::string queryParams = QUERY;
  std::string shardNumberQuery =
      "shardNumber=" + std::to_string(util::calculate_shard_number(config->deviceId));
  if (queryParams == "") {
    queryParams = "?" + shardNumberQuery;
  } else {
    queryParams += "&" + shardNumberQuery;
  }

  std::string cohortIdQuery = "cohortIds=" + config->cohortIds.dump();
  queryParams += "&" + cohortIdQuery;

  std::string deviceIdQuery = "deviceId=" + config->deviceId;

  queryParams += "&" + deviceIdQuery;

  string URL = host + serverconstants::ModelService + serverconstants::ApiVersionV4 + "/clients/" +
               config->clientId + "/deployments/" + config->compatibilityTag + "/config" +
               queryParams;
  return URL;
}

std::string ServerAPI::get_cloudconfig_url(const std::string& configJson) {
  auto config = nlohmann::json::parse(configJson);
  return get_cloudconfig_url(std::make_shared<Config>(config));
}
