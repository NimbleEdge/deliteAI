/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <utility>

#include "data_variable.hpp"

/**
 * @brief Function pointer type for custom functions
 *
 * Defines a function pointer type that takes a vector of OpReturnType arguments
 * and a CallStack reference, returning an OpReturnType.
 */
typedef OpReturnType (*CustomFuncPtr)(const std::vector<OpReturnType>&, CallStack&);

/**
 * @brief Standard function type for custom functions
 *
 * Defines a std::function type that takes a vector of OpReturnType arguments
 * and a CallStack reference, returning an OpReturnType. This provides more
 * flexibility than the function pointer type.
 */
typedef std::function<OpReturnType(const std::vector<OpReturnType>&, CallStack&)> CustomStdFunction;

// TODO (puneet): Rename to ForeignFunctionDataVariable
/**
 * @brief A foreign function.
 *
 * A function not defined in the DelitePy language.
 * It can be:
 * <ul>
 *   <li>a built-in function, bundled with the DelitePy interpreter; typically written in C++</li>
 *   <li>a client provided function, fed into the interpreter; typically written in
 * Swift/Kotlin/Python/C++</li>
 * </ul>
 */
class CustomFuncDataVariable final : public DataVariable {
  CustomStdFunction _func; /**< The custom function to be executed */

  /**
   * @brief Get the data type enumeration for this variable
   * @return DATATYPE::FUNCTION indicating this is a function type
   */
  int get_dataType_enum() const final { return DATATYPE::FUNCTION; }

  /**
   * @brief Get the container type for this variable
   * @return CONTAINERTYPE::FUNCTIONDEF indicating this is a function definition
   */
  int get_containerType() const final { return CONTAINERTYPE::FUNCTIONDEF; }

  /**
   * @brief Get the boolean representation of this function
   * @return true since functions are always truthy
   */
  bool get_bool() final { return true; }

  /**
   * @brief Convert this function to JSON representation
   * @return A JSON string "[Function]" representing this function
   */
  nlohmann::json to_json() const override { return "[Function]"; }

 public:
  /**
   * @brief Construct a CustomFuncDataVariable with a const reference to a function
   * @param func The custom function to be stored
   */
  explicit CustomFuncDataVariable(const CustomStdFunction& func) { _func = func; }

  /**
   * @brief Construct a CustomFuncDataVariable with a move reference to a function
   * @param func The custom function to be moved and stored
   */
  explicit CustomFuncDataVariable(CustomStdFunction&& func) { _func = std::move(func); }

  /**
   * @brief Virtual destructor
   */
  virtual ~CustomFuncDataVariable() = default;

  /**
   * @brief Execute the stored custom function with the given arguments
   * @param arguments Vector of arguments to pass to the function
   * @param stack Reference to the call stack for execution context
   * @return OpReturnType The result of executing the function
   */
  OpReturnType execute_function(const std::vector<OpReturnType>& arguments,
                                CallStack& stack) override;

  /**
   * @brief Get a string representation of this function
   * @return String representation using the fallback print method
   */
  std::string print() override { return fallback_print(); }
};
