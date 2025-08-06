/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "base_llm_executor.hpp"
#include "task.hpp"
#ifdef EXECUTORCH_EXECUTOR
#include "executorch/extension/llm/runner/text_llm_runner.h"
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
  std::unique_ptr<::executorch::extension::llm::IRunner>
      _runner; /**< Pointer to the Executorch LLM runner instance. */
#endif         // EXECUTORCH_EXECUTOR

  std::shared_ptr<CharStream> _charStream; /**< Stream to hold generated character output. */

  std::shared_ptr<Queue> _internalQueue; /**< Internal single-producer single-consumer queue used
                                            for inference communication. */

  std::unique_ptr<std::thread>
      _inferenceThread; /**< Thread responsible for performing inference in the background. */

  std::atomic<bool> _runInferenceThread =
      true; /**< Flag to control the lifetime of the inference thread. */

  std::mutex _mutex; /**< Mutex used to guard shared state during prompt execution. */

  std::string _endOfTurnToken; /**< Token indicating the end of a conversational turn. */

  int _maxTokensToGenerate; /**< Maximum number of tokens to generate during inference. */

  float _temperature; /**< Temperature for sampling (higher = more random) */

  std::shared_ptr<Task> _task =
      nullptr; /**< Task object to associate with inference jobs and character stream filling. */

  int64_t _start_pos = 0; /**< Keeps track of the position from which generate function should
                             start. It is modified by prefill_prompt function internally*/

  /**
   * @brief Runs the inference engine with a given prompt.
   *
   * @param prompt The input prompt to run inference on.
   */
  void run_inference(const std::string& prompt);

  /**
   * @brief Stops the running inference thread.
   */
  void stop_inference_thread();

  /**
   * @brief Marks the end of the output stream to signal completion of LLM output generation or an
   * error from the executor.
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
   *
   * @param prompt The prompt to be executed.
   * @return Shared pointer to a CharStream containing the response.
   */
  std::shared_ptr<CharStream> run_prompt(const std::string& prompt) override;

  /**
   * @brief Add a prompt to the inference pipeline.
   *
   * @param prompt Input prompt string.
   */
  void add_prompt(const std::string& prompt) override;

  /**
   * @brief Cancel any ongoing inference operation.
   */
  void cancel() override;

  /**
   * @brief Clears model context i.e. reset _start_pos.
   *
   * @return Shared pointer to a NoneVariable indicating reset completion.
   */
  std::shared_ptr<NoneVariable> clear_context() override;
};
