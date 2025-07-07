/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "pre_processor_nimble_net_variable.hpp"
#include "user_events_struct.hpp"

class CommandCenter;

class TableEventDataVariable final : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  TableEvent* const _eventPtr;
  std::map<std::string, int>* const _headerMapPtr;

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::TABLE_EVENT; }

  OpReturnType get_string_subscript(const std::string& key) override;

  std::string print() override { return DataVariable::fallback_print(); }

  nlohmann::json to_json() const override { return "[TableEvent]"; }

 public:
  TableEventDataVariable(TableEvent* event, std::map<std::string, int>* headerMapPtr)
      : _eventPtr(event), _headerMapPtr(headerMapPtr) {}
};

class FilteredDataframeVariable final : public DataVariable {
  std::shared_ptr<TableData> _tableData = nullptr;
  std::vector<int> _selectedIndices;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::FILTERED_DATAFRAME; }

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;
  OpReturnType filter_all(const std::vector<OpReturnType>& arguments);
  OpReturnType filter_by_function(const std::vector<OpReturnType>& arguments, CallStack& stack);

  FilteredDataframeVariable(std::shared_ptr<TableData> tableData,
                            std::vector<int>&& selectedIndices)
      : _tableData(tableData), _selectedIndices(std::move(selectedIndices)) {}

  OpReturnType feature_fetch_timestamp(OpReturnType typeArgument);

  nlohmann::json to_json() const override { return "[FilteredDataFrame]"; }

 public:
  static OpReturnType all_events(std::shared_ptr<TableData> tableData);
  static OpReturnType events_filtered_by_function(std::shared_ptr<TableData> tableData,
                                                  OpReturnType func, CallStack& stack);
  OpReturnType feature_fetch(const std::vector<OpReturnType>& arguments);

  std::string print() override { return DataVariable::fallback_print(); }
};

class DataframeVariable final : public DataVariable {
  CommandCenter* _commandCenter = nullptr;
  std::shared_ptr<TableStore> _tableStore = nullptr;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::DATAFRAME; }

  OpReturnType filter_all(const std::vector<OpReturnType>& arguments);

  OpReturnType events_filter_by_function(const std::vector<OpReturnType>& arguments,
                                         CallStack& stack);

  OpReturnType append(const std::vector<OpReturnType>& arguments);

  OpReturnType create_processor(const std::vector<OpReturnType>& arguments);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  nlohmann::json to_json() const override { return "[Dataframe]"; }

 public:
  DataframeVariable(CommandCenter* commandCenter_,
                    const std::map<std::string, OpReturnType>& schemaMap);

  std::string print() override { return fallback_print(); }
};
