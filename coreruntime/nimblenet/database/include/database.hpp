/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <mutex>
#include <set>
#include <string>

#include "file_store.hpp"
#include "json.hpp"
#include "ne_fwd.hpp"

class MetricsAgent;

class Database {
 private:
  mutable std::mutex _mutex;

#ifdef SIMULATION_MODE
  bool _isSimulation = true;
#else
  bool _isSimulation = false;
#endif
  MetricsAgent* _metricsAgent = nullptr;
  std::set<std::string> _currentEventTypes;
  std::set<std::string> _eventTypesInMaking;
  Store<StoreType::METRICS> _eventsStore;
  bool check_tableName_in_eventsType_Table(const std::string& tableName) const;
  bool _full = false;

 public:
  Database(MetricsAgent* metricsAgent) {
    _metricsAgent = metricsAgent;
    database_open();
  }

  void set_full() { _full = true; }

  void database_open();
  bool delete_old_rows_from_table_in_db(const std::string& tableName, const std::string& expiryType,
                                        const int64_t expiryValue) const;
  std::vector<nlohmann::json> get_events_from_db(const std::string& tableName) const;
  bool add_event_in_db(const std::string& tableName, OpReturnType eventMapTable);
  bool update_eventsType_table(const std::string& tableName);
  int get_rows_in_events_table();
  bool delete_old_entries_from_eventsType_Table();
  int get_db_size(int& dbSize) const;

#ifdef TESTING
  int get_count_from_eventsTable(const std::string& eventType) const;
#endif
};
