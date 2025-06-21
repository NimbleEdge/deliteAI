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

/**
 * @brief Configuration struct for LLMExecutor behavior.
 *
 * Contains runtime options that control resource limits and internal buffering behavior
 * for an LLM (Large Language Model) executor instance.
 */
struct LLMExecutorConfig {
  int maxInputNumTokens = 10'000; /**< Maximum number of input tokens accepted per prompt. */
  int internalQueueSize = 500;    /**< Size of internal queue containing the LLM output tokens. */
};

/**
 * @brief Deserialize LLMExecutorConfig from JSON.
 *
 * @param j The JSON object to read from.
 * @param config The config object to populate.
 */
void from_json(const nlohmann::json& j, LLMExecutorConfig& config);

/**
 * @brief Serialize LLMExecutorConfig into JSON.
 *
 * @param j The JSON object to write to.
 * @param config The config object to convert.
 */
void to_json(nlohmann::json& j, const LLMExecutorConfig& config);

/**
 * @brief Abstract base class for various LLM backends.
 *
 * Provides a common interface for LLM loading, model context management, model prompting and
 * execution stop.
 */
class BaseLLMExecutor {
 protected:
  LLMExecutorConfig _executorConfig; /**< Runtime configuration for LLM execution. */

 public:
  /**
   * @brief Construct a new BaseLLMExecutor instance.
   *
   * @param commandCenter
   */
  BaseLLMExecutor(CommandCenter* commandCenter);

  virtual ~BaseLLMExecutor() = default;

  /**
   * @brief Starts LLM inference on a separate thread. Output is continuously pushed to
   * _internalQueue.
   *
   * @param prompt The input prompt string.
   * @return A shared pointer to a CharStream representing the model's response stream.
   */
  [[nodiscard]] virtual std::shared_ptr<CharStream> run_prompt(const std::string& prompt) = 0;

  /**
   * @brief Method to add historical context to LLM.
   *   *
   * @param prompt The historical context.
   */
  virtual void add_prompt(const std::string& prompt) = 0;

  /**
   * @brief Cancel ongoing LLM execution.
   *
   * Used to stop token generation if the user initiates a cancellation.
   */
  virtual void cancel() = 0;

  /**
   * @brief Get the configured max input token limit for this executor.
   *
   * @return Maximum number of tokens allowed in a prompt.
   */
  int max_input_num_tokens() const noexcept;

  /**
   * @brief Clear the internal context i.e. conversation history.
   *
   * @return A shared pointer to a NoneVariable indicating completion of context reset.
   */
  virtual std::shared_ptr<NoneVariable> clear_context() = 0;
};
