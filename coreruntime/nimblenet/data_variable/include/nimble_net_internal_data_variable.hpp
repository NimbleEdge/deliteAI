/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "variable_scope.hpp"

#ifdef GENAI
#include "retriever.hpp"
#include "single_variable.hpp"
#include "stream_data_variable.hpp"
#endif  // GENAI

class CommandCenter;

class NimbleNetInternalDataVariable final : public DataVariable {
  CommandCenter* _commandCenter = nullptr;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_INTERNAL; }

  OpReturnType get_current_time() {
    return OpReturnType(new SingleVariable<int64_t>(Time::get_time_in_micro()));
  }

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override {
    switch (memberFuncIndex) {
      case MemberFuncType::GET_CHRONO_TIME:
        return get_current_time();
      case MemberFuncType::CREATE_SIM_CHAR_STREAM:
        return create_sim_char_stream(arguments, stack);
      case MemberFuncType::RETRIEVER:
        return create_retriever(arguments, stack);
    }
    THROW("%s not implemented for nimblenetInternalTesting",
          DataVariable::get_member_func_string(memberFuncIndex));
  }

  OpReturnType create_sim_char_stream(const std::vector<OpReturnType>& arguments,
                                      CallStack& stack) {
#ifdef GENAI
    THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 3, MemberFuncType::CREATE_SIM_CHAR_STREAM);
    std::string str = arguments[0]->get_string();
    std::size_t charsPerSec = arguments[1]->get_int32();
    std::size_t bufferSize = arguments[2]->get_int32();
    // TODO: Manage this dummy stream later, it exists forever now.
    auto offloadedRun = new DummyOffloadedStream(str, charsPerSec, bufferSize, stack.task());
    return std::make_shared<CharStreamIterDataVariable>(offloadedRun->char_stream());
#else   // GENAI
    THROW("%s", "Add GENAI flag to build SimCharStream");
#endif  // GENAI
  }

  OpReturnType create_retriever(const std::vector<OpReturnType>& arguments, CallStack& stack);

  nlohmann::json to_json() const override { return "[NimbleNetInternal]"; }

 public:
  NimbleNetInternalDataVariable(CommandCenter* commandCenter) { _commandCenter = commandCenter; }

  std::string print() override { return fallback_print(); }
};
