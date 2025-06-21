/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <string>

#include "char_stream.hpp"
#include "ne_fwd.hpp"
#include "nlohmann/json_fwd.hpp"

class NoneVariable;

struct LLMExecutorConfig {
  int maxInputNumTokens = 10'000;
  int internalQueueSize = 500;
};

void from_json(const nlohmann::json& j, LLMExecutorConfig& config);
void to_json(nlohmann::json& j, const LLMExecutorConfig& config);

class BaseLLMExecutor {
 protected:
  LLMExecutorConfig _executorConfig;

 public:
  BaseLLMExecutor(CommandCenter* commandCenter);
  virtual ~BaseLLMExecutor() = default;

  [[nodiscard]] virtual std::shared_ptr<CharStream> run_prompt(const std::string& prompt) = 0;
  virtual void add_prompt(const std::string& prompt) = 0;
  virtual void cancel() = 0;

  int max_input_num_tokens() const noexcept;

  virtual std::shared_ptr<NoneVariable> clear_context() = 0;
};
