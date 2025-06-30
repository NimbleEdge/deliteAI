/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"

class CommandCenter;
class BasePreProcessor;
class TableStore;

/**
 * @brief Data variable for managing NimbleNet pre-processor operations
 *
 * This class provides a data variable interface for configuring and executing
 * pre-processing operations on data before it's fed into machine learning models.
 * It supports rolling window operations, group-by functionality, and custom
 * computations that can be applied to the data.
 *
 * The pre-processor must be configured with rolling window parameters, group-by
 * columns, and computations before it can be created and used for data processing.
 */
class PreProcessorNimbleNetVariable final : public DataVariable {
  CommandCenter* _commandCenter; /**< Pointer to the command center for system operations */
  DATATYPE _dataType; /**< The data type this pre-processor operates on */
  OpReturnType _rollingWindow; /**< Rolling window configuration parameters */
  OpReturnType _groupByColumns; /**< Columns to group by for processing */
  // Stores vector of tuples, where each tuple will have columnName, operator and default value
  std::vector<OpReturnType> _computations; /**< Vector of computation configurations as tuples */
  bool _isPreProcessorCreated = false; /**< Flag indicating if the pre-processor has been created */
  BasePreProcessor* _processor = nullptr; /**< Pointer to the actual pre-processor instance */
  std::shared_ptr<TableStore> _tableStore = nullptr; /**< Shared pointer to the table store */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  OpReturnType add_rolling_window(const std::vector<OpReturnType>& arguments);

  OpReturnType add_groupBy_columns(const std::vector<OpReturnType>& arguments);

  OpReturnType add_computation(const std::vector<OpReturnType>& arguments);

  OpReturnType create_processor(const std::vector<OpReturnType>& arguments);

  OpReturnType get_processor_output(const std::vector<OpReturnType>& arguments);

  OpReturnType get_processor_output_by_group(const std::vector<OpReturnType>& arguments);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  std::string print() override;

  nlohmann::json to_json() const override { return "[PreProcessor]"; }

  BasePreProcessor* create_processor(std::vector<double> defaultVector, DATATYPE dataType);

 public:
  /**
   * @brief Constructs a new PreProcessorNimbleNetVariable
   * @param commandCenter Pointer to the command center for system operations
   * @param tableStore Shared pointer to the table store for data access
   * @param dataType The data type this pre-processor will operate on
   */
  PreProcessorNimbleNetVariable(CommandCenter* commandCenter,
                                std::shared_ptr<TableStore> tableStore, const DATATYPE dataType);
};
