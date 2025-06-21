/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>

namespace dbconstants {
static inline const std::string EventsTableName = "Events";
static inline const std::string EventsTypeTableName = "EventsType";
static inline const std::string EventTypeColumnName = "eventType";
static inline const std::string TimeStampColumnName = "TIMESTAMP";
static inline const std::string EventColumnName = "event";
static inline float MaxDBSizeKBs = 5000;  // 5MB
}  // namespace dbconstants
