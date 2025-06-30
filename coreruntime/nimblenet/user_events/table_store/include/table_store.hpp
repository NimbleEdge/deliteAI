/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <vector>

#include "config_manager.hpp"
#include "executor_structs.h"
#include "pre_processor.hpp"
#include "user_events_struct.hpp"

#ifdef SCRIPTING
#include "variable_scope.hpp"
#endif  // SCRIPTING

class BasePreProcessor;
class FunctionDef;

/**
 * @brief Manages in-memory storage and processing of table events.
 *
 * This class provides functionality for storing events in a tabular format,
 * creating preprocessors for data transformation, and managing the lifecycle
 * of events and their associated aggregations. It serves as the central
 * data store for user events and coordinates with preprocessors to maintain
 * rolling window aggregations.
 */
class TableStore {
  std::shared_ptr<const Config> _config = nullptr; /**< Configuration settings for the table store. */
  std::string _tableName; /**< Name of the table being managed. */
  std::shared_ptr<TableData> _tableData = std::make_shared<TableData>(); /**< Shared pointer to the table's data structure containing all events and metadata. */
  std::vector<BasePreProcessor*> _preprocessors; /**< Vector of preprocessors associated with this table. */
  bool _isInvalid = false; /**< Flag indicating whether the table store is in an invalid state. */

  /**
   * @brief Updates column metadata when new columns are added.
   *
   * @param columnName Name of the column to add to the metadata.
   */
  void update_column_meta_data(const std::string& columnName);

  /**
   * @brief Extracts group information from JSON input for a specific preprocessor.
   *
   * @param preprocessorIndex Index of the preprocessor to use for group extraction.
   * @param preprocessorInput JSON input containing event data.
   *
   * @return Vector of group identifiers extracted from the input.
   */
  std::vector<std::string> get_groups_from_json(int preprocessorIndex,
                                                const nlohmann::json& preprocessorInput);

 public:
  /**
   * @brief Creates a new preprocessor for this table.
   *
   * @param preprocessorInfo Configuration information for the preprocessor.
   *
   * @return Pointer to the created BasePreProcessor, or nullptr if creation fails.
   */
  BasePreProcessor* create_preprocessor(const PreProcessorInfo& preprocessorInfo);

  /**
   * @brief Adds a new row to the table.
   *
   * This method converts a TableRow to a TableEvent and adds it to the table.
   * It also triggers all associated preprocessors to update their aggregations.
   *
   * @param r TableRow containing the event data to add.
   */
  void add_row(const TableRow& r);

  /**
   * @brief Verifies that a key-value pair matches the expected schema.
   *
   * @param key Column name to verify.
   * @param val Data variable containing the value to verify.
   *
   * @return true if the key-value pair is valid according to the schema, false otherwise.
   */
  bool verify_key(const std::string& key, OpReturnType val);

  /**
   * @brief Constructor for TableStore with schema.
   *
   * @param schema Map of column names to their data types.
   */
  TableStore(const std::map<std::string, int>& schema);

  /**
   * @brief Default constructor for TableStore.
   */
  TableStore() {};

  TableStore(const TableStore& other) = delete; /**< Deleted copy constructor. */
  TableStore(TableStore&& other) = default; /**< Default move constructor. */
  TableStore& operator=(const TableStore& other) = delete; /**< Deleted copy assignment operator. */
  TableStore& operator=(TableStore&& other) = default; /**< Default move assignment operator. */

  /**
   * @brief Gets the table data structure.
   *
   * @return Shared pointer to the TableData containing all events and metadata.
   */
  std::shared_ptr<TableData> get_data() const { return _tableData; }

  /**
   * @brief Destructor that cleans up preprocessors.
   */
  ~TableStore();

#ifdef TESTING
  /**
   * @brief Constructor for testing purposes with JSON schema.
   *
   * @param commandCenter Pointer to the command center instance.
   * @param tableName Name of the table.
   * @param schema JSON schema defining the table structure.
   */
  TableStore(CommandCenter* commandCenter, const std::string& tableName,
             const nlohmann::json& schema);
#endif
};
