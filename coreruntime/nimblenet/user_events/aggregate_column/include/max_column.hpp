/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "aggregate_column.hpp"

class MaxColumn final : public AggregateColumn {
  using T = double;
  int _oldestIndex = -1;

 public:
  MaxColumn(int preprocessorId, int columnId, const std::string& group, T* storePtr)
      : AggregateColumn(preprocessorId, columnId, group, storePtr) {};

  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;
  void remove_events(const std::vector<TableEvent>& allEvents, int oldestValidIndex) override;
};
