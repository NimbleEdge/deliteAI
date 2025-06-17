
/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

struct WebLocation {
  bool isPrivate = false;
  std::string path;
  bool valid = false;
};

struct Location {
  std::string path;

  Location(const std::string& filePath) { path = filePath; };

  Location(){};
};

enum class AssetType {
  MODEL,
  SCRIPT,
#ifdef GENAI
  RETRIEVER,
  DOCUMENT,
  LLM,
#endif  // GENAI
};

struct AssetId {
  std::string name;
  std::string version;
  AssetType type;

  bool operator<(const AssetId& other) const;
};

struct Asset {
  AssetType type;
  std::string name;
  std::string version;
  std::vector<std::shared_ptr<Asset>> arguments;
  nlohmann::json metadata;
  // No need to write it to disk, as it is fetched from script being loaded in every session
  nlohmann::json metadataFromScript;
  WebLocation location;
  Location locationOnDisk;
  bool valid = false;
  bool osProvided = false;

  AssetId get_Id();
  std::string get_file_name_on_device() const;
};

void from_json(const nlohmann::json& j, WebLocation& location);
void to_json(nlohmann::json& j, const WebLocation& location);
void to_json(nlohmann::json& j, const Location& location);
void from_json(const nlohmann::json& j, Location& location);

namespace assetmanager {

AssetType get_asset_type_from_string(const std::string& assetTypeString);
std::string get_string_from_asset_type(const AssetType& assetType);
std::shared_ptr<Asset> parse_module_info(const nlohmann::json& moduleInfo);
nlohmann::json module_to_json(std::shared_ptr<Asset> modules);

}  // namespace assetmanager
