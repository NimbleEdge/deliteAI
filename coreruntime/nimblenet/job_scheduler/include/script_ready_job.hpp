/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "job.hpp"
class CommandCenter;

class ScriptReadyJob : public Job<void> {
  CommandCenter* _commandCenter;

  void delete_new_command_center() noexcept;

 public:
  ScriptReadyJob(CommandCenter* commandCenter_) : Job("ScriptReadyJob") {
    _commandCenter = commandCenter_;
  };

  std::future<void> init();

  Job::Status process() override;
};
