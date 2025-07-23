/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <functional>

#include "custom_func_data_variable.hpp"
#include "range_data_variable.hpp"
#include "raw_event_store_data_variable.hpp"
#include "single_variable.hpp"
#include "variable_scope.hpp"

/**
 * @brief Collection of built-in functions and operators
 *
 * Provides implementations for common Python-like functions such as print,
 * range, type conversions, and decorators for event handling and concurrency.
 */
class CustomFunctions {
  /**< Map of function names to their implementations */
  static std::map<std::string, CustomFuncPtr> _customFuncMap;
  friend class InbuiltFunctionsStatement;

 public:
/*
DELITEPY_DOC_BLOCK_BEGIN
- `print()` function
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Prints all arguments to debug log with space separation
   *
   * @param args Vector of arguments to print
   * @param stack Current call stack (unused)
   * @return NoneVariable
   */
  static OpReturnType print(const std::vector<OpReturnType>& args, CallStack& stack) {
    std::string printVar = "";
    for (auto d : args) {
      printVar += d->print() + " ";
    }
    LOG_TO_CLIENT_DEBUG("%s", printVar.c_str());
    return OpReturnType(new NoneVariable());
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `range()` function
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Creates a range data variable with specified size
   *
   * @param args Vector containing single integer argument for range size
   * @param stack Current call stack (unused)
   * @return RangeDataVariable with specified size
   * @throws Exception if size is negative or wrong number of arguments
   */
  static OpReturnType range(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("%s", "range expects only a single argument");
    }
    int size = args[0]->get_int32();
    if (size < 0) {
      THROW("range should be +ve got %d", size);
    }
    return OpReturnType(new RangeDataVariable(size));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `not` operator
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Logical NOT operator for boolean values
   *
   * @param args Vector containing single boolean argument
   * @param stack Current call stack (unused)
   * @return Boolean result of logical NOT operation
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType inverse_bool(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("%s", "not expects only a single argument");
    }
    return OpReturnType(new SingleVariable<bool>(!args[0]->get_bool()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `str()` constructor
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Converts argument to string representation
   *
   * @param args Vector containing single argument to convert
   * @param stack Current call stack (unused)
   * @return String representation of the argument
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType str(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("str expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<std::string>(args[0]->print()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `int()` constructor
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Converts argument to 32-bit integer
   *
   * @param args Vector containing single argument to convert
   * @param stack Current call stack (unused)
   * @return 32-bit integer representation of the argument
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType cast_int(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("int expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<int32_t>(args[0]->cast_int32()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `float()` constructor
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Converts argument to float
   *
   * @param args Vector containing single argument to convert
   * @param stack Current call stack (unused)
   * @return Float representation of the argument
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType cast_float(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("float expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<float>(args[0]->cast_float()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `bool()` constructor
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Converts argument to boolean
   *
   * @param args Vector containing single argument to convert
   * @param stack Current call stack (unused)
   * @return Boolean representation of the argument
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType cast_bool(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("bool expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<bool>(args[0]->get_bool()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `len()` function
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Returns the size/length of the argument
   *
   * @param args Vector containing single argument to measure
   * @param stack Current call stack (unused)
   * @return Integer representing the size of the argument
   * @throws Exception if wrong number of arguments
   */
  static OpReturnType len(const std::vector<OpReturnType>& args, CallStack& stack) {
    if (args.size() != 1) {
      THROW("len expects a single argument, provided %d.", args.size());
    }
    // In case of floats, it will return trailing zeros e.g. for 1.1, it returns 1.10000
    return OpReturnType(new SingleVariable<int32_t>(args[0]->get_size()));
  }

/*
DELITEPY_DOC_BLOCK_BEGIN
- `Exception` class
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Creates an exception object
   *
   * @param arguments Arguments for exception creation
   * @param stack Current call stack
   * @return Exception object
   */
  static OpReturnType create_exception(const std::vector<OpReturnType>& arguments,
                                       CallStack& stack);

  /**
   * @brief Decorator function for event handling
   *
   * Returns a callable function data variable that can be used
   * to add events to the specified raw store.
   *
   * @param rawStoreDataVariables Vector containing raw store data variables
   * @param stack Current call stack
   * @return Callable function data variable
   */
  static OpReturnType add_event(const std::vector<OpReturnType>& rawStoreDataVariables,
                                CallStack& stack);

/*
DELITEPY_DOC_BLOCK_BEGIN
- `concurrent()` decorator
DELITEPY_DOC_BLOCK_END
*/
  /**
   * @brief Decorator function for concurrent execution
   *
   * Returns a callable function data variable that can be executed
   * concurrently with other functions.
   *
   * @param arguments Vector containing function arguments
   * @param stack Current call stack
   * @return Callable function data variable for concurrent execution
   */
  static OpReturnType concurrent(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Hook function called before adding events
   *
   * @param typesDataVariable Data variable containing type information
   * @param stack Current call stack
   * @return Result of pre-add event processing
   */
  static OpReturnType pre_add_event_hook(const std::vector<OpReturnType>& typesDataVariable,
                                         CallStack& stack);

  /*
  DELITEPY_DOC_BLOCK_BEGIN
- `next()` function
  DELITEPY_DOC_BLOCK_END
  */
  /**
   * @brief Returns the next item from an iterable
   *
   * @param args Vector containing one iterable object
   * @param stack Current call stack
   * @return Next element in the iterable
   */
  static OpReturnType next(const std::vector<OpReturnType>& args, CallStack& stack);

  /*
  DELITEPY_DOC_BLOCK_BEGIN
- `enumerate()` function
  DELITEPY_DOC_BLOCK_END
  */
  /**
   * @brief Returns an enumerate object over an iterable
   *
   * Wraps the given iterable with an index counter, yielding (index, value) pairs.
   *
   * @param args Vector containing one iterable object
   * @param stack Current call stack
   * @return Iterable object yielding (index, value) pairs
   */
  static OpReturnType enumerate(const std::vector<OpReturnType>& args, CallStack& stack);
};
