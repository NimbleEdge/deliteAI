/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "data_variable.hpp"
#include "util.hpp"
#include "rolling_window.hpp"
#include "user_events_struct.hpp"

/**
 * @brief Abstract base class for event preprocessing operations.
 *
 * Defines the interface for preprocessing user events, including group
 * extraction, model input generation, and event aggregation. Provides
 * virtual methods that must be implemented by derived classes.
 */
class BasePreProcessor {
 public:
  int _id; /**< Unique identifier for this preprocessor. */

  /**
   * @brief Adds a new event to the preprocessor.
   *
   * @param newEventIndex Index of the new event in the table.
   */
  virtual void add_event(int newEventIndex) = 0;

  /**
   * @brief Extracts group identifier from a table event.
   *
   * Creates a group identifier by concatenating values from group columns
   * in the TableEvent.
   *
   * @param e TableEvent to extract group from.
   * @return Group identifier as a string.
   */
  virtual std::string get_group_from_event(const TableEvent& e) = 0;

  /**
   * @brief Extracts group identifier from row data with validation.
   *
   * Creates a group identifier by concatenating values from group columns
   * in the row data. Validates that all required group columns have data
   * before creating the group. Returns false if any required column is missing.
   *
   * @param row Vector of string values representing a row.
   * @param columnsFilled Vector indicating which columns have valid data.
   * @param retGroup Output parameter for the extracted group.
   * @return true if group extraction was successful, false if required columns are missing.
   */
  virtual bool get_group_from_row(const std::vector<std::string>& row,
                                  const std::vector<bool>& columnsFilled,
                                  std::string& retGroup) = 0;

  /**
   * @brief Extracts group identifiers from JSON input.
   *
   * @param preprocessorInput JSON array containing event data.
   * @return Vector of group identifiers.
   */
  virtual std::vector<std::string> get_groups_from_json(
      const nlohmann::json& preprocessorInput) = 0;

  /**
   * @brief Generates model input from JSON data.
   *
   * @param preprocessorInput JSON array containing event data.
   * @return Shared pointer to ModelInput containing processed data.
   */
  virtual std::shared_ptr<ModelInput> get_model_input(const nlohmann::json& preprocessorInput) = 0;

  /**
   * @brief Generates model input data variable from group identifiers.
   *
   * @param groups Vector of group identifiers.
   * @return Data variable containing processed model input.
   */
  virtual OpReturnType get_model_input_data_variable(const std::vector<std::string>& groups) = 0;

  /**
   * @brief Generates model input data variable from JSON data.
   *
   * @param json JSON array containing event data.
   * @return Data variable containing processed model input.
   */
  virtual OpReturnType get_model_input_data_variable(const nlohmann::json& json) = 0;

  /**
   * @brief Generates model input data variable from nested group vectors.
   *
   * @param allGroups Vector of group vectors.
   * @return Data variable containing processed model input.
   */
  virtual OpReturnType get_model_input_data_variable(
      const std::vector<std::vector<std::string>>& allGroups) = 0;

  /**
   * @brief Gets the number of group-by columns.
   *
   * @return Number of columns used for grouping.
   */
  virtual int get_num_of_groupBys() = 0;

  /**
   * @brief Creates group identifier from group vector.
   *
   * @param groupVec Vector of group values.
   * @return Combined group identifier.
   */
  virtual std::string get_group_from_group_vector(const std::vector<std::string>& groupVec) = 0;

  /**
   * @brief Constructor for BasePreProcessor.
   *
   * @param id Unique identifier for this preprocessor.
   */
  BasePreProcessor(int id) { _id = id; }

  /**
   * @brief Virtual destructor.
   */
  virtual ~BasePreProcessor() = default;
};

/**
 * @brief Concrete implementation of event preprocessing with rolling window aggregations.
 *
 * Implements the BasePreProcessor interface with support for rolling window
 * aggregations, group-based feature extraction, and model input generation.
 * Manages multiple rolling windows and maintains feature maps per group.
 */
class PreProcessor : public BasePreProcessor {
  using T = double; /**< Type alias for aggregation data type. */

  std::vector<int> _groupIds; /**< Column indices used for grouping. */
  std::vector<int> _columnIds; /**< Column indices to aggregate. */
  bool _isUseless = false; /**< Flag indicating if preprocessor is in invalid state. */
  PreProcessorInfo _info; /**< Configuration information for this preprocessor. */
  std::vector<double> _defaultFeature; /**< Default feature values when no data is available. */
  std::vector<RollingWindow*> _rollingWindows; /**< Rolling windows for time-based aggregations. */
  std::map<std::string, std::vector<double>> _groupWiseFeatureMap; /**< Feature values per group. */
  std::shared_ptr<TableData> _tableData = nullptr; /**< Shared pointer to table data. */

 public:
  /**
   * @brief Gets the number of group-by columns.
   *
   * @return Number of columns used for grouping.
   */
  int get_num_of_groupBys() override { return _groupIds.size(); }

  /**
   * @brief Extracts group identifier from row data with validation.
   *
   * Creates a group identifier by concatenating values from group columns
   * in the row data. Validates that all required group columns have data
   * before creating the group. Returns false if any required column is missing.
   *
   * @param row Vector of string values representing a row.
   * @param columnsFilled Vector indicating which columns have valid data.
   * @param retGroup Output parameter for the extracted group.
   * @return true if group extraction was successful, false if required columns are missing.
   */
  bool get_group_from_row(const std::vector<std::string>& row,
                          const std::vector<bool>& columnsFilled, std::string& retGroup) override;

  /**
   * @brief Creates group identifier from group vector.
   *
   * @param groupVec Vector of group values.
   * @return Combined group identifier.
   */
  std::string get_group_from_group_vector(const std::vector<std::string>& groupVec) override;

  /**
   * @brief Extracts group identifier from a table event.
   *
   * Creates a group identifier by concatenating values from group columns
   * in the TableEvent.
   *
   * @param e TableEvent to extract group from.
   * @return Group identifier as a string (e.g., "user123+US+").
   */
  std::string get_group_from_event(const TableEvent& e) override;

  /**
   * @brief Extracts group identifiers from JSON input.
   *
   * @param preprocessorInput JSON array containing event data.
   * @return Vector of group identifiers.
   */
  std::vector<std::string> get_groups_from_json(const nlohmann::json& preprocessorInput) override;

  /**
   * @brief Generates model input from JSON data.
   *
   * @param preprocessorInput JSON array containing event data.
   * @return Shared pointer to ModelInput containing processed data.
   */
  std::shared_ptr<ModelInput> get_model_input(const nlohmann::json& preprocessorInput) override;

  /**
   * @brief Generates model input data variable from group identifiers.
   *
   * @param groups Vector of group identifiers.
   * @return Data variable containing processed model input.
   */
  OpReturnType get_model_input_data_variable(const std::vector<std::string>& groups) override;

  /**
   * @brief Generates model input data variable from JSON data.
   *
   * @param json JSON array containing event data.
   * @return Data variable containing processed model input.
   */
  OpReturnType get_model_input_data_variable(const nlohmann::json& json) override;

  /**
   * @brief Generates model input data variable from nested group vectors.
   *
   * @param allGroups Vector of group vectors.
   * @return Data variable containing processed model input.
   */
  OpReturnType get_model_input_data_variable(
      const std::vector<std::vector<std::string>>& allGroups) override;

  /**
   * @brief Constructor for PreProcessor.
   *
   * @param id Unique identifier for this preprocessor.
   * @param info Configuration information for the preprocessor.
   * @param groupIds Column indices used for grouping.
   * @param columnIds Column indices to aggregate.
   * @param tableData Shared pointer to table data.
   */
  PreProcessor(int id, const PreProcessorInfo& info, const std::vector<int>& groupIds,
               const std::vector<int>& columnIds, std::shared_ptr<TableData> tableData);

  /**
   * @brief Adds a new event to the preprocessor.
   *
   * @param newEventIndex Index of the new event in the table.
   */
  void add_event(int newEventIndex) override;

  /**
   * @brief Destructor that cleans up rolling windows.
   */
  virtual ~PreProcessor() {
    for (auto rollWindow : _rollingWindows) {
      delete rollWindow;
    }
  }
};
