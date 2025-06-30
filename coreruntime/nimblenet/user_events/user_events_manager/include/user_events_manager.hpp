/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "config_manager.hpp"
#include "database.hpp"
#include "raw_store.hpp"
#include "variable_scope.hpp"

/**
 * @brief Deserializes JSON configuration into TableInfo structure.
 *
 * @param j JSON object containing table configuration.
 * @param tableInfo TableInfo structure to populate.
 */
void from_json(const json& j, TableInfo& tableInfo);

class CommandCenter;
class FunctionDef;

/**
 * @brief Manages user events parsing, storage, and preprocessing operations.
 *
 * This class provides functionality for adding events to the system, creating
 * preprocessors for event data transformation, and managing raw data stores
 * for different event types. It handles both JSON string and structured data
 * event inputs and integrates with the on-disk custom format database for
 * persistent storage.
 */
class UserEventsManager {
  std::shared_ptr<const Config> _config = nullptr; /**< Configuration settings for the events manager. */
  Database* _database = nullptr; /**< On-disk custom format database instance for persistent storage. */
  std::map<std::string, std::shared_ptr<RawStore>> _rawStoreMap; /**< Map of event types to their corresponding raw data stores. */
  std::map<std::string, std::string> _modelInputToTableNameMap; /**< Mapping from model input names to table names. */
  std::unordered_map<std::string, OpReturnType> _rawEventsTypesToPreProcess; /**< Preprocessing functions mapped by event type. */
  bool create_tables(const std::shared_ptr<const Config> config); /**< Creates database tables based on configuration. */
  bool _debugMode = false; /**< Flag indicating whether debug mode is enabled. */
  CommandCenter* _commandCenter; /**< Pointer to the command center for system coordination. */

 public:
  /**
   * @brief Constructor for UserEventsManager.
   *
   * @param commandCenter Pointer to the command center instance.
   * @param database Pointer to the on-disk custom format database instance for event storage.
   * @param config Shared pointer to configuration settings.
   */
  UserEventsManager(CommandCenter* commandCenter, Database* database,
                    std::shared_ptr<const Config> config);

  /**
   * @brief Creates a preprocessor from JSON configuration.
   *
   * @param preprocessorJson JSON configuration for the preprocessor.
   * @param dataType Data type for the preprocessor to handle.
   *
   * @return Pointer to the created BasePreProcessor, or nullptr if creation fails.
   */
  BasePreProcessor* create_preprocessor(const json& preprocessorJson, DATATYPE dataType);

  /**
   * @brief Creates a preprocessor from PreProcessorInfo structure.
   *
   * @param preProcessorInfo PreProcessorInfo structure containing configuration.
   *
   * @return Pointer to the created BasePreProcessor, or nullptr if creation fails.
   */
  BasePreProcessor* create_preprocessor(const PreProcessorInfo& preProcessorInfo);

  /**
   * @brief Adds an event using JSON string representation.
   *
   * @param eventMapJsonString JSON string containing the event data.
   * @param eventType Type/category of the event.
   *
   * @return UserEventsData containing the result of the operation.
   */
  UserEventsData add_event(const std::string& eventMapJsonString, const std::string& eventType);

  /**
   * @brief Adds an event using structured data representation.
   *
   * @param event Structured data variable containing the event information.
   * @param eventType Type/category of the event.
   *
   * @return UserEventsData containing the result of the operation.
   */
  UserEventsData add_event(const OpReturnType event, const std::string& eventType);

  /**
   * @brief Retrieves metrics information.
   *
   * @return JSON object containing metrics data.
   */
  nlohmann::json get_metrics();

#ifdef SCRIPTING
  /**
   * @brief Deletes old entries from the events type table.
   *
   * @return true if deletion was successful, false otherwise.
   */
  bool delete_old_entries_from_eventsType_Table();

  /**
   * @brief Triggered when a script is loaded, performs cleanup and validation.
   */
  void script_loaded_trigger();

  /**
   * @brief Creates a new raw store for the specified event type.
   *
   * @param eventType Type of events to be stored.
   * @param expiryType Type of expiry mechanism (e.g., "time", "count").
   * @param expiryValue Value for the expiry mechanism.
   *
   * @return Shared pointer to the created RawStore.
   */
  std::shared_ptr<RawStore> create_raw_store(const std::string& eventType,
                                             const std::string& expiryType, int expiryValue);

  /**
   * @brief Adds a preprocessing hook for specified event types.
   *
   * @param functionDataVariable Function to be executed as a preprocessing hook.
   * @param types Vector of event types that should trigger this preprocessing hook.
   */
  void add_pre_event_hook(OpReturnType functionDataVariable, std::vector<std::string>&& types);

#endif

#ifdef TESTING
  /**
   * @brief Adds an event type to the system for testing purposes.
   *
   * @param tableName Name of the table for the event type.
   * @param schema JSON schema defining the structure of the event data.
   *
   * @return true if the event type was added successfully, false otherwise.
   */
  bool add_eventType(const std::string& tableName, const nlohmann::json& schema);

  /**
   * @brief Gets the count of events from the events table for testing purposes.
   *
   * @param tableName Name of the table to query.
   *
   * @return Number of events in the specified table.
   */
  int get_count_from_eventsTable(const std::string& tableName);
#endif
};
