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

/**
 * @brief Class responsible for managing database operations such as storing,
 * retrieving, and managing event-related data. The data is stored in a sqlite db.
 * sqlite db.
 */
class Database {
 private:
#ifdef SIMULATION_MODE
  /** @brief Flag indicating if the database operation is triggered from nimblenet_py. */
  bool _isSimulation = true;
#else
  /** @brief Flag indicating if the database operation is triggered from nimblenet_py. */
  bool _isSimulation = false;
#endif  // SIMULATION_MODE

  sqlite3* _db = NULL; /**< Sqlite db instance used for CRUD operations. */

  MetricsAgent* _metricsAgent =
      nullptr; /**<Pointer to a MetricsAgent used for logging database related metrics. */

  std::set<std::string> _eventTypes; /**< Set of event types stored in the DB. */

  bool _full = false; /**< Flag indicating if the database has reached its full capacity */

  /**
   * @brief Initializes the sqlite DB.
   */
  int open_database_file();

  /**
   * @brief Does a sanity check of the DB by checking its size
   */
  int run_sanity_check_command();

  /**
   * @brief Delete database.
   */
  void remove_database_file();

  /**
   * @brief Returns true in case the db is malformed.
   */
  static bool should_delete(int status);

  /**
   * @brief Checks if the tableName exists in eventsType table.
   *
   * @param tableName eventType to check
   */
  bool check_tableName_in_eventsType_Table(const std::string& tableName);

  /**
   * @brief Create eventsType table in DB.
   */
  bool create_eventsType_table();

  /**
   * @brief Create events table in DB.
   */
  bool create_events_table();

  /**
   * @brief Delete old rows from table before expiry time.
   *
   * @param tableName table from which old rows to remove
   * @param expiryTimeInMins expiry timestamp from current before which all rows to remove
   */
  bool delete_old_rows_by_expiryTime(const std::string& tableName, const int64_t expiryTimeInMins);

  /**
   * @brief Delete old rows from table based on count of events.
   *
   * @param tableName table from which old rows to remove
   * @param maxEvents maximum number of events to keep in the table
   */
  bool delete_old_rows_by_count(const std::string& tableName, const int64_t maxEvents);

 public:
  /**
   * @brief Constructor to Create DB.
   */
  Database(MetricsAgent* metricsAgent) : _db(nullptr) {
    _metricsAgent = metricsAgent;
    database_open();
  };

  /**
   * @brief Marks the database as full (at capacity).
   */
  void set_full() { _full = true; }

  /**
   * @brief Opens or initializes the database.
   */
  void database_open();

  /**
   * @brief Deletes old rows from a specified table based on expiry condition.
   *
   * @param tableName Name of the table to clean up.
   * @param expiryType The field to use for expiry comparison.
   * @param expiryValue Rows older than this value are deleted.
   * @return true on successful deletion; false otherwise.
   */
  bool delete_old_rows_from_table_in_db(const std::string& tableName, const std::string& expiryType,
                                        const int64_t expiryTimeInMins);
  /**
   * @brief Adds event in table.
   *
   * @param tableName eventType table in which to add the event.
   * @param eventMapTable event to be added in the table.
   */
  bool add_event_in_db(const std::string& tableName, const OpReturnType eventMapTable);

  /**
   * @brief Get all the events stored in Events table filtering by tableName.
   *
   * @param tableName eventType to be used for filtering.
   */
  std::vector<nlohmann::json> get_events_from_db(const std::string& tableName);

  /**
   * @brief Updates the events type table with a new or modified table name.
   *
   * @param tableName Event type table name to be added or updated.
   * @return true on successful update; false otherwise.
   */
  bool update_eventsType_table(const std::string& tableName);

  /**
   * @brief Retrieves the number of rows currently present in the main events table.
   *
   * @return Row count in the events table.
   */
  int get_rows_in_events_table();

  /**
   * @brief Deletes outdated entries from the events type table.
   *
   * @return true if deletion is successful; false otherwise.
   */
  bool delete_old_entries_from_eventsType_Table();

  /**
   * @brief Retrieves the current database size in bytes.
   *
   * @param dbSize Output parameter to store the database size.
   * @return 0 on success, non-zero on failure.
   */
  int get_db_size(int& dbSize);

#ifdef TESTING
  /**
   * @brief Returns the number of events of a given type.
   *
   * @param eventType The type of event to count.
   * @return Number of entries with the specified event type.
   */
  int get_count_from_eventsTable(const std::string& eventType);
#endif

  ~Database() { sqlite3_close_v2(_db); }
};
