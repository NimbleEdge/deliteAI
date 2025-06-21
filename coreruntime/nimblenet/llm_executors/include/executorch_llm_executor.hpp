/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "base_llm_executor.hpp"
#include "task.hpp"
#ifdef EXECUTORCH_EXECUTOR
#include "executorch/extension/llm/runner/runner.h"
#endif  // EXECUTORCH_EXECUTOR
#include "ne_fwd.hpp"
#include "rigtorp/SPSCQueue.h"

/**
 * @class ExecutorchLLMExecutor
 * @brief Executor class responsible for running inference using Executorch.
 *
 * This class extends BaseLLMExecutor to integrate with Executorch for local LLM inference.
 * It manages internal inference threads, token streams, and provides prompt execution
 * functionality.
 */
class ExecutorchLLMExecutor : public BaseLLMExecutor {
  using Queue = rigtorp::SPSCQueue<char>;

#ifdef EXECUTORCH_EXECUTOR
  /// Pointer to the Executorch LLM runner instance.
  std::unique_ptr<::executorch::extension::llm::IRunner> _runner;
#endif  // EXECUTORCH_EXECUTOR

  /// Stream to hold generated character output.
  std::shared_ptr<CharStream> _charStream;

  /// Internal single-producer single-consumer queue used for inference communication.
  std::shared_ptr<Queue> _internalQueue;

  /// Thread responsible for performing inference in the background.
  std::unique_ptr<std::thread> _inferenceThread;

  /// Flag to control the lifetime of the inference thread.
  std::atomic<bool> _runInferenceThread = true;

  /// Mutex used to guard shared state during prompt execution.
  std::mutex _mutex;

  /// Token indicating the end of a conversational turn.
  std::string _endOfTurnToken;

  /// Maximum number of tokens to generate during inference.
  int _maxTokensToGenerate;

  /// Temperature for sampling (higher = more random)
  float _temperature;

  /// Task object to associate with inference jobs and character stream filling.
  std::shared_ptr<Task> _task = nullptr;

  /// Keeps track of the position from which generate function should start
  /// It is modified by prefill_prompt function internally
  int64_t _start_pos = 0;

  /**
   * @brief Runs the inference engine with a given prompt.
   * @param prompt The input prompt to run inference on.
   */
  void run_inference(const std::string& prompt);

  /**
   * @brief Stops the running inference thread.
   */
  void stop_inference_thread();

  /**
   * @brief Marks the end of the output stream to signal completion of LLM output generation.
   */
  void mark_end_of_stream();

 public:
  /**
   * @brief Constructor for ExecutorchLLMExecutor.
   *
   * @param modelDirectoryPath Path to the model directory.
   * @param task Shared pointer to the task being executed.
   * @param commandCenter Pointer to the command center interface.
   * @param pteFileName Path to the PTE file.
   * @param endOfTurnToken Token used to signify end of a conversation turn.
   * @param maxTokensToGenerate Maximum number of tokens to generate in response.
   * @param temperature Sampling temperature for generation.
   * @param tokenizerFileName Tokenizer file name to be loaded for model e.g. tokenizer.bin for
                              llama, tokenizer.json for qwen
   */
  ExecutorchLLMExecutor(const std::string& modelDirectoryPath, std::shared_ptr<Task> task,
                        CommandCenter* commandCenter, const std::string& pteFileName,
                        const std::string& endOfTurnToken, int maxTokensToGenerate,
                        float temperature, const std::string& tokenizerFileName);

  /**
   * @brief Executes a prompt and returns a character stream with generated output.
   * @param prompt The prompt to be executed.
   * @return Shared pointer to a CharStream containing the response.
   */
  std::shared_ptr<CharStream> run_prompt(const std::string& prompt) override;

  /**
   * @brief Adds a prompt for streaming execution (used in conversational flows).
   * @param prompt The prompt to add.
   */
  void add_prompt(const std::string& prompt) override;

  /**
   * @brief Cancels the current running inference process.
   */
  void cancel() override;

  /**
   * @brief Clears the current context or conversation state.
   * @return Shared pointer to a NoneVariable object indicating cleared state.
   */
  std::shared_ptr<NoneVariable> clear_context() override;
};
