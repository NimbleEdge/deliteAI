/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "base_llm_executor.hpp"

#include "command_center.hpp"
#include "nlohmann/json.hpp"

void from_json(const nlohmann::json& j, LLMExecutorConfig& config) {
  if (auto it = j.find("maxInputNumTokens"); it != j.end()) {
    it.value().get_to(config.maxInputNumTokens);
  }

  if (auto it = j.find("internalQueueSize"); it != j.end()) {
    it.value().get_to(config.internalQueueSize);
  }
};

void to_json(nlohmann::json& j, const LLMExecutorConfig& config) {
  j = nlohmann::json{{"maxInputNumTokens", config.maxInputNumTokens},
                     {"internalQueueSize", config.internalQueueSize}};
};

BaseLLMExecutor::BaseLLMExecutor(CommandCenter* commandCenter)
    : _executorConfig(commandCenter->get_llm_executor_config()) {}

int BaseLLMExecutor::max_input_num_tokens() const noexcept {
  return _executorConfig.maxInputNumTokens;
}
