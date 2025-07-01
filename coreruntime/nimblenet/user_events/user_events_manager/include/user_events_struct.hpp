/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "util.hpp"
#include "data_variable.hpp"

/**
 * @brief Represents model input data with automatic memory management.
 *
 * This struct encapsulates raw data that will be passed to machine learning models,
 * providing automatic cleanup of allocated memory through a custom deleter.
 */
struct ModelInput {
  int length = 0; /**< Length of the data buffer in elements. */
  std::shared_ptr<void> data = nullptr; /**< Shared pointer to the raw data buffer with automatic cleanup. */

  /**
   * @brief Constructor for ModelInput.
   *
   * @param d Pointer to the data buffer.
   * @param l Length of the data buffer in elements.
   */
  ModelInput(void* d, int l) : data(d, [](void* p) { operator delete[](p); }) { length = l; }
};

/**
 * @brief Configuration information for preprocessing operations.
 *
 * This struct contains all the configuration needed to set up a preprocessor,
 * including rolling window parameters, aggregation operations, grouping columns,
 * and default values for missing data.
 */
struct PreProcessorInfo {
  std::vector<float> rollingWindowsInSecs; /**< Time windows in seconds for rolling aggregations. */
  std::vector<std::string> columnsToAggregate; /**< Column names to perform aggregation operations on. */
  std::vector<std::string> aggregateOperators; /**< Aggregation operators (e.g., "Sum", "Count", "Avg"). */
  std::vector<std::string> groupColumns; /**< Column names used for grouping events. */
  std::vector<double> defaultVector; /**< Default values for each aggregated column when no data is available. */
  std::string tableName; /**< Name of the table this preprocessor operates on. */
  DATATYPE dataType; /**< Data type for the preprocessor output. */
  bool valid = false; /**< Flag indicating whether the configuration is valid. */
};

/**
 * @brief Deserializes JSON configuration into PreProcessorInfo structure.
 *
 * @param j JSON object containing preprocessor configuration.
 * @param preProcessorInfo PreProcessorInfo structure to populate.
 */
void from_json(const json& j, PreProcessorInfo& preProcessorInfo);

/**
 * @brief Configuration information for database tables.
 *
 * This struct defines the schema and metadata for tables that store user events,
 * including column types, expiry settings, and validation status.
 */
struct TableInfo {
  bool valid = false; /**< Flag indicating whether the table configuration is valid. */
  std::string name; /**< Name of the table. */
  std::map<std::string, int> schema; /**< Column name to data type mapping. */
  int64_t expiryTimeInMins; /**< Time in minutes after which table rows expire. */
};

/**
 * @brief Represents a single event stored in a table.
 *
 * This struct contains the data for one event, including its timestamp,
 * grouping information, and the actual event data as a vector of data variables.
 */
struct TableEvent {
  std::vector<std::string> groups; /**< Group identifiers for this event from different preprocessors. */
  int64_t timestamp; /**< Timestamp when the event occurred. */
  std::vector<OpReturnType> row; /**< Event data as a vector of data variables. */
};

/**
 * @brief Represents a table row with key-value pairs.
 *
 * This struct stores event data as a map of column names to data variables,
 * along with a timestamp.
 */
struct TableRow {
  int64_t timestamp; /**< Timestamp when the row was created. */
  std::map<std::string, OpReturnType> row; /**< Column name to data variable mapping. */
};

/**
 * @brief Complete table data structure containing all events and metadata.
 *
 * This struct holds all the data for a table, including the events themselves,
 * column metadata, schema information, and mappings for efficient access.
 */
struct TableData {
  std::vector<TableEvent> allEvents; /**< All events stored in the table. */
  std::map<std::string, int> columnToIdMap; /**< Mapping from column names to their indices. */
  std::vector<std::string> columns; /**< Ordered list of column names. */
  std::map<std::string, int> schema; /**< Column name to data type mapping. */
};
