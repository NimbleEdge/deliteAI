/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "max_column.hpp"

#include "data_variable.hpp"
#include "util.hpp"

using namespace std;

void MaxColumn::add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) {
  const auto& event = allEvents[newEventIndex];
  const auto eventGroup = event.groups[this->_preprocessorId];
  if (eventGroup != this->_group) {
    LOG_TO_ERROR("MaxColumn: add_event event.group=%s not same as column.group=%s",
                 eventGroup.c_str(), this->_group.c_str());
    return;
  }
  this->_totalCount++;
  if (_oldestIndex == -1) {
    _oldestIndex = newEventIndex;
    *this->_storeValue = event.row[this->_columnId]->template get<T>();
    return;
  }

  *this->_storeValue = max(*this->_storeValue, event.row[this->_columnId]->template get<T>());
}

void MaxColumn::remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) {
  if (_oldestIndex == -1) return;
  bool isMaxChanged = false;
  for (int i = _oldestIndex; i < allEvents.size(); i++) {
    if (!isMaxChanged && (i >= oldestValidIndex)) {
      break;
    }
    const auto eventGroup = allEvents[i].groups[this->_preprocessorId];
    if (eventGroup == this->_group) {
      T val = allEvents[i].row[this->_columnId]->template get<T>();
      if (isMaxChanged) {
        if (this->_totalCount == 0) {
          *this->_storeValue = val;
          _oldestIndex = i;
        } else
          *this->_storeValue = max(*this->_storeValue, val);
        this->_totalCount++;
      } else if (val == *this->_storeValue) {
        isMaxChanged = true;
        this->_totalCount = 0;
        *this->_storeValue = this->_defaultValue;
        i = oldestValidIndex - 1;
      } else {
        this->_totalCount--;
      }
    }
  }
  if (this->_totalCount == 0) {
    *this->_storeValue = this->_defaultValue;
    _oldestIndex = -1;
    return;
  }
  _oldestIndex = oldestValidIndex;
}
