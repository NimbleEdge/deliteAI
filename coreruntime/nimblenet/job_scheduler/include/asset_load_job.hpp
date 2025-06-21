/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "asset_manager.hpp"
#include "future.hpp"
#include "job.hpp"
#include "ne_fwd.hpp"

/**
 * @brief Job responsible for loading an asset into memory.
 *
 * This job is responsible for resolving the asset (from a location), depending
 * on previous jobs such as a download. It returns an `OpReturnType`, representing
 * a handle to loaded data.
 */
class AssetLoadJob : public Job<OpReturnType> {
  CommandCenter* _commandCenter; /**< Pointer to the CommandCenter coordinating this job. */

  /* In case this job has dependencies (other jobs returning OpReturnType), track their output
   * here.
   */
  std::vector<OpReturnType> _arguments;

  /* In case there are no OpReturnType dependencies, we fall back to waiting for a Location from a
   * future.
   */
  ne::Future<Location> _locationFuture;

  std::shared_ptr<Asset> _asset; /**< The asset to be loaded. */

 public:
  /**
   * @brief Construct a new AssetLoadJob instance.
   *
   * @param asset            Shared pointer to the asset to load.
   * @param commandCenter_   Pointer to the controlling CommandCenter.
   */
  AssetLoadJob(std::shared_ptr<Asset> asset, CommandCenter* commandCenter_);

  /**
   * @brief Initializes the job and returns a future to await the loaded data.
   *
   * @return Future wrapping the output OpReturnType.
   */
  [[nodiscard]] std::shared_ptr<FutureDataVariable> init();

  /**
   * @brief Executes the asset loading logic.
   *
   * @return Job status indicating success or retry state.
   */
  Job::Status process() override;

  /**
   * @brief Static helper to synchronously create AssetLoadJob and initialize it.
   *
   * @param asset           Asset to be loaded.
   * @param commandCenter   Pointer to the CommandCenter.
   * @return                The loaded data in OpReturnType format.
   */
  static OpReturnType fetch(std::shared_ptr<Asset> asset, CommandCenter* commandCenter);
};
