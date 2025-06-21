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

/**
 * @brief Class responsible for managing database operations such as storing,
 * retrieving, and managing event-related data. The data is stored in custom file format using
 * _eventsStore.
 */
class Database {
 private:
  /** @brief Mutex to synchronize access to the database operations. */
  mutable std::mutex _mutex;

#ifdef SIMULATION_MODE
  /** @brief Flag indicating if the database operation is triggered from nimblenet_py.. */
  bool _isSimulation = true;
#else
  /** @brief Flag indicating if the database operation is triggered from nimblenet_py. */
  bool _isSimulation = false;
#endif

  /** @brief Pointer to a MetricsAgent used for logging database related metrics. */
  MetricsAgent* _metricsAgent = nullptr;

  /** @brief Set of current event types for which at least event is present in the disk. */
  std::set<std::string> _currentEventTypes;

  /** @brief Set of event types being added in the current session. */
  std::set<std::string> _eventTypesInMaking;

  /** @brief Underlying storage to add/update/delete events data. */
  Store<StoreType::METRICS> _eventsStore;

  /** @brief Flag indicating if the database has reached its full capacity. */
  bool _full = false;

  /**
   * @brief Checks if a given table name exists in the event types table.
   *
   * @param tableName Name of the table to check.
   * @return true if the table name exists; false otherwise.
   */
  bool check_tableName_in_eventsType_Table(const std::string& tableName) const;

 public:
  /**
   * @brief Constructs a Database instance with a given metrics agent.
   *
   * @param metricsAgent Pointer to a MetricsAgent for reporting metrics.
   */
  Database(MetricsAgent* metricsAgent) {
    _metricsAgent = metricsAgent;
    database_open();
  }

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
                                        const int64_t expiryValue) const;

  /**
   * @brief Retrieves all events from a specified table.
   *
   * @param tableName Name of the table to query.
   * @return Vector of JSON objects representing the events.
   */
  std::vector<nlohmann::json> get_events_from_db(const std::string& tableName) const;

  /**
   * @brief Adds a new event entry to the specified table.
   *
   * @param tableName Name of the table to insert into.
   * @param eventMapTable Map structure representing the event data.
   * @return true on successful insertion; false otherwise.
   */
  bool add_event_in_db(const std::string& tableName, OpReturnType eventMapTable);

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
   * @brief Retrieves the current database size in kilobytes.
   *
   * @param dbSize Output parameter to store the database size.
   * @return 0 on success, non-zero on failure.
   */
  int get_db_size(int& dbSize) const;

#ifdef TESTING
  /**
   * @brief Returns the number of events of a given type.
   *
   * @param eventType The type of event to count.
   * @return Number of entries with the specified event type.
   */
  int get_count_from_eventsTable(const std::string& eventType) const;
#endif
};
