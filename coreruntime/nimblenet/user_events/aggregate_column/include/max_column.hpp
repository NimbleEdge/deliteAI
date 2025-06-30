/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "aggregate_column.hpp"

/**
 * @brief Calculates the maximum value of a column over a rolling window.
 *
 * This class maintains the running maximum value for a specific column and group.
 * It re-scans the window to find the new maximum if the current one expires.
 */
class MaxColumn final : public AggregateColumn {
  using T = double; /**< Type alias for the aggregation data type. */
  int _oldestIndex = -1; /**< Index of the oldest event included in the aggregation. */

 public:
  /**
   * @brief Constructor for MaxColumn.
   *
   * @param preprocessorId Identifier of the parent preprocessor.
   * @param columnId Index of the column to aggregate.
   * @param group Group identifier for this aggregation.
   * @param storePtr Pointer to where the aggregated value is stored.
   */
  MaxColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr)
      : AggregateColumn(preprocessorId, columnId, group, storePtr) {};

  /**
   * @brief Updates the maximum value if new event's value is higher.
   *
   * @param allEvents Vector of all events in the table.
   * @param newEventIndex Index of the new event to add.
   */
  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;

  /**
   * @brief Removes expired events and recalculates the maximum if necessary.
   *
   * @param allEvents Vector of all events in the table.
   * @param oldestValidIndex Index of the oldest event still in the window.
   */
  void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) override;
};
