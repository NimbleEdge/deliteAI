/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <vector>

#include "aggregate_column.hpp"
#include "user_events_struct.hpp"

/**
 * @brief Abstract base class for rolling window aggregation operations.
 *
 * This class provides the foundation for implementing time-based or count-based
 * rolling windows that maintain aggregated statistics over a sliding window of events.
 * It manages aggregate columns for different groups and provides virtual methods
 * for adding events and updating the window state.
 */
class RollingWindow {
  using T = double; /**< Type alias for the data type used in aggregations. */

 public:
  int _preprocessorId; /**< Identifier of the preprocessor this rolling window belongs to. */
  PreProcessorInfo _preprocessorInfo; /**< Configuration information for the preprocessor. */
  std::map<std::string, std::vector<AggregateColumn*>> _groupWiseAggregatedColumnMap; /**< Map of group names to their aggregate columns for different operations. */

  /**
   * @brief Creates aggregate columns for a specific group.
   *
   * This method initializes the appropriate aggregate columns (Sum, Count, Min, Max, Avg)
   * for each column that needs to be aggregated within the specified group.
   *
   * @param group Group identifier for which to create aggregate columns.
   * @param columnIds Vector of column indices to aggregate.
   * @param totalFeatureVector Reference to the feature vector where aggregated values will be stored.
   * @param rollingWindowFeatureStartIndex Starting index in the feature vector for this rolling window's features.
   * @return true if aggregate columns were created successfully, false otherwise.
   */
  bool create_aggregate_columns_for_group(const std::string& group,
                                          const std::vector<int>& columnIds,
                                          std::vector<double>& totalFeatureVector,
                                          int rollingWindowFeatureStartIndex);

  /**
   * @brief Adds a new event to the rolling window.
   *
   * This virtual method must be implemented by derived classes to handle
   * the addition of new events to the rolling window.
   *
   * @param allEvents Vector containing all events in the table.
   * @param newEventIndex Index of the new event to be added.
   */
  virtual void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) = 0;

  /**
   * @brief Updates the rolling window state.
   *
   * This virtual method must be implemented by derived classes to update
   * the window state, typically by removing expired events and recalculating
   * aggregations.
   *
   * @param allEvents Vector containing all events in the table.
   */
  virtual void update_window(const std::vector<TableEvent>& allEvents) = 0;

  /**
   * @brief Constructor for RollingWindow.
   *
   * @param preprocessorId Identifier of the preprocessor this rolling window belongs to.
   * @param info Configuration information for the preprocessor.
   */
  RollingWindow(int preprocessorId, const PreProcessorInfo& info) {
    _preprocessorInfo = info;
    _preprocessorId = preprocessorId;
  }

  /**
   * @brief Virtual destructor that cleans up aggregate columns.
   *
   * Ensures proper cleanup of all aggregate column objects to prevent memory leaks.
   */
  virtual ~RollingWindow() {
    for (auto it : _groupWiseAggregatedColumnMap) {
      for (auto col : it.second) {
        delete col;
      }
    }
  }
};
