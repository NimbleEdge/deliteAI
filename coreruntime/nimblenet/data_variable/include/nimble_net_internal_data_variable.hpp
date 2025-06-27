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

/**
 * @brief Internal data variable for NimbleNet system operations
 *
 * NimbleNetInternalDataVariable provides specialized functionality for internal
 * NimbleNet operations that are not part of the standard data variable interface.
 * This class handles operations such as time retrieval, character stream creation,
 * and retriever instantiation for AI/ML workflows.
 *
 * The class is designed to work with the CommandCenter for coordinating internal
 * system operations and provides a bridge between the data variable system and
 * NimbleNet's internal services.
 */
class NimbleNetInternalDataVariable final : public DataVariable {
  CommandCenter* _commandCenter = nullptr; /**< Pointer to the command center for system coordination */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_INTERNAL; }

  /**
   * @brief Retrieves the current system time in microseconds
   * @return OpReturnType containing the current time as a SingleVariable<int64_t>
   */
  OpReturnType get_current_time() {
    return OpReturnType(new SingleVariable<int64_t>(Time::get_time_in_micro()));
  }

  /**
   * @brief Dispatches member function calls based on function index
   * @param memberFuncIndex The index of the member function to call
   * @param arguments Vector of arguments to pass to the function
   * @param stack The call stack for execution context
   * @return OpReturnType containing the function result
   * @throws Exception for unimplemented or unsupported function indices
   */
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

  /**
   * @brief Creates a simulated character stream for testing purposes
   * @param arguments Vector containing [string, charsPerSec, bufferSize]
   * @param stack The call stack for execution context
   * @return OpReturnType containing a CharStreamIterDataVariable
   * @throws Exception if GENAI flag is not defined or arguments don't match
   */
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

  /**
   * @brief Creates a retriever instance for AI/ML operations
   * @param arguments Vector of arguments for retriever configuration
   * @param stack The call stack for execution context
   * @return OpReturnType containing a RetrieverDataVariable
   */
  OpReturnType create_retriever(const std::vector<OpReturnType>& arguments, CallStack& stack);

  nlohmann::json to_json() const override { return "[NimbleNetInternal]"; }

 public:
  /**
   * @brief Constructor for NimbleNetInternalDataVariable
   * @param commandCenter Pointer to the command center for system coordination
   */
  NimbleNetInternalDataVariable(CommandCenter* commandCenter) { _commandCenter = commandCenter; }

  std::string print() override { return fallback_print(); }
};
