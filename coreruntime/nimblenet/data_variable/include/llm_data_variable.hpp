/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include "base_llm_executor.hpp"
#include "char_stream.hpp"
#include "core_sdk_structs.hpp"
#include "data_variable.hpp"

/**
 * @brief DataVariable implementation for Large Language Model (LLM) operations
 *
 * LLMDataVariable provides a unified interface for interacting with various LLM backends
 * including Gemini Nano, ONNX, and Executorch models. It encapsulates the complexity
 * of model loading, prompt execution, and context management behind a simple DataVariable
 * interface that can be used within the NimbleNet runtime.
 *
 * The class supports both OS-provided models (like Gemini) and custom models loaded
 * from deployment assets. It handles model-specific configuration through metadata
 * and provides streaming text generation capabilities.
 */
class LLMDataVariable final : public DataVariable {
  std::unique_ptr<BaseLLMExecutor> _llmExecutor; /**< Backend LLM executor instance */

 private:
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  nlohmann::json to_json() const override { return "[LLM]"; }

  std::string print() override { return fallback_print(); }

 public:
  /**
   * @brief Constructs an LLMDataVariable with the specified LLM asset
   * @param llmAsset The asset containing LLM model information and metadata
   * @param commandCenter Command center for task and deployment management
   */
  LLMDataVariable(const std::shared_ptr<Asset> llmAsset, CommandCenter* commandCenter);

  /**
   * @brief Asynchronously loads an LLM model based on configuration
   * @param llmConfig Configuration map containing model name, provider, and metadata
   * @param commandCenter Command center for deployment and asset management
   * @return FutureDataVariable that resolves to the loaded LLM instance
   */
  static std::shared_ptr<FutureDataVariable> load_async(
      const std::map<std::string, OpReturnType>& llmConfig, CommandCenter* commandCenter);

 private:
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  /**
   * @brief Executes a prompt on the LLM and returns a streaming response
   * @param arguments Vector containing the prompt string as the first argument
   * @param stack Current call stack for execution context
   * @return CharStream for streaming the LLM response
   */
  std::shared_ptr<CharStream> prompt(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Cancels ongoing LLM generation
   * @param arguments Empty argument vector (no parameters required)
   * @param stack Current call stack for execution context
   * @return NoneVariable indicating successful cancellation
   */
  OpReturnType cancel_generation(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Adds context to the LLM's conversation history
   * @param arguments Vector containing the context string as the first argument
   * @param stack Current call stack for execution context
   * @return NoneVariable indicating successful context addition
   */
  OpReturnType add_context(const std::vector<OpReturnType>& arguments, CallStack& stack);
};
