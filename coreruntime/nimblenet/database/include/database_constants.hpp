/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

/**
 * @brief Namespace containing constants used for database operations related to events.
 */
namespace dbconstants {

/**
 * @brief Name of the table that stores raw event entries.
 */
static inline const std::string EventsTableName = "Events";

/**
 * @brief Name of the table that stores event type definitions.
 */
static inline const std::string EventsTypeTableName = "EventsType";

/**
 * @brief Column name used to store the event type in the Events table.
 */
static inline const std::string EventTypeColumnName = "eventType";

/**
 * @brief Column name used to store timestamps in Events table.
 */
static inline const std::string TimeStampColumnName = "TIMESTAMP";

/**
 * @brief Column name used to store event payloads in Events table.
 */
static inline const std::string EventColumnName = "event";

/**
 * @brief Maximum allowed database size in kilobytes.
 *
 * This value corresponds to 5MB (5000 KB) and is used to limit local storage size.
 */
static inline float MaxDBSizeKBs = 5000;

}  // namespace dbconstants
