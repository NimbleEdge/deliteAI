/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "aggregate_column.hpp"

/**
 * @brief Counts the number of events over a rolling window.
 *
 * This class maintains a running count of events for a specific
 * column and group.
 */
class CountColumn final : public AggregateColumn {
  using T = double; /**< Type alias for the aggregation data type. */
  int _oldestIndex = -1; /**< Index of the oldest event included in the aggregation. */

 public:
  /**
   * @brief Constructor for CountColumn.
   *
   * @param preprocessorId Identifier of the parent preprocessor.
   * @param columnId Index of the column to aggregate.
   * @param group Group identifier for this aggregation.
   * @param storePtr Pointer to where the aggregated value is stored.
   */
  CountColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr)
      : AggregateColumn(preprocessorId, columnId, group, storePtr) {};

  /**
   * @brief Increments the event count.
   *
   * @param allEvents Vector of all events in the table.
   * @param newEventIndex Index of the new event to add.
   */
  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;
  
  /**
   * @brief Decrements the event count for expired events.
   *
   * @param allEvents Vector of all events in the table.
   * @param oldestValidIndex Index of the oldest event still in the window.
   */
  void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) override;
};
