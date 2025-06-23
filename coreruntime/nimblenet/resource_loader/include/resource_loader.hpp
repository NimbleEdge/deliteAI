/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <memory>

#include "asset_manager.hpp"
#include "ne_fwd.hpp"
#include "task.hpp"

/**
 * @class ResourceLoader
 * @brief Responsible for loading various types of resources such as assets.
 *
 * This class interacts with the CommandCenter and AssetManager to initialize and load resources
 * required for task execution.
 */
class ResourceLoader {
  CommandCenter* _commandCenter;
  bool _isCurrentState; /**< Flag indicating whether this loader represents the active state. */
  std::map<AssetId, std::shared_ptr<Task>> _taskMap; /**<  Tracks loaded tasks by AssetId. */

 public:
  /**
   * @brief Constructs a ResourceLoader.
   *
   * @param commandCenter_ Pointer to the CommandCenter.
   * @param isCurrentState
   */
  ResourceLoader(CommandCenter* commandCenter_, bool isCurrentState);

  /**
   * @brief Loads a generic asset, delegating to the appropriate handler based on asset type.
   *
   * @param asset The asset to load.
   * @param arguments Additional assets on which the asset loading depends.
   * @return OpReturnType The datavariable with the loaded asset.
   */
  OpReturnType load_asset(std::shared_ptr<Asset> asset, const std::vector<OpReturnType>& arguments);

  /**
   * @brief Loads a delitepy script.
   *
   * @param task The delitepy script asset to load.
   * @return True if the task was successfully loaded, false otherwise.
   */
  bool load_task(std::shared_ptr<Asset> task);

 private:
  /**
   * @brief Handles loading of model assets.
   *
   * @param asset The model asset.
   * @return Result of the load operation.
   */
  OpReturnType load_model(std::shared_ptr<Asset> asset);

#ifdef GENAI
  /**
   * @brief Handles loading of document-type assets,
   *
   * @param asset The document asset.
   * @return Datavariable with the json content.
   */
  OpReturnType load_document(std::shared_ptr<Asset> asset);

  /**
   * @brief Handles loading of RAG.
   *
   * @param asset The retriever asset.
   * @param arguments Additional assets to be loaded for the RAG.
   * @return Result of the load operation.
   */
  OpReturnType load_retriever(std::shared_ptr<Asset> asset,
                              const std::vector<OpReturnType>& arguments);

  /**
   * @brief Handles loading of LLM (large language model) assets.
   *
   * @param asset The LLM asset.
   * @return Result of the load operation.
   */
  OpReturnType load_llm(std::shared_ptr<Asset> asset);
#endif  // GENAI
};
