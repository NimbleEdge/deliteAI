/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "aggregate_column.hpp"

/**
 * @brief Calculates the average of a column's values over a rolling window.
 *
 * This class maintains the running sum and count of events to efficiently
 * compute the average value for a specific column and group.
 */
class AverageColumn final : public AggregateColumn {
  using T = double; /**< Type alias for the aggregation data type. */

  int _oldestIndex = -1; /**< Index of the oldest event included in the aggregation. */
  int _totalCount = 0; /**< Total number of events currently in the aggregation. */
  T _sum = 0; /**< The running sum of the column values. */

 public:
  /**
   * @brief Constructor for AverageColumn.
   *
   * @param preprocessorId Identifier of the parent preprocessor.
   * @param columnId Index of the column to aggregate.
   * @param group Group identifier for this aggregation.
   * @param storePtr Pointer to where the aggregated value is stored.
   */
  AverageColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr)
      : AggregateColumn(preprocessorId, columnId, group, storePtr) {};
  
  /**
   * @brief Adds a new event's value to the running average.
   *
   * @param allEvents Vector of all events in the table.
   * @param newEventIndex Index of the new event to add.
   */
  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;
  
  /**
   * @brief Removes expired events' values from the running average.
   *
   * @param allEvents Vector of all events in the table.
   * @param oldestValidIndex Index of the oldest event still in the window.
   */
  void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) override;
};
