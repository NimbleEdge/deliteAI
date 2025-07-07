/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "executorch_llm_executor.hpp"

#include "executorch/runtime/core/error.h"
#include "executorch/runtime/core/result.h"

ExecutorchLLMExecutor::ExecutorchLLMExecutor(
    const std::string& modelDirectoryPath, std::shared_ptr<Task> task, CommandCenter* commandCenter,
    const std::string& pteFileName, const std::string& endOfTurnToken, int maxTokensToGenerate,
    const float temperature, const std::string& tokenizerFileName)
    : BaseLLMExecutor(commandCenter) {
  if (!task) {
    THROW("%s", "Task pointer not set");
  }
  _task = task;
  _endOfTurnToken = endOfTurnToken;
  _maxTokensToGenerate = maxTokensToGenerate;
  _temperature = temperature;

  try {
    _runner = example::Runner::create(modelDirectoryPath + "/" + pteFileName + ".pte",
                                      modelDirectoryPath + "/" + tokenizerFileName, std::nullopt,
                                      temperature);
    _runner->load();
    if (!_runner->is_loaded()) {
      THROW("Could not load LLM using executorch");
    }
  } catch (const std::exception& e) {
#ifndef SIMULATION_MODE
    util::delete_folder_recursively(modelDirectoryPath);
#endif  // SIMULATION_MODE
    THROW("Could not load llm: %s with error: %s using executorch.", pteFileName.c_str(), e.what());
  }
}

std::shared_ptr<CharStream> ExecutorchLLMExecutor::run_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};

  stop_inference_thread();
  // Creating these variables again to drop ones used by previous inference
  _charStream = CharStream::construct();
  _internalQueue = std::make_shared<Queue>(_executorConfig.internalQueueSize);

  auto job = std::make_shared<FillCharStreamJob>((decltype(_charStream)::weak_type){_charStream},
                                                 _internalQueue);
  _task->add_stream_push_job(job);

  // NOTE: Initialize all variables that may be used by the inference thread before starting
  // the thread. This will ensure those variables are only touched by the thread
  _inferenceThread =
      std::make_unique<std::thread>(&ExecutorchLLMExecutor::run_inference, this, prompt);
  return _charStream;
}

void ExecutorchLLMExecutor::run_inference(const std::string& prompt) {
  std::shared_ptr<int> numOfTokens = std::make_shared<int>(0);
  std::function<void(const std::string&)> token_callback = [this,
                                                            numOfTokens](const std::string& piece) {
    if (*numOfTokens >= _maxTokensToGenerate) {
      mark_end_of_stream();
      return;
    }
    if (piece == _endOfTurnToken) {
      mark_end_of_stream();
      return;
    }
    for (char c : piece) {
      if (c != '\0') {
        _internalQueue->push(c);
      } else {
        mark_end_of_stream();
        return;
      }
    }
    (*numOfTokens)++;
  };
  try {
    executorch::extension::llm::GenerationConfig config{
        .echo = false,
        .seq_len = _executorConfig.maxInputNumTokens,
        .temperature = _temperature,
    };
    auto status = _runner->generate_from_pos(prompt, config, _start_pos, token_callback, {});
    if (status != ::executorch::runtime::Error::Ok) {
      mark_end_of_stream();
      LOG_TO_CLIENT_ERROR("Error while running inference on LLM using executorch.");
    }
  } catch (const std::exception& e) {
    mark_end_of_stream();
    LOG_TO_CLIENT_ERROR("Error: %s while running inference on LLM using executorch.", e.what());
  }
}

void ExecutorchLLMExecutor::add_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();
  try {
    auto status = _runner->prefill_prompt(prompt, _start_pos, 0, 0);
    if (!status.ok()) {
      LOG_TO_CLIENT_ERROR("Error while setting context in LLM using executorch.");
    }
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Error: %s while setting context in LLM using executorch.", e.what());
  }
}

void ExecutorchLLMExecutor::cancel() {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();
}

std::shared_ptr<NoneVariable> ExecutorchLLMExecutor::clear_context() {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();
  _start_pos = 0;
  return std::make_shared<NoneVariable>();
}

void ExecutorchLLMExecutor::stop_inference_thread() {
  if (!_inferenceThread) return;
  _runner->stop();
  _runInferenceThread.store(false);
  _inferenceThread->join();
  _inferenceThread.reset();
  _runInferenceThread.store(true);

  _charStream = nullptr;
  _internalQueue = nullptr;
}

void ExecutorchLLMExecutor::mark_end_of_stream() { _internalQueue->push('\0'); }
