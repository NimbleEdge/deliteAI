/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "base_llm_executor.hpp"
#include "char_stream.hpp"
#include "ne_fwd.hpp"
#include "rigtorp/SPSCQueue.h"

using Queue = rigtorp::SPSCQueue<char>;

class GeminiNanoExecutor : public BaseLLMExecutor {
  static std::shared_ptr<Queue> _internalQueue;
  static std::mutex _mutex;
  // Store task so we can add charStreamFillJob to it when it's created
  std::shared_ptr<Task> _task = nullptr;
  // Stores context in memory if provided
  std::string _context;

 public:
  GeminiNanoExecutor(std::shared_ptr<Task> task, CommandCenter* commandCenter);
  std::shared_ptr<CharStream> run_prompt(const std::string& prompt) override;
  void add_prompt(const std::string& prompt) override;
  void cancel() override;
  std::shared_ptr<NoneVariable> clear_context() override;
  // Will be called from Kotlin
  static void push_to_queue(const std::string& text);
  static void mark_end_of_stream();
};
