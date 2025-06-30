/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "list_data_variable.hpp"
#include "thread_pool.hpp"
#include "variable_scope.hpp"

/**
 * @brief A data variable that provides concurrent execution capabilities
 * @details ConcurrentExecutorVariable extends DataVariable to provide thread-safe
 *          execution of functions. It maintains a static thread pool for parallel
 *          execution and uses mutex-based synchronization for synchronous calls.
 *          The class supports both synchronous execution (run_sync) and parallel
 *          execution (run_parallel) of functions over iterable data.
 */
class ConcurrentExecutorVariable final : public DataVariable {
  static std::unique_ptr<ThreadPool> _threadpool; /**< Static thread pool for parallel execution */
  static int _numThreads; /**< Number of threads in the thread pool */
  // mutex to implement sync calls
  std::mutex _mutex; /**< Mutex for thread-safe synchronous execution */
  static bool init_threadpool();

 public:
  /**
   * @brief Default constructor
   * @details Initializes the thread pool if it hasn't been created yet
   */
  ConcurrentExecutorVariable();

  /**
   * @brief Set the number of threads in the thread pool
   * @param threadCount Number of threads (must be >= 1)
   * @throws std::runtime_error if thread pool is already created or threadCount < 1
   */
  static void set_threadpool_threads(int threadCount);

  /**
   * @brief Get the data type enum
   * @return DATATYPE::CONCURRENT_EXECUTOR
   */
  int get_dataType_enum() const override { return DATATYPE::CONCURRENT_EXECUTOR; }

  /**
   * @brief Get the container type
   * @return CONTAINERTYPE::SINGLE
   */
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  /**
   * @brief Get boolean representation
   * @return Always returns true
   */
  bool get_bool() override { return true; }

  /**
   * @brief Get string representation
   * @return Fallback print string
   */
  std::string print() override { return fallback_print(); }

  /**
   * @brief Convert to JSON representation
   * @return JSON string "[ConcurrentExecutorVariable]"
   */
  nlohmann::json to_json() const override { return "[ConcurrentExecutorVariable]"; }

  /**
   * @brief Execute a function synchronously with thread safety
   * @param arguments Vector of arguments (first argument must be the function to call)
   * @param stack Call stack for execution context
   * @return Result of the function execution
   */
  OpReturnType run_sync(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Execute a function in parallel over an iterable
   * @param arguments Vector of arguments (first: function, second: iterable, rest: additional args)
   * @param stack Call stack for execution context
   * @return ListDataVariable containing results from all parallel executions
   */
  OpReturnType run_parallel(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Call a member function by index
   * @param memberFuncIndex Index of the member function to call
   * @param arguments Arguments to pass to the function
   * @param stack Call stack for execution context
   * @return Result of the function execution
   */
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  /**
   * @brief Reset the thread pool
   * @details Destroys the current thread pool instance
   */
  static void reset_threadpool() { _threadpool.reset(); }
};
