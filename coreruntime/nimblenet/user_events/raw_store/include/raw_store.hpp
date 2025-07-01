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

/**
 * @brief Manages raw event storage with optional preprocessing capabilities.
 *
 * Provides a bridge between on-disk database and in-memory table storage.
 * Can operate with TableStore for structured processing or as an event
 * hook manager in the script.
 */
class RawStore {
  TableStore* _tableStore = nullptr; /**< Table store for structured processing, null if not used. */
  CommandCenter* _commandCenter = nullptr; /**< Command center for system coordination. */
  Database* _database = nullptr; /**< On-disk custom format database for persistent storage. */
  OpReturnType _functionDataVariable; /**< Function executed when events are added. */
  std::string _eventType; /**< Type of events managed by this store. */
  bool _eventHookSet = false; /**< Flag indicating whether an event hook has been set. */

 public:
  /**
   * @brief Constructor for RawStore with table schema from frontend.
   *
   * Creates RawStore with full table processing capabilities. Initializes
   * database table, loads existing events, and sets up TableStore.
   *
   * @param commandCenter Pointer to the command center instance.
   * @param database Pointer to the on-disk custom format database.
   * @param tableInfo Configuration information for the table including schema and expiry settings.
   */
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

  /**
   * @brief Constructor for schema-less RawStore created through script.
   *
   * Creates RawStore for script-based event processing without structured
   * table storage. Events are processed through hooks.
   *
   * @param commandCenter Pointer to the command center instance.
   * @param database Pointer to the on-disk custom format database.
   * @param eventType Type/category of events to manage.
   * @param expiryType Type of expiry mechanism (e.g., "time", "count").
   * @param expiryValue Value for the expiry mechanism.
   */
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

  /**
   * @brief Sets a function hook to be executed when events are added.
   *
   * Sets up a callback function for event processing and processes
   * any existing events from the database through this hook.
   *
   * @param functionDataVariable Function to be executed as an event hook.
   */
  void set_add_event_hook(OpReturnType functionDataVariable);

  /**
   * @brief Adds a new event to the store.
   *
   * If TableStore is available, processes event through structured storage.
   * Otherwise, executes the event hook function.
   *
   * @param eventMapTable Event data as a map of key-value pairs.
   * @return true if the event was added successfully, false otherwise.
   */
  bool add_event(OpReturnType eventMapTable);

  /**
   * @brief Creates a preprocessor for this store.
   *
   * Delegates to underlying TableStore. Returns nullptr if no TableStore available.
   *
   * @param info Configuration information for the preprocessor.
   * @return Pointer to the created BasePreProcessor, or nullptr if creation fails.
   */
  BasePreProcessor* create_processor(const PreProcessorInfo& info) {
    if (!_tableStore) {
      return nullptr;
    }
    return _tableStore->create_preprocessor(info);
  }

  /**
   * @brief Destructor that cleans up the TableStore.
   */
  ~RawStore() { delete _tableStore; }
};
