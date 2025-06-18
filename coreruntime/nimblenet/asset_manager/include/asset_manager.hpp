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

/**
 * @brief Represents a web-accessible location for an asset.
 */
struct WebLocation {
  bool isPrivate = false; /**< Indicates if the asset is privately hosted on client's cloud. */
  std::string path;       /**< The web path to the asset. */
  bool valid = false;     /**< Whether the location is valid. */
};

/**
 * @brief Represents a local file system location.
 */
struct Location {
  std::string path; /**< The file system path. */

  /**
   * @brief Constructs a Location with a given file path.
   *
   * @param filePath The path to the file.
   */
  Location(const std::string& filePath) { path = filePath; }

  /**
   * @brief Default constructor for Location.
   */
  Location() {}
};

/**
 * @brief Enum representing supported asset types.
 */
enum class AssetType {
  MODEL,  /**< Machine learning model asset. */
  SCRIPT, /**< Script asset. */
#ifdef GENAI
  RETRIEVER, /**< Retriever asset. */
  DOCUMENT,  /**< Document asset. */
  LLM,       /**< Large Language Model asset. */
#endif       // GENAI
};

/**
 * @brief Unique identifier for an asset.
 */
struct AssetId {
  std::string name;    /**< Asset name. */
  std::string version; /**< Asset version. */
  AssetType type;      /**< Type of the asset. */

  /**
   * @brief Compares this AssetId to another for ordering.
   *
   * @param other The other AssetId to compare with.
   * @return True if this is less than other.
   */
  bool operator<(const AssetId& other) const;
};

/**
 * @brief Represents an asset and its metadata.
 */
struct Asset {
  AssetType type;                                /**< Type of asset. */
  std::string name;                              /**< Asset name. */
  std::string version;                           /**< Asset version. */
  std::vector<std::shared_ptr<Asset>> arguments; /**< Dependent assets. */
  nlohmann::json metadata;                       /**< Metadata fetched from cloud. */
  nlohmann::json metadataFromScript;             /**< Metadata directly added in script. */
  WebLocation location;                          /**< Remote or web location. */
  Location locationOnDisk;                       /**< File system location. */
  bool valid = false;                            /**< Indicates if asset is valid. */
  bool osProvided = false;                       /**< Whether asset is provided by OS. */

  /**
   * @brief Gets the asset ID.
   *
   * @return An AssetId instance for this asset.
   */
  AssetId get_Id();

  /**
   * @brief Gets the filename on the device for this asset.
   *
   * @return A string representing the filename.
   */
  std::string get_file_name_on_device() const;
};

/**
 * @brief Deserializes a WebLocation from JSON.
 *
 * @param j The JSON object.
 * @param location The WebLocation to populate.
 */
void from_json(const nlohmann::json& j, WebLocation& location);

/**
 * @brief Serializes a WebLocation to JSON.
 *
 * @param j The JSON object to populate.
 * @param location The WebLocation to serialize.
 */
void to_json(nlohmann::json& j, const WebLocation& location);

/**
 * @brief Serializes a Location to JSON.
 *
 * @param j The JSON object to populate.
 * @param location The Location to serialize.
 */
void to_json(nlohmann::json& j, const Location& location);

/**
 * @brief Deserializes a Location from JSON.
 *
 * @param j The JSON object.
 * @param location The Location to populate.
 */
void from_json(const nlohmann::json& j, Location& location);

namespace assetmanager {

/**
 * @brief Converts a string to an AssetType enum.
 *
 * @param assetTypeString The string representing the asset type.
 * @return The corresponding AssetType.
 */
AssetType get_asset_type_from_string(const std::string& assetTypeString);

/**
 * @brief Converts an AssetType enum to a string.
 *
 * @param assetType The AssetType to convert.
 * @return The corresponding string representation.
 */
std::string get_string_from_asset_type(const AssetType& assetType);

/**
 * @brief Parses module info from JSON into an Asset.
 *
 * @param moduleInfo JSON object representing module info.
 * @return A shared pointer to an Asset.
 */
std::shared_ptr<Asset> parse_module_info(const nlohmann::json& moduleInfo);

/**
 * @brief Converts an Asset to a JSON representation.
 *
 * @param modules The Asset to serialize.
 * @return A JSON object representing the asset.
 */
nlohmann::json module_to_json(std::shared_ptr<Asset> modules);

}  // namespace assetmanager
