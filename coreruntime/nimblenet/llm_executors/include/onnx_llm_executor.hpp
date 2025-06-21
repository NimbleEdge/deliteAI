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

class ONNXLLMExecutor : public BaseLLMExecutor {
  using Queue = rigtorp::SPSCQueue<char>;
  OgaHandle _ogaHandle;

  std::unique_ptr<OgaModel> _model;
  std::unique_ptr<OgaTokenizer> _tokenizer;
  std::unique_ptr<OgaGenerator> _generator;
  std::unique_ptr<OgaGeneratorParams> _params;

  std::shared_ptr<CharStream> _charStream;
  std::shared_ptr<Queue> _internalQueue;
  std::unique_ptr<std::thread> _inferenceThread;
  std::atomic<bool> _runInferenceThread = true;
  std::mutex _mutex;

  // Store task so we can add charStreamFillJob to it when it's created
  std::weak_ptr<Task> _task;

 public:
  ONNXLLMExecutor(const std::string& configPath, std::shared_ptr<Task> task,
                  CommandCenter* commandCenter);
  std::shared_ptr<CharStream> run_prompt(const std::string& prompt) override;
  void add_prompt(const std::string& prompt) override;
  void cancel() override;
  std::shared_ptr<NoneVariable> clear_context() override;

 private:
  // This is the main function that runs on a separate thread
  void run_inference(std::string prompt);
  void stop_inference_thread();

  // PRE: Expects lock on mutex to be held by the caller
  void add_input_to_generator(const std::string& input);

  // In case of any error in ORT APIs, terminate the internal queue by appending '\0'
  void mark_end_of_stream();
};
