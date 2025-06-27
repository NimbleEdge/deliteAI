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

class RollingWindow {
  using T = double;

 public:
  int _preprocessorId;
  PreProcessorInfo _preprocessorInfo;
  std::map<std::string, std::vector<AggregateColumn*>> _groupWiseAggregatedColumnMap;
  bool create_aggregate_columns_for_group(const std::string& group,
                                          const std::vector<int>& columnIds,
                                          std::vector<double>& totalFeatureVector,
                                          int rollingWindowFeatureStartIndex);
  virtual void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) = 0;
  virtual void update_window(const std::vector<TableEvent>& allEvents) = 0;

  RollingWindow(int preprocessorId, const PreProcessorInfo& info) {
    _preprocessorInfo = info;
    _preprocessorId = preprocessorId;
  }

  virtual ~RollingWindow() {
    for (auto it : _groupWiseAggregatedColumnMap) {
      for (auto col : it.second) {
        delete col;
      }
    }
  }
};
