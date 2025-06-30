/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "time_based_rolling_window.hpp"

#include "time_manager.hpp"

TimeBasedRollingWindow::TimeBasedRollingWindow(int preprocessorId, const PreProcessorInfo& info,
                                               float windowTime)
    : RollingWindow(preprocessorId, info) {
  _windowTime = windowTime;
  _oldestIndex = -1;
}

void TimeBasedRollingWindow::add_event(const std::vector<TableEvent>& allEvents,
                                       int newEventIndex) {
  const auto& event = allEvents[newEventIndex];
  if (Time::get_time() - event.timestamp > _windowTime) {
    return;
  }
  if (_oldestIndex == -1) {
    _oldestIndex = newEventIndex;
  }
  for (auto aggregatedColumn :
       this->_groupWiseAggregatedColumnMap[event.groups[this->_preprocessorId]]) {
    aggregatedColumn->add_event(allEvents, newEventIndex);
  }
}

void TimeBasedRollingWindow::update_window(const std::vector<TableEvent>& allEvents) {
  if (_oldestIndex == -1) return;
  for (int i = _oldestIndex; i < allEvents.size(); i++) {
    if (Time::get_time() - allEvents[i].timestamp > _windowTime) {
      _oldestIndex++;
    }
  }
  for (auto it = this->_groupWiseAggregatedColumnMap.begin();
       it != this->_groupWiseAggregatedColumnMap.end(); it++) {
    for (auto aggregatedColumn : it->second) {
      aggregatedColumn->remove_events(allEvents, _oldestIndex);
    }
  }
}
