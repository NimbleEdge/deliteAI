/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "base_llm_executor.hpp"
#include "char_stream.hpp"
#include "ne_fwd.hpp"
#include "ort_genai.h"
#include "ort_genai_c.h"
#include "rigtorp/SPSCQueue.h"

class Task;

/**
 * @class ONNXLLMExecutor
 * @brief Executor class responsible for running inference using ONNX-GenAI executor.
 *
 * Inherits from BaseLLMExecutor. It wraps and manages the ONNX GenAI model, tokenizer,
 * generator, and associated inference thread. Responsible for prompt submission,
 * token streaming, cancellation, and context reset.
 */
class ONNXLLMExecutor : public BaseLLMExecutor {
  using Queue = rigtorp::SPSCQueue<char>;

  OgaHandle _ogaHandle; /**< Handle to ONNX GenAI runtime environment. */

  // Core GenAI components for local inference
  std::unique_ptr<OgaModel> _model;         /**< Loaded ONNX LLM model. */
  std::unique_ptr<OgaTokenizer> _tokenizer; /**< Tokenizer for converting text to tokens. */
  std::unique_ptr<OgaGenerator> _generator; /**< Generator object which stores the context of user
                                               prompts and assistant response. */
  std::unique_ptr<OgaGeneratorParams> _params; /**< Parameters for text generation. */

  std::shared_ptr<CharStream> _charStream; /**< Stream to hold generated character output. */
  std::shared_ptr<Queue> _internalQueue;   /**< Internal single-producer single-consumer queue used
                                                for inference communication. */
  std::unique_ptr<std::thread>
      _inferenceThread; /**< Thread responsible for performing inference in the background. */
  std::atomic<bool> _runInferenceThread = true; /**< Flag to stop inference thread. */
  std::mutex _mutex;                            /**< Protects generator state and prompt I/O. */

  std::weak_ptr<Task>
      _task; /**< Store task so we can add charStreamFillJob to it when it's created */

 public:
  /**
   * @brief Constructor for ONNXLLMExecutor.
   *
   * @param configPath Path to model directory which has the genai_config.json, .onnx and tokenizer
   * files.
   * @param task Task used to orchestrate llm generation with delitepy script.
   * @param commandCenter
   */
  ONNXLLMExecutor(const std::string& configPath, std::shared_ptr<Task> task,
                  CommandCenter* commandCenter);

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
   * @brief Clears model context (i.e., resets generator state).
   *
   * @return Shared pointer to a NoneVariable indicating reset completion.
   */
  std::shared_ptr<NoneVariable> clear_context() override;

 private:
  /**
   * @brief Inference loop run in a background thread.
   *
   * Responsible for token generation and pushing tokens to the CharStream.
   *
   * @param prompt Prompt to feed to the generator.
   */
  void run_inference(std::string prompt);

  /**
   * @brief Stops the inference thread and joins it safely.
   */
  void stop_inference_thread();

  /**
   * @brief Adds input string to the generator.
   *
   * @param input The input prompt.
   *
   * @pre Caller must hold the `_mutex` lock.
   */
  void add_input_to_generator(const std::string& input);

  /**
   * @brief Marks the end of stream in case of error or an error from the executor.
   *
   * Pushes a null character `'\0'` to the stream queue.
   */
  void mark_end_of_stream();
};
