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

class PreProcessorNimbleNetVariable final : public DataVariable {
  CommandCenter* _commandCenter;
  DATATYPE _dataType;
  OpReturnType _rollingWindow;
  OpReturnType _groupByColumns;
  // Stores vector of tuples, where each tuple will have columnName, operator and default value
  std::vector<OpReturnType> _computations;
  bool _isPreProcessorCreated = false;
  BasePreProcessor* _processor = nullptr;
  std::shared_ptr<TableStore> _tableStore = nullptr;

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
  PreProcessorNimbleNetVariable(CommandCenter* commandCenter,
                                std::shared_ptr<TableStore> tableStore, const DATATYPE dataType);
};
