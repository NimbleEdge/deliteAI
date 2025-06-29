/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "command_center.hpp"
#include "data_variable.hpp"

class CommandCenter;

/**
 * @brief Data variable for Retriever, enabling Retrieval-Augmented Generation (RAG) in AI workflows.
 *
 * This class encapsulates the logic for retrieving relevant documents given a query, using embedding models and document stores.
 */
class RetrieverDataVariable final : public DataVariable {
  CommandCenter* _commandCenter;           /**< Pointer to the command center. */
  OpReturnType _embeddingModel;            /**< Model for converting text into vector embeddings. */
  OpReturnType _embeddingStoreModel;       /**< Model for handling similarity search over embedding vectors. */
  OpReturnType _documentStore;             /**< Store containing retrievable documents. */

  /**
   * @brief Get the container type for this variable.
   *
   * @return Always CONTAINERTYPE::SINGLE for RetrieverDataVariable.
   */
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  /**
   * @brief Get the boolean value of this variable.
   *
   * @return Always true for RetrieverDataVariable.
   */
  bool get_bool() override { return true; }

  /**
   * @brief Get the data type enum for this variable.
   *
   * @return Always DATATYPE::NIMBLENET for RetrieverDataVariable.
   */
  int get_dataType_enum() const override { return DATATYPE::NIMBLENET; }

  /**
   * @brief Serialize this variable to JSON.
   *
   * @return JSON representation as a string.
   */
  nlohmann::json to_json() const override { return "[Retriever]"; }

 public:
  /**
   * @brief Constructor for RetrieverDataVariable.
   *
   * @param commandCenter_ Pointer to the command center.
   * @param arguments Vector containing embedding model, embedding store model, and document store.
   */
  RetrieverDataVariable(CommandCenter* commandCenter_, const std::vector<OpReturnType>& arguments);

  /**
   * @brief Print a string representation of this variable.
   *
   * @return String representation.
   */
  std::string print() override { return fallback_print(); }

 private:
  /**
   * @brief Retrieve top-k relevant documents for a query.
   *
   * @param arguments Vector containing the query string and k.
   * @param stack Call stack for function invocation context.
   * @return Tuple containing scores and documents.
   */
  OpReturnType topk(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Call a member function by index.
   *
   * @param memberFuncIndex Index of the member function.
   * @param arguments Arguments for the function.
   * @param stack Call stack for function invocation context.
   * @return Result of the function call.
   */
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;
};
