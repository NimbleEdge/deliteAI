/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <shared_mutex>

#include "database.hpp"
#include "single_variable.hpp"
#include "table_store.hpp"
#include "user_events_constants.hpp"
#include "user_events_struct.hpp"

class Database;
class CommandCenter;

class RawStore {
  TableStore* _tableStore = nullptr;
  CommandCenter* _commandCenter = nullptr;
  Database* _database = nullptr;
  OpReturnType _functionDataVariable;
  std::string _eventType;
  bool _eventHookSet = false;

 public:
  RawStore(CommandCenter* commandCenter, Database* database, const TableInfo& tableInfo) {
    _eventType = tableInfo.name;
    _database = database;
    _commandCenter = commandCenter;
    if (!_database->update_eventsType_table(_eventType)) {
      LOG_TO_ERROR("Could not update %s table with eventType=%s.",
                   dbconstants::EventsTypeTableName.c_str(), _eventType.c_str());
    }
    if (!_database->delete_old_rows_from_table_in_db(_eventType, "time",
                                                     tableInfo.expiryTimeInMins)) {
      LOG_TO_ERROR("Could not delete old rows from the table %s ", _eventType.c_str());
    }
    _tableStore = new TableStore(tableInfo.schema);
    auto events = _database->get_events_from_db(_eventType);
    for (auto& event : events) {
      TableRow r;
      for (const auto& column : event.items()) {
        r.row[column.key()] = DataVariable::get_SingleVariableFrom_JSON(column.value());
      }
      r.timestamp = event[usereventconstants::TimestampField];
      _tableStore->add_row(r);
    }
  }

  RawStore(CommandCenter* commandCenter, Database* database, const std::string& eventType,
           const std::string& expiryType, int expiryValue) {
    _commandCenter = commandCenter;
    _database = database;
    _eventType = eventType;
    if (!_database->update_eventsType_table(_eventType)) {
      LOG_TO_ERROR("Could not update %s table with eventType=%s.",
                   dbconstants::EventsTypeTableName.c_str(), _eventType.c_str());
    }
    if (!_database->delete_old_rows_from_table_in_db(_eventType, expiryType, expiryValue)) {
      LOG_TO_ERROR("Could not delete old rows from the table %s ", _eventType.c_str());
    }
    // reads events from database only on the add_event hook, as it is not required before that
  }

  void set_add_event_hook(OpReturnType functionDataVariable);
  bool add_event(OpReturnType eventMapTable);

  BasePreProcessor* create_processor(const PreProcessorInfo& info) {
    if (!_tableStore) {
      return nullptr;
    }
    return _tableStore->create_preprocessor(info);
  }

  ~RawStore() { delete _tableStore; }
};
