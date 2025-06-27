/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "count_column.hpp"

#include "util.hpp"

using namespace std;

void CountColumn::add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) {
  const auto& event = allEvents[newEventIndex];
  const auto eventGroup = event.groups[this->_preprocessorId];
  if (eventGroup != this->_group) {
    LOG_TO_ERROR("CountColumn: add_event event.group=%s not same as column.group=%s",
                 eventGroup.c_str(), this->_group.c_str());
    return;
  }
  this->_totalCount++;
  if (_oldestIndex == -1) {
    _oldestIndex = newEventIndex;
    *this->_storeValue = 1;
    return;
  }
  *this->_storeValue = (*this->_storeValue) + 1;
}

void CountColumn::remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) {
  if (_oldestIndex == -1) return;
  for (int i = _oldestIndex; i < oldestValidIndex; i++) {
    const auto eventGroup = allEvents[i].groups[this->_preprocessorId];
    if (eventGroup == this->_group) {
      *this->_storeValue = (*this->_storeValue) - 1;
      this->_totalCount--;
    }
  }
  if (this->_totalCount == 0) {
    *this->_storeValue = this->_defaultValue;
    _oldestIndex = -1;
    return;
  }
  _oldestIndex = oldestValidIndex;
}
