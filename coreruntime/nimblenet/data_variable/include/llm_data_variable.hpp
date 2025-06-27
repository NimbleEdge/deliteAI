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

class LLMDataVariable final : public DataVariable {
  std::unique_ptr<BaseLLMExecutor> _llmExecutor;

 private:
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  nlohmann::json to_json() const override { return "[LLM]"; }

  std::string print() override { return fallback_print(); }

 public:
  LLMDataVariable(const std::shared_ptr<Asset> llmAsset, CommandCenter* commandCenter);
  static std::shared_ptr<FutureDataVariable> load_async(
      const std::map<std::string, OpReturnType>& llmConfig, CommandCenter* commandCenter);

 private:
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  std::shared_ptr<CharStream> prompt(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType cancel_generation(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType add_context(const std::vector<OpReturnType>& arguments, CallStack& stack);
};
