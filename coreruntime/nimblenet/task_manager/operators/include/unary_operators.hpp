/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "data_variable.hpp"
#include "single_variable.hpp"

typedef OpReturnType (*UnaryOpFuncPtr)(OpReturnType);

/**
 * @brief Main class for unary operations
 *
 * Provides a unified interface for unary operations including
 * logical NOT and unary subtraction (negation).
 */
class UnaryOperators {
  /**< Map of operation names to their function pointers */
  static std::map<std::string, UnaryOpFuncPtr> _unaryOpMap;

 public:
  /**
   * @brief Gets the unary function for the specified operation type
   *
   * @param opType String identifier for the unary operation
   * @return Function pointer to the unary operation
   */
  static UnaryOpFuncPtr get_operator(const std::string& opType);

  /**
   * @brief Logical NOT operation
   *
   * Inverts the boolean value of the operand.
   *
   * @param v Operand to negate
   * @return Boolean result of logical NOT operation
   */
  static OpReturnType inverse_bool(OpReturnType v) {
    bool val = v->get_bool();
    return OpReturnType(new SingleVariable<bool>(!val));
  }

  /**
   * @brief Unary subtraction (negation) operation
   *
   * Delegates to the operand's unary_sub method for type-specific negation.
   *
   * @param v Operand to negate
   * @return Result of unary subtraction operation
   */
  static OpReturnType unary_sub(OpReturnType v) { return v->unary_sub(); }
};
