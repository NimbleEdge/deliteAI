/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rolling_window.hpp"

/**
 * @brief Time-based rolling window implementation for event aggregation.
 *
 * This class implements a rolling window that maintains aggregated statistics
 * over a fixed time period. Events older than the specified window time are
 * automatically removed from the aggregation, ensuring that only recent events
 * contribute to the computed features.
 */
class TimeBasedRollingWindow final : public RollingWindow {
  int _oldestIndex; /**< Index of the oldest event currently in the rolling window. */
  float _windowTime = 0; /**< Time window duration in seconds for event retention. */

 public:
  /**
   * @brief Constructor for TimeBasedRollingWindow.
   *
   * @param preprocessorId Identifier of the preprocessor this rolling window belongs to.
   * @param info Configuration information for the preprocessor.
   * @param windowTime Duration of the time window in seconds.
   */
  TimeBasedRollingWindow(int preprocessorId, const PreProcessorInfo& info, float windowTime);

  /**
   * @brief Adds a new event to the time-based rolling window.
   *
   * This method adds an event to the rolling window if it falls within the
   * current time window. Events outside the window are ignored.
   *
   * @param allEvents Vector containing all events in the table.
   * @param newEventIndex Index of the new event to be added.
   */
  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;

  /**
   * @brief Updates the time-based rolling window by removing expired events.
   *
   * This method removes events that have fallen outside the time window
   * and updates the aggregate columns accordingly. It also updates the
   * oldest event index to maintain the window boundaries.
   *
   * @param allEvents Vector containing all events in the table.
   */
  void update_window(const std::vector<TableEvent>& allEvents) override;
};
