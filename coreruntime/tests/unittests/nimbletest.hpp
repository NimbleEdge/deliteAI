/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <gtest/gtest.h>
#include <sys/stat.h>

#include <fstream>
#include <shared_mutex>
#include <thread>

#include "command_center.hpp"
#include "database.hpp"
#include "native_interface.hpp"
#include "nimbletest.hpp"
#include "resource_manager.hpp"
#include "server_api.hpp"
#include "time_manager.hpp"
#include "util.hpp"

using namespace std;

extern const std::string configJsonChar;
extern const std::string scriptConfigJsonChar;
extern const std::string validE2EConfigJson;
extern const std::string scriptDeploymentJson;

class ServerHelpers {
 public:
  [[nodiscard]] static bool get_file_from_assets(const std::string &fileName, std::string &result) {
    std::ifstream inFile("./assets/" + fileName, std::ios::binary);
    if (inFile.fail()) return false;
    result =
        std::string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    return true;
  }

  [[nodiscard]] static bool get_full_file_path_from_assets(const std::string &fileName,
                                                           std::string &fullFilePath) {
    std::string path = "./assets/" + fileName;
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
      return false;
    }
    fullFilePath = path;
    return true;
  }

  [[nodiscard]] static bool create_folder(const std::string &folderFullPath) {
    int dirCreated = mkdir(folderFullPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    if (dirCreated != 0 && dirCreated == EEXIST) {
      LOG_TO_ERROR("Could not create directory %s", folderFullPath.c_str());
      return false;
    }
    return true;
  }

  [[nodiscard]] static Deployment load_deployment_config_from_device(
      std::shared_ptr<Config> config) {
    string deploymentString;
    if (!nativeinterface::get_file_from_device_common(
            config->compatibilityTag + coresdkconstants::DeploymentFileName, deploymentString)) {
      return Deployment();
    }
    return jsonparser::get<Deployment>(deploymentString);
  }
};
