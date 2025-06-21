/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "job.hpp"
class CommandCenter;

/**
 * @brief A job that signals the system that the delitepy script is ready to run.
 *
 * This job performs finalization steps after the script has been downloaded, loaded and all the
 * models/llm used in the script are also loaded. It does not return a value (`Job<void>`).
 */
class ScriptReadyJob : public Job<void> {
  CommandCenter* _commandCenter;

  /**
   * @brief This method is used in case the script is downloaded in the background thread i.e. the
   * initiazation of SDK was a success based on the assets present on disk, but there was a new
   * version in cloud. Then a new commandCenter is created and all the assets downloaded and
   * loaded. Once that is done delete the commandCenter.
   */
  void delete_new_command_center() noexcept;

 public:
  ScriptReadyJob(CommandCenter* commandCenter_) : Job("ScriptReadyJob") {
    _commandCenter = commandCenter_;
  };

  std::future<void> init();

  Job::Status process() override;
};
