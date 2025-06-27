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

void from_json(const json& j, TableInfo& tableInfo);

class CommandCenter;
class FunctionDef;

class UserEventsManager {
  std::shared_ptr<const Config> _config = nullptr;
  Database* _database = nullptr;
  std::map<std::string, std::shared_ptr<RawStore>> _rawStoreMap;
  std::map<std::string, std::string> _modelInputToTableNameMap;
  std::unordered_map<std::string, OpReturnType> _rawEventsTypesToPreProcess;
  bool create_tables(const std::shared_ptr<const Config> config);
  bool _debugMode = false;
  CommandCenter* _commandCenter;

 public:
  UserEventsManager(CommandCenter* commandCenter, Database* database,
                    std::shared_ptr<const Config> config);

  BasePreProcessor* create_preprocessor(const json& preprocessorJson, DATATYPE dataType);

  BasePreProcessor* create_preprocessor(const PreProcessorInfo& preProcessorInfo);
  UserEventsData add_event(const std::string& eventMapJsonString, const std::string& eventType);
  UserEventsData add_event(const OpReturnType event, const std::string& eventType);
  nlohmann::json get_metrics();
#ifdef SCRIPTING
  bool delete_old_entries_from_eventsType_Table();
  void script_loaded_trigger();
  std::shared_ptr<RawStore> create_raw_store(const std::string& eventType,
                                             const std::string& expiryType, int expiryValue);
  void add_pre_event_hook(OpReturnType functionDataVariable, std::vector<std::string>&& types);

#endif
#ifdef TESTING
  bool add_eventType(const std::string& tableName, const nlohmann::json& schema);
  int get_count_from_eventsTable(const std::string& tableName);
#endif
};
