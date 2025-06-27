/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "data_variable.hpp"

/**
 * @brief DataVariable implementation for representing exception/error states
 *
 * ExceptionDataVariable is used to wrap error messages and exception states
 * in the NimbleNet data variable system. It provides a consistent interface
 * for handling errors while maintaining the DataVariable contract.
 * All operations return sensible defaults or the error message as appropriate.
 */
class ExceptionDataVariable final : public DataVariable {
  std::string _errorMessage; /**< The error message describing the exception */

 public:
  /**
   * @brief Constructs an ExceptionDataVariable with the given error message
   * @param errorMessage The error message to store
   */
  ExceptionDataVariable(const std::string& errorMessage) : _errorMessage(errorMessage) {}

  int get_dataType_enum() const override { return DATATYPE::EXCEPTION; }

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  std::string print() override { return _errorMessage; }

  nlohmann::json to_json() const override { return "[Exception]"; }
};