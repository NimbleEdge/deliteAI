/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "asset_manager.hpp"

#include "core_utils/fmt.hpp"
#include "resource_manager_constants.hpp"

bool AssetId::operator<(const AssetId& other) const {
  if (name != other.name) return name < other.name;
  if (version != other.version) return version < other.version;
  return type < other.type;
};

AssetId Asset::get_Id() { return AssetId{.name = name, .version = version, .type = type}; }

std::string Asset::get_file_name_on_device() const {
  switch (type) {
    case AssetType::MODEL:
      return name + version + rmconstants::InferenceFileName;
    case AssetType::SCRIPT:
      return name + version + rmconstants::TaskDataFileName;
#ifdef GENAI
    case AssetType::RETRIEVER:
      THROW("%s", "Cannot get file name for retriever, there is nothing to save");
    case AssetType::DOCUMENT:
      return name + version + rmconstants::DocumentDataFileName;
    case AssetType::LLM:
      return name + version + rmconstants::LLMFolderName;
#endif  // GENAI
  }
}

void from_json(const nlohmann::json& j, WebLocation& location) {
  location.valid = true;
  if (j.find("path") != j.end()) {
    j.at("path").get_to(location.path);
  } else {
    location.valid = false;
  }
  if (j.find("isPrivate") != j.end()) {
    j.at("isPrivate").get_to(location.isPrivate);
  } else {
    location.valid = false;
  }
  return;
};

void to_json(nlohmann::json& j, const WebLocation& location) {
  j = nlohmann::json{{"path", location.path}, {"isPrivate", location.isPrivate}};
};

void to_json(nlohmann::json& j, const Location& location) {
  j = nlohmann::json{{"path", location.path}};
};

void from_json(const nlohmann::json& j, Location& location) {
  if (j.find("path") != j.end()) {
    j.at("path").get_to(location.path);
  }
};

namespace assetmanager {

AssetType get_asset_type_from_string(const std::string& assetTypeString) {
  if (assetTypeString == "model")
    return AssetType::MODEL;
  else if (assetTypeString == "script")
    return AssetType::SCRIPT;
#ifdef GENAI
  else if (assetTypeString == "retriever")
    return AssetType::RETRIEVER;
  else if (assetTypeString == "document")
    return AssetType::DOCUMENT;
  else if (assetTypeString == "llm")
    return AssetType::LLM;
#endif  // GENAI
  else
    THROW("Unknown asset type %s", assetTypeString.c_str());
}

std::string get_string_from_asset_type(const AssetType& assetType) {
  switch (assetType) {
    case AssetType::MODEL:
      return "model";
    case AssetType::SCRIPT:
      return "script";
#ifdef GENAI
    case AssetType::RETRIEVER:
      return "retriever";
    case AssetType::DOCUMENT:
      return "document";
    case AssetType::LLM:
      return "llm";
#endif  // GENAI
  }
}

std::shared_ptr<Asset> parse_module_info(const nlohmann::json& moduleInfo) {
  auto asset = std::make_shared<Asset>();
  if (moduleInfo.find("type") != moduleInfo.end()) {
    auto moduleString = moduleInfo.at("type");
    if (moduleString.is_string()) {
      asset->type = get_asset_type_from_string(moduleString);
    } else {
      asset->type = moduleInfo.at("type").get<AssetType>();
    }
  }
  if (moduleInfo.find("name") != moduleInfo.end()) {
    asset->name = moduleInfo.at("name").get<std::string>();
  }
  if (moduleInfo.find("version") != moduleInfo.end()) {
    asset->version = moduleInfo.at("version").get<std::string>();
  }
  if (moduleInfo.find("location") != moduleInfo.end()) {
    moduleInfo.at("location").get_to(asset->location);
  }

  if (moduleInfo.find("metadata") != moduleInfo.end()) {
    asset->metadata = moduleInfo.at("metadata");
  }

  if (moduleInfo.find("locationOnDisk") != moduleInfo.end()) {
    moduleInfo.at("locationOnDisk").get_to(asset->locationOnDisk);
  }

  if (moduleInfo.find("arguments") != moduleInfo.end()) {
    auto argumentsArray = moduleInfo.at("arguments");
    if (!argumentsArray.is_array()) {
      THROW("arguments should be array for module %s", asset->name.c_str());
    }

    for (auto& argument : argumentsArray) {
      asset->arguments.emplace_back(parse_module_info(argument));
    }
  }

  if (moduleInfo.find("osProvided") != moduleInfo.end()) {
    moduleInfo.at("osProvided").get_to(asset->osProvided);
  }

  asset->valid = true;
  return asset;
}

nlohmann::json module_to_json(std::shared_ptr<Asset> modules) {
  nlohmann::json moduleJson;

  moduleJson["type"] = modules->type;
  moduleJson["name"] = modules->name;
  moduleJson["version"] = modules->version;
  moduleJson["location"] = nlohmann::json(modules->location);
  moduleJson["locationOnDisk"] = nlohmann::json(modules->locationOnDisk);
  moduleJson["arguments"] = nlohmann::json::array();
  moduleJson["metadata"] = modules->metadata;
  for (auto arg : modules->arguments) {
    moduleJson["arguments"].push_back(module_to_json(arg));
  }
  moduleJson["osProvided"] = modules->osProvided;
  return moduleJson;
}

}  // namespace assetmanager
