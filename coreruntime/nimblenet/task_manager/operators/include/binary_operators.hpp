/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "data_variable.hpp"
#include "logger.hpp"
#include "ne_type_traits.hpp"
#include "nimble_net_util.hpp"
#include "operator_types.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"
#include "util.hpp"

/**
 * @brief Base class for binary operations on DataVariable objects
 *
 * Provides common functionality for comparing and performing binary operations
 * on different types of DataVariable objects including single variables, lists, and maps.
 */
class BaseBinOp {
 public:
  /**
   * @brief Compares two DataVariable objects for equality
   *
   * Handles comparison of different container types (single, list, map) and
   * performs type-specific comparisons for basic data types.
   *
   * @param a First DataVariable to compare
   * @param b Second DataVariable to compare
   * @return true if the variables are equal, false otherwise
   */
  static bool compare_equal(const OpReturnType& a, const OpReturnType& b) {
    // If the container types are different, they're not equal
    if (a->get_containerType() != b->get_containerType()) {
      return false;
    }

    // If they're both single variables, compare their values
    if (a->is_single() && b->is_single()) {
      // For basic types, try a simple comparison
      if (a->get_dataType_enum() == b->get_dataType_enum()) {
        switch (a->get_dataType_enum()) {
          case DATATYPE::INT32:
            return a->get_int32() == b->get_int32();
          case DATATYPE::INT64:
            return a->get_int64() == b->get_int64();
          case DATATYPE::FLOAT:
            return a->get_float() == b->get_float();
          case DATATYPE::DOUBLE:
            return a->get_double() == b->get_double();
          case DATATYPE::STRING:
            return a->get_string() == b->get_string();
          case DATATYPE::BOOLEAN:
            return a->get_bool() == b->get_bool();
          default:
            // For other types, we need a more complex comparison
            return a->print() == b->print();
        }
      }
      return false;
    }

    // For lists, compare each element
    if (a->get_containerType() == CONTAINERTYPE::LIST &&
        b->get_containerType() == CONTAINERTYPE::LIST) {
      if (a->get_size() != b->get_size()) {
        return false;
      }

      for (int i = 0; i < a->get_size(); i++) {
        if (!compare_equal(a->get_int_subscript(i), b->get_int_subscript(i))) {
          return false;
        }
      }
      return true;
    }

    // For maps, compare each key-value pair
    if (a->get_containerType() == CONTAINERTYPE::MAP &&
        b->get_containerType() == CONTAINERTYPE::MAP) {
      if (a->get_size() != b->get_size()) {
        return false;
      }

      const auto& map_a = a->get_map();
      const auto& map_b = b->get_map();

      for (const auto& [key, value] : map_a) {
        auto it_b = map_b.find(key);
        if (it_b == map_b.end()) {
          return false;  // Key not found in map b
        }
        if (!compare_equal(value, it_b->second)) {
          return false;  // Values don't match
        }
      }
      return true;
    }

    // For other container types, compare their string representation
    return a->print() == b->print();
  }

  /**
   * @brief Performs the specified binary operation on two values
   *
   * Routes the operation to the appropriate method based on the operation type.
   *
   * @param val1 First operand
   * @param val2 Second operand
   * @param opType String identifier for the operation ("Add", "Sub", "Mult", "Div", "Pow", "Mod")
   * @return Result of the operation or nullptr if operation is not supported
   */
  OpReturnType perform_operation(OpReturnType val1, OpReturnType val2, std::string& opType) {
    if (opType == "Add") {
      return add(val1, val2);
    } else if (opType == "Sub") {
      return sub(val1, val2);
    } else if (opType == "Mult") {
      return mult(val1, val2);
    } else if (opType == "Div") {
      return div(val1, val2);
    } else if (opType == "Pow") {
      return pow(val1, val2);
    } else if (opType == "Mod") {
      return mod(val1, val2);
    }
    return nullptr;
  }

  /** @brief Virtual method for addition operation */
  virtual OpReturnType add(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  /** @brief Virtual method for subtraction operation */
  virtual OpReturnType sub(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  /** @brief Virtual method for multiplication operation */
  virtual OpReturnType mult(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  /** @brief Virtual method for division operation */
  virtual OpReturnType div(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  /** @brief Virtual method for power operation */
  virtual OpReturnType pow(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  /** @brief Virtual method for modulo operation */
  virtual OpReturnType mod(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual ~BaseBinOp() = default;
};

/**
 * @brief Specialized modulo operator for numeric types
 *
 * Handles modulo operation with proper sign handling for floating-point types.
 * Ensures the result has the same sign as the divisor when possible.
 */
template <typename T,
          typename = std::enable_if_t<ne::is_one_of_v<T, float, int32_t, double, int64_t>>>
struct ModOperator {
  /**
   * @brief Computes modulo operation with proper sign handling
   *
   * @param a Dividend
   * @param b Divisor
   * @return Result of a % b with proper sign handling
   */
  static T compute(T a, T b) {
    auto result = std::fmod(a, b);  // Works for integer types
    if (result < 0 && b > 0) {
      result += b;
    }
    return result;
  }
};

/**
 * @brief Template class for numeric binary operations
 *
 * Provides implementations of all binary operations (add, sub, mult, div, pow, mod)
 * for numeric types (float, int32_t, double, int64_t).
 */
template <typename T,
          typename = std::enable_if_t<ne::is_one_of_v<T, float, int32_t, double, int64_t>>>
class NumericBinOp : public BaseBinOp {
 public:
  /** @brief Adds two numeric values */
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() + val2->get<T>()));
  }

  /** @brief Subtracts second value from first */
  OpReturnType sub(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() - val2->get<T>()));
  }

  /** @brief Multiplies two numeric values */
  OpReturnType mult(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() * val2->get<T>()));
  }

  /**
   * @brief Divides first value by second
   * @throws Exception if division by zero is attempted
   */
  OpReturnType div(OpReturnType val1, OpReturnType val2) const override {
    if (val2->get<T>() == (T)0) {
      THROW("%s", "Division by zero will result in undefined behaviour.");
    }
    return OpReturnType(new SingleVariable<T>(val1->get<T>() / val2->get<T>()));
  }

  /** @brief Raises first value to the power of second */
  OpReturnType pow(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(std::pow(val1->get<T>(), val2->get<T>())));
  }

  /**
   * @brief Computes modulo of first value by second
   * @throws Exception if modulo by zero is attempted
   */
  OpReturnType mod(OpReturnType val1, OpReturnType val2) const override {
    if (val2->get<T>() == (T)0) {
      THROW("%s", "Modulo by zero error.");
    }
    T result = ModOperator<T>::compute(val1->get<T>(), val2->get<T>());
    return OpReturnType(new SingleVariable<T>(result));
  }
};

/**
 * @brief Binary operations for string types
 *
 * Currently supports string concatenation (addition operation).
 */
class StringBinOp : public BaseBinOp {
 public:
  /** @brief Concatenates two strings */
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<std::string>(val1->get_string() + val2->get_string()));
  }
};

/**
 * @brief Binary operations for list types
 *
 * Supports list concatenation (addition) and list repetition (multiplication).
 */
class ListBinOp : public BaseBinOp {
 public:
  /** @brief Concatenates two lists */
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override;

  /** @brief Repeats a list by the specified number of times */
  OpReturnType mult(OpReturnType val1, OpReturnType val2) const override;
};

/**
 * @brief Main class for performing binary operations
 *
 * Routes operations to appropriate specialized classes based on operand types.
 * Supports numeric, string, and list operations.
 */
class BinaryOperators {
 public:
  /**
   * @brief Performs binary operation on two operands
   *
   * Automatically selects the appropriate operation handler based on operand types:
   * - Lists: Uses ListBinOp
   * - Tensors: Currently throws exception (not supported)
   * - Numeric: Uses NumericBinOp with appropriate type promotion
   * - Strings: Uses StringBinOp
   *
   * @param v1 First operand
   * @param v2 Second operand
   * @param opType Operation type string
   * @return Result of operation or nullptr if operation is not supported
   */
  static OpReturnType operate(OpReturnType v1, OpReturnType v2, std::string& opType) {
    // First, check for list operations
    if (v1->get_containerType() == CONTAINERTYPE::LIST ||
        v2->get_containerType() == CONTAINERTYPE::LIST) {
      static ListBinOp listOp;
      return listOp.perform_operation(v1, v2, opType);
    } else if (auto t1 = std::dynamic_pointer_cast<BaseTensorVariable>(v1),
               t2 = std::dynamic_pointer_cast<BaseTensorVariable>(v2);
               t1 && t2) {
      THROW("%s", "tensor ops not supported");
    } else if (v1->is_numeric() && v2->is_numeric()) {
      auto returnType = get_max_dataType(v1->get_dataType_enum(), v2->get_dataType_enum());
      switch (returnType) {
        case DATATYPE::FLOAT: {
          NumericBinOp<float> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::INT32: {
          NumericBinOp<int32_t> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::DOUBLE: {
          NumericBinOp<double> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::INT64: {
          NumericBinOp<int64_t> n;
          return n.perform_operation(v1, v2, opType);
        }
      }
    } else if (v1->is_string() && v2->is_string()) {
      StringBinOp s;
      return s.perform_operation(v1, v2, opType);
    }

    return nullptr;
  }
};
