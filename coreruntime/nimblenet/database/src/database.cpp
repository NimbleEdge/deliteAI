/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "database.hpp"

#include <mutex>

#include "command_center.hpp"
#include "data_variable.hpp"
#include "native_interface.hpp"

using namespace std;

int Database::get_db_size(int& dbSize) const {
  std::lock_guard<std::mutex> lockGuard(_mutex);

  // also used for sanity check of database
  dbSize = _eventsStore.size_in_bytes();
  return 0;
}

void Database::database_open() {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (_isSimulation) return;
  _eventsStore.init(nativeinterface::HOMEDIR + "/events/");
  _currentEventTypes = _eventsStore.get_all_types();
  nlohmann::json j;
  int dbSize;
  j["dbSize"] = _eventsStore.size_in_bytes();
  j["numEvents"] = _eventsStore.get_num_events();
  _metricsAgent->save_metrics("DATABASEMETRIC", j);
}

std::vector<nlohmann::json> Database::get_events_from_db(const std::string& tableName) const {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (this->_isSimulation) {
    return {};
  }

  return _eventsStore.read(tableName.c_str());
}

bool Database::delete_old_rows_from_table_in_db(const std::string& tableName,
                                                const std::string& expiryType,
                                                const int64_t expiryValue) const {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (this->_isSimulation) {
    return true;
  }
  if (expiryType == "time") {
    _eventsStore.delete_old_events(tableName.c_str(), Time::get_time() - expiryValue);
    return true;
  } else if (expiryType == "count") {
    _eventsStore.delete_old_events_by_count(tableName.c_str(), expiryValue);
    return true;
  } else {
    THROW("cannot set expiryType=%s for table=%s", expiryType.c_str(), tableName.c_str());
  }
  return true;
}

bool Database::add_event_in_db(const string& tableName, OpReturnType eventMapTable) {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (this->_isSimulation) {
    return true;
  }
  if (_full) {
    LOG_TO_ERROR("%s", "Event not added db is full");
    return false;
  }

  if (!check_tableName_in_eventsType_Table(tableName)) {
    // NEED to comment it out since with cloud relay user events flowing this would be the case most
    // of the time, thus overwhelming our debug logs
    // LOG_TO_DEBUG("TableName=%s not found in %s
    // table, event won't be added to database.",
    //              tableName.c_str(), dbconstants::EventsTypeTableName.c_str());
    return true;
  }
  auto eventJsonString = eventMapTable->to_json_str();
  _eventsStore.write(tableName.c_str(), eventJsonString.c_str());
  return true;
}

bool Database::update_eventsType_table(const std::string& tableName) {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (this->_isSimulation) {
    return true;
  }
  _eventsStore.add_type(tableName.c_str());
  // Store eventType in memory once added to DB
  _eventTypesInMaking.insert(tableName);
  _currentEventTypes.insert(tableName);
  return true;
}

bool Database::check_tableName_in_eventsType_Table(const std::string& tableName) const {
  // Caller is supposed to be holding the mutex when this is called
  if (this->_isSimulation) {
    return true;
  }
  if (_currentEventTypes.find(tableName) == _currentEventTypes.end()) {
    return false;
  }
  return true;
}

bool Database::delete_old_entries_from_eventsType_Table() {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  if (this->_isSimulation) {
    return true;
  }
  _currentEventTypes = _eventTypesInMaking;
  auto allTypes = _eventsStore.get_all_types();
  for (auto type : allTypes) {
    if (_currentEventTypes.find(type) == _currentEventTypes.end()) {
      // EventType not required anymore, deleting
      _eventsStore.delete_type(type.c_str());
    }
  }
  return true;
}

#ifdef TESTING

int Database::get_count_from_eventsTable(const std::string& eventType) const {
  std::lock_guard<std::mutex> lockGuard(_mutex);
  return _eventsStore.get_num_events(eventType.c_str());
}

#endif
