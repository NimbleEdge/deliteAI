/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include "user_events_struct.hpp"

/**
 * @brief Abstract base class for performing aggregation operations on a column.
 *
 * Defines the interface for different types of aggregations (e.g., Sum, Count, Min, Max, Avg)
 * over a rolling window of events. Manages the state for a specific aggregation
 * on a single column for a particular group.
 */
class AggregateColumn {
  using T = double; /**< Type alias for the aggregation data type. */

 public:
  T* _storeValue = nullptr; /**< Pointer to the location where the aggregated value is stored. */
  int _columnId; /**< Index of the column being aggregated. */
  std::string _group; /**< Group identifier this aggregation belongs to. */
  int _preprocessorId; /**< Identifier of the preprocessor this aggregation is for. */
  T _defaultValue; /**< Default value for this aggregation. */
  int _totalCount = 0; /**< Total number of events considered in the aggregation. */

  /**
   * @brief Constructor for AggregateColumn.
   *
   * @param preprocessorId Identifier of the parent preprocessor.
   * @param columnId Index of the column to aggregate.
   * @param group Group identifier for this aggregation.
   * @param storePtr Pointer to where the aggregated value is stored.
   */
  AggregateColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr) {
    _preprocessorId = preprocessorId;
    _columnId = columnId;
    _group = group;
    _storeValue = storePtr;
    _defaultValue = *storePtr;
  }

  /**
   * @brief Adds a new event to the aggregation.
   *
   * @param allEvents Vector of all events in the table.
   * @param newEventIndex Index of the new event to add.
   */
  virtual void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) = 0;

  /**
   * @brief Removes expired events from the aggregation.
   *
   * @param allEvents Vector of all events in the table.
   * @param oldestValidIndex Index of the oldest event still in the window.
   */
  virtual void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) = 0;
  
  /**
   * @brief Virtual destructor.
   */
  virtual ~AggregateColumn() = default;
};

#include "average_column.hpp"
#include "count_column.hpp"
#include "max_column.hpp"
#include "min_column.hpp"
#include "sum_column.hpp"

/**
 * @brief Utility function to convert string to a numeric type.
 *
 * @tparam T Numeric type to convert to.
 * @param s String to convert.
 * @return The converted numeric value.
 */
template <typename T>
T GetAs(const std::string& s) {
  std::stringstream ss{s};
  T t;
  if (!(ss >> t)) LOG_TO_ERROR("%s cannot be converted to %s", s.c_str(), typeid(T).name());
  return t;
}
