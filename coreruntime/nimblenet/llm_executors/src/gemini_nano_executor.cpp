/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gemini_nano_executor.hpp"

#include "native_interface.hpp"
#include "task.hpp"

std::shared_ptr<Queue> GeminiNanoExecutor::_internalQueue;
std::mutex GeminiNanoExecutor::_mutex;

GeminiNanoExecutor::GeminiNanoExecutor(std::shared_ptr<Task> task, CommandCenter* commandCenter)
    : BaseLLMExecutor(commandCenter) {
  if (!task) {
    THROW("%s", "Task pointer not set");
  }
  _task = task;
}

std::shared_ptr<CharStream> GeminiNanoExecutor::run_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};

  // Cancel any previous prompt
  if (_internalQueue) {
    nativeinterface::cancel_os_llm_query();
  }

  std::shared_ptr<CharStream> charStream = CharStream::construct();
  _internalQueue = std::make_shared<Queue>(_executorConfig.internalQueueSize);

  auto job = std::make_shared<FillCharStreamJob>((decltype(charStream)::weak_type){charStream},
                                                 _internalQueue);
  _task->add_stream_push_job(job);

  // Add context to current prompt
  std::string fullPrompt = _context + prompt;
  nativeinterface::prompt_os_llm(std::move(fullPrompt));

  return charStream;
}

void GeminiNanoExecutor::add_prompt(const std::string& prompt) {
  std::lock_guard<std::mutex> lock{_mutex};

  _context += prompt;
}

void GeminiNanoExecutor::push_to_queue(const std::string& text) {
  std::lock_guard<std::mutex> lock{_mutex};

  if (_internalQueue) {
    for (char c : text) {
      _internalQueue->push(c);
    }
  }
}

void GeminiNanoExecutor::mark_end_of_stream() {
  std::lock_guard<std::mutex> lock{_mutex};

  if (_internalQueue) {
    _internalQueue->push('\0');
    _internalQueue = nullptr;
  }
}

void GeminiNanoExecutor::cancel() {
  std::lock_guard<std::mutex> lock{_mutex};

  nativeinterface::cancel_os_llm_query();
  _internalQueue = nullptr;
}

std::shared_ptr<NoneVariable> GeminiNanoExecutor::clear_context() {
  std::lock_guard<std::mutex> lock{_mutex};

  _context.clear();
  return std::make_shared<NoneVariable>();
}
