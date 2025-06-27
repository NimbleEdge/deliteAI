/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rolling_window.hpp"

class TimeBasedRollingWindow final : public RollingWindow {
  int _oldestIndex;
  float _windowTime = 0;

 public:
  TimeBasedRollingWindow(int preprocessorId, const PreProcessorInfo& info, float windowTime);
  void add_event(const std::vector<TableEvent>& allEvents, int newEventIndex) override;
  void update_window(const std::vector<TableEvent>& allEvents) override;
};
