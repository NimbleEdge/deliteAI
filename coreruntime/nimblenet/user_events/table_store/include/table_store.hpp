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

class TableStore {
  std::shared_ptr<const Config> _config = nullptr;
  std::string _tableName;
  std::shared_ptr<TableData> _tableData = std::make_shared<TableData>();
  std::vector<BasePreProcessor*> _preprocessors;
  bool _isInvalid = false;

  void update_column_meta_data(const std::string& columnName);
  std::vector<std::string> get_groups_from_json(int preprocessorIndex,
                                                const nlohmann::json& preprocessorInput);

 public:
  BasePreProcessor* create_preprocessor(const PreProcessorInfo& preprocessorInfo);
  void add_row(const TableRow& r);
  bool verify_key(const std::string& key, OpReturnType val);
  TableStore(const std::map<std::string, int>& schema);
  TableStore() {};
  TableStore(const TableStore& other) = delete;
  TableStore(TableStore&& other) = default;
  TableStore& operator=(const TableStore& other) = delete;
  TableStore& operator=(TableStore&& other) = default;

  std::shared_ptr<TableData> get_data() const { return _tableData; }

  ~TableStore();

#ifdef TESTING
  TableStore(CommandCenter* commandCenter, const std::string& tableName,
             const nlohmann::json& schema);
#endif
};
