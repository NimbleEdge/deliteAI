/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "command_center.hpp"
#include "data_variable.hpp"

class CommandCenter;

class RetrieverDataVariable final : public DataVariable {
  CommandCenter* _commandCenter;
  OpReturnType _embeddingModel;
  OpReturnType _embeddingStoreModel;
  OpReturnType _documentStore;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  nlohmann::json to_json() const override { return "[Retriever]"; }

 public:
  RetrieverDataVariable(CommandCenter* commandCenter_, const std::vector<OpReturnType>& arguments);

  std::string print() override { return fallback_print(); }

 private:
  OpReturnType topk(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;
};
