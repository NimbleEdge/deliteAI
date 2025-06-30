/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "data_variable.hpp"
#include "operator_types.hpp"
#include "single_variable.hpp"

typedef OpReturnType (*CompareFuncPtr)(OpReturnType, OpReturnType);

/**
 * @brief Template class for comparison operations
 *
 * Provides a generic framework for implementing comparison operations
 * between two DataVariable objects of the same type.
 *
 * @tparam Oper The comparison operator class (e.g., EqualOp, GreaterOp)
 * @tparam T The data type for comparison
 */
template <template <typename T> class Oper, typename T>
class CompareOp {
 public:
  /**
   * @brief Performs comparison operation between two operands
   *
   * @param v1 First operand
   * @param v2 Second operand
   * @return Result of comparison as boolean DataVariable
   */
  static OpReturnType operate(OpReturnType v1, OpReturnType v2);
};

/**
 * @brief Equality comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class EqualOp {
 public:
  /**
   * @brief Compares two values for equality
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 == val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 == val2));
  }
};

/**
 * @brief Greater than comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class GreaterOp {
 public:
  /**
   * @brief Compares if first value is greater than second
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 > val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 > val2));
  }
};

/**
 * @brief Greater than or equal comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class GreaterEqualOp {
 public:
  /**
   * @brief Compares if first value is greater than or equal to second
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 >= val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 >= val2));
  }
};

/**
 * @brief Less than comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class LessThanOp {
 public:
  /**
   * @brief Compares if first value is less than second
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 < val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 < val2));
  }
};

/**
 * @brief Less than or equal comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class LessThanEqualOp {
 public:
  /**
   * @brief Compares if first value is less than or equal to second
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 <= val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 <= val2));
  }
};

/**
 * @brief Not equal comparison operator
 *
 * @tparam T The data type for comparison
 */
template <typename T>
class NotEqualOp {
 public:
  /**
   * @brief Compares if two values are not equal
   *
   * @param val1 First value
   * @param val2 Second value
   * @return Boolean result of val1 != val2
   */
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 != val2));
  }
};

/**
 * @brief Main class for comparison operations
 *
 * Provides a unified interface for all comparison operations including
 * numeric comparisons, string comparisons, and membership testing.
 */
class CompareOperators {
  static std::map<std::string, CompareFuncPtr> _compareOpMap;

 public:
  /**
   * @brief Performs comparison operation between two operands
   *
   * Automatically handles type promotion for numeric comparisons and
   * routes to appropriate comparison implementation based on operand types.
   *
   * @tparam Oper The comparison operator template class
   * @param v1 First operand
   * @param v2 Second operand
   * @return Result of comparison or nullptr if operation not supported
   */
  template <template <typename T> class Oper>
  static OpReturnType operate(OpReturnType v1, OpReturnType v2) {
    if (v1->is_numeric() && v2->is_numeric()) {
      auto returnType = get_max_dataType(v1->get_dataType_enum(), v2->get_dataType_enum());

      switch (returnType) {
        case DATATYPE::FLOAT:
          return CompareOp<Oper, float>::operate(v1, v2);
        case DATATYPE::INT32:
          return CompareOp<Oper, int32_t>::operate(v1, v2);
        case DATATYPE::DOUBLE:
          return CompareOp<Oper, double>::operate(v1, v2);
        case DATATYPE::INT64:
          return CompareOp<Oper, int64_t>::operate(v1, v2);
      }
    } else if (v1->is_string() && v2->is_string()) {
      return CompareOp<Oper, std::string>::operate(v1, v2);
    }
    // TODO: How to add comparing a variable with None for e.g. if var == None:
    return nullptr;
  }

  /**
   * @brief Gets the comparison function for the specified operation type
   *
   * @param opType String identifier for the comparison operation
   * @return Function pointer to the comparison operation
   */
  static CompareFuncPtr get_operator(const std::string& opType);

  /**
   * @brief Tests if first value is contained in second value
   *
   * @param v1 Value to search for
   * @param v2 Container to search in
   * @return Boolean result of membership test
   */
  static OpReturnType in(OpReturnType v1, OpReturnType v2) {
    return OpReturnType(new SingleVariable<bool>(v2->in(v1)));
  }

  /**
   * @brief Tests if first value is not contained in second value
   *
   * @param v1 Value to search for
   * @param v2 Container to search in
   * @return Boolean result of inverse membership test
   */
  static OpReturnType notIn(OpReturnType v1, OpReturnType v2) {
    return OpReturnType(new SingleVariable<bool>(v2->notIn(v1)));
  }
};

/**
 * @brief Implementation of comparison operation for specific types
 *
 * @tparam Operation The comparison operator template class
 * @tparam T The data type for comparison
 * @param v1 First operand
 * @param v2 Second operand
 * @return Result of comparison operation
 */
template <template <typename Type> class Operation, class T>
OpReturnType CompareOp<Operation, T>::operate(OpReturnType v1, OpReturnType v2) {
  if (v1->is_single() && v2->is_single()) {
    auto val1 = v1->get<T>();
    auto val2 = v2->get<T>();
    return Operation<T>::operate_single(val1, val2);
  }
  return nullptr;
}
