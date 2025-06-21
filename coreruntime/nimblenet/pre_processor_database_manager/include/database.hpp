/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <sqlite3.h>

#include <set>
#include <string>
#include <unordered_map>

#include "database_constants.hpp"
#include "table_store.hpp"
#include "time_manager.hpp"
#include "user_events_struct.hpp"

class MetricsAgent;

class Database {
 private:
#ifdef SIMULATION_MODE
  bool _isSimulation = true;
#else
  bool _isSimulation = false;
#endif
  sqlite3* _db = NULL;
  MetricsAgent* _metricsAgent = nullptr;
  std::set<std::string> _eventTypes;
  bool _full = false;
  int open_database_file();
  int run_sanity_check_command();
  void remove_database_file();
  static bool should_delete(int status);
  bool check_tableName_in_eventsType_Table(const std::string& tableName);
  bool create_eventsType_table();
  bool create_events_table();
  bool delete_old_rows_by_expiryTime(const std::string& tableName, const int64_t expiryTimeInMins);
  bool delete_old_rows_by_count(const std::string& tableName, const int64_t maxEvents);

 public:
  Database(MetricsAgent* metricsAgent) : _db(nullptr) {
    _metricsAgent = metricsAgent;
    database_open();
  };

  void set_full() { _full = true; }

  void database_open();
  bool delete_old_rows_from_table_in_db(const std::string& tableName, const std::string& expiryType,
                                        const int64_t expiryTimeInMins);
  bool database_processor(const std::string& tableName,
                          std::map<std::string, TableStore>& tableStoreMap);
  bool add_event_in_db(const std::string& tableName, const json& eventMapTable);
  bool add_event_in_db(const std::string& tableName, const OpReturnType eventMapTable);
  std::vector<nlohmann::json> get_events_from_db(const std::string& tableName);
  bool update_eventsType_table(const std::string& tableName);
  int get_rows_in_events_table();
  bool delete_old_entries_from_eventsType_Table();
  int get_db_size(int& dbSize);

#ifdef TESTING
  int get_count_from_eventsTable(const std::string& eventType);
#endif

  ~Database() { sqlite3_close_v2(_db); }
};
