/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "command_center.hpp"
#include "database.hpp"
#include "native_interface.hpp"
#include "user_events_constants.hpp"

using namespace std;

struct CallBackData {
  std::string tableName;
  std::vector<nlohmann::json> events;
};

static int emptyCallback(void* input, int argc, char** argv, char** azColName) { return 0; }

static int get_rows(void* data, int argc, char** argv, char** azColName) {
  CallBackData* callBackData = (CallBackData*)data;
  std::string tableName = callBackData->tableName;
  int timestampIndex = -1;
  int eventIndex = -1;
  nlohmann::json eventMapTable;

  for (int i = 0; i < argc; i++) {
    if (!strcmp(azColName[i], dbconstants::TimeStampColumnName.c_str())) {
      timestampIndex = i;
    } else if (!strcmp(azColName[i], dbconstants::EventColumnName.c_str())) {
      eventIndex = i;
    }
  }
  if (eventIndex == -1) {
    LOG_TO_ERROR("No event found in table row for eventType=%s", tableName.c_str());
    return 1;
  }
  try {
    eventMapTable = nlohmann::json::parse(argv[eventIndex]);
  } catch (...) {
    LOG_TO_ERROR("Event=%s is not a valid json", argv[eventIndex]);
    return 1;
  }
  if (timestampIndex != -1) {
    eventMapTable[usereventconstants::TimestampField] = stoll(argv[timestampIndex]);
  } else {
    LOG_TO_ERROR("No timestamp found in %s table row for eventType=%s",
                 dbconstants::EventsTableName.c_str(), tableName.c_str());
    return 1;
  }
  callBackData->events.push_back(eventMapTable);
  return 0;
}

static int exists_tableName(void* data, int argc, char** argv, char** azColName) {
  int* res = static_cast<int*>(data);
  *res = std::stoi(argv[0]);
  return 0;
}

int Database::get_db_size(int& dbSize) {
  // also used for sanity check of database
  if (_isSimulation) {
    dbSize = 0;
    return SQLITE_OK;
  }
  sqlite3_stmt* stmt;
  char* sanityCheckCommand;
  asprintf(&sanityCheckCommand, "%s",
           "SELECT page_count * page_size as size FROM pragma_page_count(), pragma_page_size();");
  int sanityCode = sqlite3_prepare_v2(_db, sanityCheckCommand, -1, &stmt, NULL);

  if (sanityCode != SQLITE_OK) {
    LOG_TO_ERROR("Error in Sanity Check for Database with error: %s on command %s",
                 sqlite3_errmsg(_db), sanityCheckCommand);
    sqlite3_finalize(stmt);
    free(sanityCheckCommand);
    return sanityCode;
  }
  free(sanityCheckCommand);
  while ((sanityCode = sqlite3_step(stmt)) == SQLITE_ROW) {
    dbSize = sqlite3_column_int(stmt, 0);
  }
  if (sanityCode != SQLITE_DONE) {
    LOG_TO_ERROR("Error in sanity_check sqlite3_done: %s", sqlite3_errmsg(_db));
    sqlite3_finalize(stmt);
    return sanityCode;
  }
  sqlite3_finalize(stmt);
  return SQLITE_OK;
}

int Database::run_sanity_check_command() {
  int dbSize;
  auto sanityStatus = get_db_size(dbSize);
  if (sanityStatus != SQLITE_OK) {
    return sanityStatus;
  }
  nlohmann::json j;
  j["dbSize"] = dbSize;
  j["numEvents"] = get_rows_in_events_table();
  _metricsAgent->save_metrics("DATABASEMETRIC", j);
  return SQLITE_OK;
}

void Database::remove_database_file() {
  sqlite3_close(_db);
  int didRemove = remove((nativeinterface::HOMEDIR + DEFAULT_SQLITE_DB_NAME).c_str());
  if (didRemove) {
    LOG_TO_ERROR("%s could not be removed from the system. Failed with error %d",
                 DEFAULT_SQLITE_DB_NAME, didRemove);
  } else {
    LOG_TO_ERROR("removed database file %s", DEFAULT_SQLITE_DB_NAME);
  }
}

int Database::open_database_file() {
  int rc = sqlite3_open((nativeinterface::HOMEDIR + DEFAULT_SQLITE_DB_NAME).c_str(), &_db);
  if (rc) {
    LOG_TO_ERROR("Can't open database: %s %s", DEFAULT_SQLITE_DB_NAME, sqlite3_errmsg(_db));
    return rc;
  }
  return rc;
}

bool Database::should_delete(int status) {
  switch (status) {
    case SQLITE_CORRUPT:
    case SQLITE_NOTADB:
      return true;
  }
  return false;
}

void Database::database_open() {
  if (_isSimulation) return;
  int databaseStatus = open_database_file();
  if (databaseStatus) {
    if (should_delete(databaseStatus)) remove_database_file();
    if (open_database_file()) return;
  }
  if (!create_eventsType_table()) {
    LOG_TO_ERROR("Could not create tableName=%s", dbconstants::EventsTypeTableName.c_str());
    return;
  }
  if (!create_events_table()) {
    LOG_TO_ERROR("Could not create tableName=%s", dbconstants::EventsTableName.c_str());
    return;
  }

  int sanityStatus = run_sanity_check_command();
  if (sanityStatus) {
    if (should_delete(sanityStatus)) remove_database_file();
    if (open_database_file()) return;
  }

  LOG_TO_INFO("Opened database=%s successfully", DEFAULT_SQLITE_DB_NAME);
}

bool Database::database_processor(const std::string& tableName,
                                  std::map<std::string, TableStore>& tableStoreMap) {
  if (this->_isSimulation) {
    return true;
  }

  char* zErrMsg = 0;
  // SELECT * from Events WHERE eventType='%s' ORDER BY TIMESTAMP;
  const char* selectCommand = "SELECT * from %s WHERE %s='%s' ORDER BY %s;";
  char* sqlQueryCommand;
  asprintf(&sqlQueryCommand, selectCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str(),
           dbconstants::TimeStampColumnName.c_str());

  CallBackData callBackData;
  callBackData.tableName = tableName;
  int rc = sqlite3_exec(_db, sqlQueryCommand, get_rows, &callBackData, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in fetching events from table=%s for eventType=%s with error %s",
                 dbconstants::EventsTableName.c_str(), tableName.c_str(), zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }

  return true;
}

std::vector<nlohmann::json> Database::get_events_from_db(const std::string& tableName) {
  if (this->_isSimulation) {
    return {};
  }

  char* zErrMsg = 0;
  // SELECT * from Events WHERE eventType='%s' ORDER BY TIMESTAMP;
  const char* selectCommand = "SELECT * from %s WHERE %s='%s' ORDER BY %s;";
  char* sqlQueryCommand;
  asprintf(&sqlQueryCommand, selectCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str(),
           dbconstants::TimeStampColumnName.c_str());

  CallBackData callBackData;
  callBackData.tableName = tableName;
  int rc = sqlite3_exec(_db, sqlQueryCommand, get_rows, &callBackData, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in fetching events from table=%s for eventType=%s with error %s",
                 dbconstants::EventsTableName.c_str(), tableName.c_str(), zErrMsg);
    sqlite3_free(zErrMsg);
    return {};
  }

  return callBackData.events;
}

bool Database::delete_old_rows_by_count(const std::string& tableName, const int64_t maxEvents) {
  char* zErrMsg = 0;
  // DELETE FROM Events where eventType=%s AND TIMESTAMP<%ld;
  const char* tableDeleteRowsCommand =
      "DELETE FROM %s WHERE %s = '%s' and TIMESTAMP NOT IN(SELECT TIMESTAMP FROM "
      "%s where %s = '%s' ORDER "
      "BY TIMESTAMP DESC LIMIT %ld);";
  char* sqlQueryCommand;
  asprintf(&sqlQueryCommand, tableDeleteRowsCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str(),
           dbconstants::EventsTableName.c_str(), dbconstants::EventTypeColumnName.c_str(),
           tableName.c_str(), maxEvents);
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR(
        "Error in Deleting old rows from Table %s with eventType=%s and maxEvents=%ld with "
        "error=%s",
        dbconstants::EventsTableName.c_str(), tableName.c_str(), maxEvents, zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  LOG_TO_DEBUG("Deleted old rows from Table %s where eventType=%s in DB successfully",
               dbconstants::EventsTableName.c_str(), tableName.c_str());
  return true;
}

bool Database::delete_old_rows_by_expiryTime(const std::string& tableName,
                                             const int64_t expiryTimeInMins) {
  char* zErrMsg = 0;
  // DELETE FROM Events where eventType=%s AND TIMESTAMP<%ld;
  const char* tableDeleteRowsCommand = "DELETE FROM %s where %s='%s' AND %s <%ld";
  char* sqlQueryCommand;
  long expiryTimestamp = Time::get_time() - expiryTimeInMins * 60;  // change the time func
  asprintf(&sqlQueryCommand, tableDeleteRowsCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str(),
           dbconstants::TimeStampColumnName.c_str(), expiryTimestamp);
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR(
        "Error in Deleting old rows from Table %s with eventType=%s and expiryTimeStamp=%ld with "
        "error=%s",
        dbconstants::EventsTableName.c_str(), tableName.c_str(), expiryTimestamp, zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  LOG_TO_DEBUG("Deleted old rows from Table %s where eventType=%s in DB successfully",
               dbconstants::EventsTableName.c_str(), tableName.c_str());
  return true;
}

bool Database::delete_old_rows_from_table_in_db(const std::string& tableName,
                                                const std::string& expiryType,
                                                const int64_t expiryValue) {
  if (this->_isSimulation) {
    return true;
  }
  if (expiryType == "time") {
    return delete_old_rows_by_expiryTime(tableName, expiryValue);
  } else if (expiryType == "count") {
    return delete_old_rows_by_count(tableName, expiryValue);
  }
  THROW("Cannot set expiryType=%s for table=%s", expiryType.c_str(), tableName.c_str());
}

bool Database::add_event_in_db(const string& tableName, OpReturnType eventMapTable) {
  if (this->_isSimulation) {
    return true;
  }
  if (_full) {
    LOG_TO_ERROR("%s", "Event not added db is full");
    return false;
  }
  if (!check_tableName_in_eventsType_Table(tableName)) {
    LOG_TO_DEBUG("TableName=%s not found in %s table, event won't be added to database.",
                 tableName.c_str(), dbconstants::EventsTypeTableName.c_str());
    return true;
  }

  char* zErrMsg = 0;
  // INSERT INTO Events (TIMESTAMP, eventType, event) VALUES (%ld, '%s', '%s')
  const char* tableInserCommand = "INSERT INTO %s (%s, %s, %s) VALUES (%ld, '%s', '%s');";
  char* sqlQueryCommand;
  std::string eventDump = eventMapTable->to_json_str();
  asprintf(&sqlQueryCommand, tableInserCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::TimeStampColumnName.c_str(), dbconstants::EventTypeColumnName.c_str(),
           dbconstants::EventColumnName.c_str(), long(Time::get_time()), tableName.c_str(),
           eventDump.c_str());
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in Inserting event=%s to Table %s with eventType=%s with error %s",
                 eventDump.c_str(), dbconstants::EventsTableName.c_str(), tableName.c_str(),
                 zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  return true;
}

bool Database::create_eventsType_table() {
  if (this->_isSimulation) {
    return true;
  }

  char* zErrMsg = 0;
  const char* createTableCommand = "CREATE TABLE IF NOT EXISTS %s (%s TEXT UNIQUE);";

  char* sqlQueryCommand;
  asprintf(&sqlQueryCommand, createTableCommand, dbconstants::EventsTypeTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str());
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in Creating %s Table with error %s",
                 dbconstants::EventsTypeTableName.c_str(), zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  return true;
}

bool Database::update_eventsType_table(const std::string& tableName) {
  if (this->_isSimulation) {
    return true;
  }

  char* zErrMsg = 0;
  const char* insertOrReplaceSql = "INSERT OR IGNORE INTO %s (%s) VALUES ('%s')";
  char* sqlQueryCommand = 0;
  asprintf(&sqlQueryCommand, insertOrReplaceSql, dbconstants::EventsTypeTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str());
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in inserting %s in EventTypes Table with error %s", tableName.c_str(),
                 zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  // Store eventType in memory once added to DB
  _eventTypes.insert(tableName);
  return true;
}

bool Database::create_events_table() {
  if (this->_isSimulation) {
    return true;
  }

  char* zErrMsg = 0;
  // CREATE TABLE IF NOT EXISTS Events (event TEXT, TIMESTAMP INTEGER, eventType TEXT);
  const char* createTableCommand = "CREATE TABLE IF NOT EXISTS %s (%s TEXT, %s INTEGER, %s TEXT);";
  char* sqlQueryCommand = 0;
  asprintf(&sqlQueryCommand, createTableCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventColumnName.c_str(), dbconstants::TimeStampColumnName.c_str(),
           dbconstants::EventTypeColumnName.c_str());
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in Creating %s Table with error %s", dbconstants::EventsTableName.c_str(),
                 zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  return true;
}

bool Database::check_tableName_in_eventsType_Table(const std::string& tableName) {
  if (this->_isSimulation) {
    return true;
  }
  if (_eventTypes.find(tableName) != _eventTypes.end()) {
    return true;
  }
  char* zErrMsg = 0;
  const char* existsCommand = "SELECT EXISTS(SELECT 1 FROM %s WHERE %s = '%s');";
  char* sqlQueryCommand = 0;
  asprintf(&sqlQueryCommand, existsCommand, dbconstants::EventsTypeTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), tableName.c_str());
  int result = 0;
  int rc = sqlite3_exec(_db, sqlQueryCommand, exists_tableName, &result, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in checking if tableName=%s is present in EventsTypes table with error %s",
                 tableName.data(), zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  if (result == 1) {
    return true;
  }
  return false;
}

bool Database::delete_old_entries_from_eventsType_Table() {
  if (this->_isSimulation) {
    return true;
  }
  char* zErrMsg = 0;
  const char* deleteCommand = "DELETE FROM %s WHERE %s NOT IN (%s)";

  std::string currentEventTypes = "";
  for (const auto& it : _eventTypes) {
    currentEventTypes += "'" + it + "',";
  }
  // Remove last character(,) from the command
  if (currentEventTypes.size() > 0) {
    currentEventTypes.pop_back();
  }

  char* sqlQueryCommand = 0;
  // DELETE FROM EventsType WHERE eventType NOT IN ('%s', '%s')
  asprintf(&sqlQueryCommand, deleteCommand, dbconstants::EventsTypeTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), currentEventTypes.c_str());
  int result = 0;
  int rc = sqlite3_exec(_db, sqlQueryCommand, emptyCallback, 0, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in deleting old eventTypes from tableName=%s with error %s",
                 dbconstants::EventsTypeTableName.c_str(), zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  return true;
}

static int callback(void* count, int argc, char** argv, char** azColName) {
  int* c = static_cast<int*>(count);
  *c = atoi(argv[0]);
  return 0;
}

int Database::get_rows_in_events_table() {
  char* zErrMsg = 0;
  const char* countCommand = "SELECT COUNT(*) FROM %s;";
  char* sqlQueryCommand = 0;
  asprintf(&sqlQueryCommand, countCommand, dbconstants::EventsTableName.c_str());
  int result = 0;
  int rc = sqlite3_exec(_db, sqlQueryCommand, callback, &result, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in getting count from %s table with error %s",
                 dbconstants::EventsTableName.c_str(), zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }
  return result;
}

#ifdef TESTING

int Database::get_count_from_eventsTable(const std::string& eventType) {
  char* zErrMsg = 0;
  const char* countCommand = "SELECT COUNT(*) FROM %s WHERE %s = '%s';";
  char* sqlQueryCommand = 0;
  asprintf(&sqlQueryCommand, countCommand, dbconstants::EventsTableName.c_str(),
           dbconstants::EventTypeColumnName.c_str(), eventType.c_str());
  int result = 0;
  int rc = sqlite3_exec(_db, sqlQueryCommand, callback, &result, &zErrMsg);
  free(sqlQueryCommand);
  if (rc != SQLITE_OK) {
    LOG_TO_ERROR("Error in getting count from %s table for eventType=%s with error %s",
                 dbconstants::EventsTableName.c_str(), eventType.data(), zErrMsg);
    sqlite3_free(zErrMsg);
    return -1;
  }
  return result;
}

#endif
