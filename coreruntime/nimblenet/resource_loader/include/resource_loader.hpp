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

class ResourceLoader {
  CommandCenter* _commandCenter;
  bool _isCurrentState;
  std::map<AssetId, std::shared_ptr<Task>> _taskMap;

 public:
  ResourceLoader(CommandCenter* commandCenter_, bool isCurrentState);

  OpReturnType load_asset(std::shared_ptr<Asset> asset, const std::vector<OpReturnType>& arguments);
  bool load_task(std::shared_ptr<Asset> task);

 private:
  OpReturnType load_model(std::shared_ptr<Asset> asset);
#ifdef GENAI
  OpReturnType load_document(std::shared_ptr<Asset> asset);
  OpReturnType load_retriever(std::shared_ptr<Asset> asset,
                              const std::vector<OpReturnType>& arguments);
  OpReturnType load_llm(std::shared_ptr<Asset> asset);
#endif  // GENAI
};
