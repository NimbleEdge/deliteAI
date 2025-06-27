/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include "user_events_struct.hpp"

class AggregateColumn {
  using T = double;

 public:
  T* _storeValue = nullptr;
  int _columnId;
  std::string _group;
  int _preprocessorId;
  T _defaultValue;
  int _totalCount = 0;

  AggregateColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr) {
    _preprocessorId = preprocessorId;
    _columnId = columnId;
    _group = group;
    _storeValue = storePtr;
    _defaultValue = *storePtr;
  }

  virtual void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) = 0;
  virtual void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) = 0;
  virtual ~AggregateColumn() = default;
};

#include "average_column.hpp"
#include "count_column.hpp"
#include "max_column.hpp"
#include "min_column.hpp"
#include "sum_column.hpp"

template <typename T>
T GetAs(const std::string& s) {
  std::stringstream ss{s};
  T t;
  if (!(ss >> t)) LOG_TO_ERROR("%s cannot be converted to %s", s.c_str(), typeid(T).name());
  return t;
}
