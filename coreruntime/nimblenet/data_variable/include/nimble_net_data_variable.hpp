/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <type_traits>

#include "dataframe_variable.hpp"
#include "model_nimble_net_variable.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann/json_fwd.hpp"
#include "pre_processor_nimble_net_variable.hpp"
#include "raw_event_store_data_variable.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"

#ifndef MINIMAL_BUILD
#include "concurrent_executor_variable.hpp"
#endif  // MINIMAL_BUILD

class CommandCenter;

/**
 * @brief Main data variable class for NimbleNet operations
 *
 * NimbleNetDataVariable serves as the primary interface for all NimbleNet-specific
 * operations in the data variable system. It provides functionality for creating
 * tensors, loading models and LLMs, mathematical operations, data management,
 * and system configuration access.
 *
 * The class implements a comprehensive set of operations including:
 * - Tensor creation and manipulation
 * - Model and LLM loading with async support
 * - Mathematical functions (exp, pow, min, max, sum, mean, log)
 * - Data storage and retrieval (raw events, dataframes)
 * - System utilities (time, configuration access)
 * - Concurrent execution support
 *
 * All operations are dispatched through the call_function method using
 * member function indices, providing a unified interface for script execution.
 */
class NimbleNetDataVariable final : public DataVariable {
  CommandCenter* _commandCenter =
      nullptr; /**< Pointer to the command center for system operations */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  /*
  DELITEPY_DOC_BLOCK_BEGIN

def zeros(shape: list[int], dtype: str) -> Tensor:
    """
    Creates and return a tensor with zeroes of given shape and data type.

    Parameters
    ----------
    shape : list[int]
        Desired shape of the tensor.
    dtype : str
        Data type with which to create the tensor.

    Returns
    ----------
    tensor : Tensor
        Returns the tensor of the shape and data type filled with zeros.
    """
    pass
  DELITEPY_DOC_BLOCK_END
  */
  OpReturnType create_tensor(const std::vector<OpReturnType>& arguments);
  OpReturnType load_model(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType load_llm(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType get_current_time(const std::vector<OpReturnType>& arguments);

  OpReturnType get_config(const std::vector<OpReturnType>& arguments);

  OpReturnType get_exp(const std::vector<OpReturnType>& arguments);
  OpReturnType get_pow(const std::vector<OpReturnType>& arguments);

  OpReturnType get_raw_events_store(const std::vector<OpReturnType>& arguments);

  OpReturnType get_dataframe(const std::vector<OpReturnType>& arguments);

  OpReturnType min(const std::vector<OpReturnType>& args);

  OpReturnType max(const std::vector<OpReturnType>& args);

  OpReturnType sum(const std::vector<OpReturnType>& args);

  OpReturnType mean(const std::vector<OpReturnType>& args);

  OpReturnType log(const std::vector<OpReturnType>& args);

  OpReturnType create_retriever(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType create_json_document(const std::vector<OpReturnType>& arguments, CallStack& stack) {
    THROW("%s", "Currently not supporting loading JSON document directly");
  }

  std::vector<std::map<std::string, std::string>> get_compatible_llms(CommandCenter* commandCenter);

  OpReturnType list_compatible_llms(const std::vector<OpReturnType>& arguments);

  OpReturnType create_concurrent_executor(const std::vector<OpReturnType>& arguments);

  OpReturnType set_threads(const std::vector<OpReturnType>& arguments);

#ifdef IOS
  OpReturnType convert_text_to_phonemes(const std::vector<OpReturnType>& arguments);
#endif  // IOS

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  nlohmann::json to_json() const override { return "[NimbleNet]"; }

 public:
  NimbleNetDataVariable(CommandCenter* commandCenter) { _commandCenter = commandCenter; }

  std::string print() override { return fallback_print(); }
};
