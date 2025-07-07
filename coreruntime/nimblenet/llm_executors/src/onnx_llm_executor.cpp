/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "onnx_llm_executor.hpp"

#include <memory>
#include <mutex>
#include <thread>

#include "base_llm_executor.hpp"
#include "char_stream.hpp"
#include "command_center.hpp"
#include "ort_genai.h"
#include "ort_genai_c.h"
#include "task.hpp"

ONNXLLMExecutor::ONNXLLMExecutor(const std::string& configPath, std::shared_ptr<Task> task,
                                 CommandCenter* commandCenter)
    : BaseLLMExecutor(commandCenter) {
  if (!task) {
    THROW("%s", "Task pointer not set");
  }
  _task = task;
  try {
    _model = OgaModel::Create(configPath.c_str());
    _tokenizer = OgaTokenizer::Create(*_model);
    _params = OgaGeneratorParams::Create(*_model);
    // TODO: Set this via ep config probably
    _params->SetSearchOption("max_length", _executorConfig.maxInputNumTokens);
    _generator = OgaGenerator::Create(*_model, *_params);
  } catch (const std::exception& e) {
// Don't delete LLM for simulator mode, as it is a symlink, which will delete the original
#ifndef SIMULATION_MODE
    util::delete_folder_recursively(configPath);
#endif
    THROW("Could not load LLM with error: %s using onnxruntime-genai.", e.what());
  }
}

std::shared_ptr<CharStream> ONNXLLMExecutor::run_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();

  // Creating these variables again to drop ones used by previous inference
  _charStream = CharStream::construct();
  _internalQueue = std::make_shared<Queue>(_executorConfig.internalQueueSize);

  auto job = std::make_shared<FillCharStreamJob>((decltype(_charStream)::weak_type){_charStream},
                                                 _internalQueue);

  auto task = _task.lock();
  if (!task) {
    THROW("Task destroyed before running prompt.");
  }
  task->add_stream_push_job(job);

  // NOTE: Initialize all variables that may be used by the inference thread before starting
  // the thread. This will ensure those variables are only touched by the thread
  _inferenceThread = std::make_unique<std::thread>(&ONNXLLMExecutor::run_inference, this, prompt);
  return _charStream;
}

void ONNXLLMExecutor::add_input_to_generator(const std::string& input) {
  auto sequences = OgaSequences::Create();
  _tokenizer->Encode(input.c_str(), *sequences);
  _generator->AppendTokenSequences(*sequences);
}

void ONNXLLMExecutor::add_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();
  try {
    add_input_to_generator(prompt);
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Could not add input to generator with error: %s using onnxruntime-genai",
                        e.what());
  }
}

void ONNXLLMExecutor::run_inference(std::string prompt) {
  try {
    add_input_to_generator(prompt);

    auto tokenizerOutStream = OgaTokenizerStream::Create(*_tokenizer);
    while (_runInferenceThread.load() && !_generator->IsDone()) {
      _generator->GenerateNextToken();

      const auto num_tokens = _generator->GetSequenceCount(0);
      const auto new_token = _generator->GetSequenceData(0)[num_tokens - 1];

      const char* outStr = tokenizerOutStream->Decode(new_token);
      // LOG_TO_DEBUG("got from tokenizer: %s", outStr);
      while (*outStr != '\0') {
        _internalQueue->push(*outStr);
        outStr++;
      }
    }
    mark_end_of_stream();
  } catch (const std::exception& e) {
    mark_end_of_stream();
    LOG_TO_CLIENT_ERROR("Error: %s while running inference on LLM using onnxruntime-genai.",
                        e.what());
  }
}

void ONNXLLMExecutor::stop_inference_thread() {
  if (!_inferenceThread) return;

  _runInferenceThread.store(false);
  _inferenceThread->join();
  _inferenceThread.reset();
  _runInferenceThread.store(true);

  _charStream = nullptr;
  _internalQueue = nullptr;
}

void ONNXLLMExecutor::cancel() {
  std::lock_guard<std::mutex> lock{_mutex};

  stop_inference_thread();
}

std::shared_ptr<NoneVariable> ONNXLLMExecutor::clear_context() {
  std::lock_guard<std::mutex> lock{_mutex};
  stop_inference_thread();
  try {
    _generator->SetRuntimeOption("terminate_session", "1");
    _generator.reset();
    _generator = OgaGenerator::Create(*_model, *_params);
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Error: %s while clearing context for LLM using onnxruntime-genai.",
                        e.what());
  }

  return std::make_shared<NoneVariable>();
}

void ONNXLLMExecutor::mark_end_of_stream() { _internalQueue->push('\0'); }
