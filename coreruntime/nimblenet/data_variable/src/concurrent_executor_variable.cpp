/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "concurrent_executor_variable.hpp"

#include <optional>

std::unique_ptr<ThreadPool> ConcurrentExecutorVariable::_threadpool = nullptr;

/**
 * @brief Calculate default number of threads for the thread pool
 * @return Number of threads (max of hardware_concurrency-1 and 2)
 * @details Uses hardware_concurrency() to determine optimal thread count,
 *          subtracting 1 to leave one core for the main thread, with a minimum of 2
 */
static inline int default_num_threads() {
  return std::max(int(std::thread::hardware_concurrency() - 1), 2);
}

int ConcurrentExecutorVariable::_numThreads = default_num_threads();

/**
 * @brief Initialize the thread pool
 * @return Always returns true
 * @details Creates a new ThreadPool instance with the configured number of threads
 */
bool ConcurrentExecutorVariable::init_threadpool() {
  // run once to init Threadpool
  _threadpool = std::make_unique<ThreadPool>(_numThreads);
  return true;
}

void ConcurrentExecutorVariable::set_threadpool_threads(int threadCount) {
  if (_threadpool != nullptr) {
    THROW("Threadpool is already created can't set threads now");
  }
  if (threadCount < 1) {
    THROW("ThreadCount cannot be less than 1 given %d", threadCount);
  }
  _numThreads = threadCount;
}

ConcurrentExecutorVariable::ConcurrentExecutorVariable() {
  if (_threadpool == nullptr) {
    _threadpool = std::make_unique<ThreadPool>(_numThreads);
  }
}

OpReturnType ConcurrentExecutorVariable::run_sync(const std::vector<OpReturnType>& arguments,
                                                  CallStack& stack) {
  std::lock_guard<std::mutex> locker(_mutex);
  if (arguments.size() < 1) {
    THROW("%s", "sync requires atleast one argument, the function to call");
  }
  auto functionDataVariable = arguments[0];
  std::vector<OpReturnType> remainingArgs(arguments.begin() + 1, arguments.end());

  return functionDataVariable->execute_function(remainingArgs, stack);
}

OpReturnType ConcurrentExecutorVariable::run_parallel(const std::vector<OpReturnType>& arguments,
                                                      CallStack& stack) {
  if (arguments.size() < 2) {
    THROW(
        "run_parallel requires atleast 2 arguments 1st function and 2nd iteratable got %d "
        "arguments",
        arguments.size());
  }
  auto functionDataVariable = arguments[0];
  auto iteratableArg = arguments[1];
  std::vector<OpReturnType> remainingArgs(arguments.begin() + 1, arguments.end());

  std::vector<std::future<OpReturnType>> returnVals;
  int totalParallelCalls = iteratableArg->get_size();

  // using atomic variable to early cancel other tasks if thread is complete.
  std::shared_ptr<std::atomic<bool>> toCancel = std::make_shared<std::atomic<bool>>(false);
  for (int i = 0; i < totalParallelCalls; i++) {
    // set 1st argument of the remaining args with item in the iteratable
    remainingArgs[0] = iteratableArg->get_int_subscript(i);

    auto lambdaFunc = [func = functionDataVariable, args = remainingArgs, &stack,
                       cancel = toCancel]() -> OpReturnType {
      if (*cancel) {
        return nullptr;
      }
      // TODO: when we change script lock, ensure that newStack is captured by value in lambda
      auto newStack = stack.create_copy_with_deferred_lock();
      return func->execute_function(args, newStack);
    };
    returnVals.emplace_back(_threadpool->enqueue(lambdaFunc));
  }
  std::vector<OpReturnType> returnList;
  std::optional<std::runtime_error> firstError;
  for (int i = 0; i < totalParallelCalls; i++) {
    OpReturnType ret;
    std::future_status status;
    while ((status = returnVals[i].wait_for(std::chrono::milliseconds(0))) !=
           std::future_status::ready) {
      _threadpool->run_threadpool_task();
    }
    try {
      ret = returnVals[i].get();
    } catch (std::runtime_error& e) {
      *toCancel = true;
      if (!firstError) {
        firstError = e;  // Store only the first exception
      }
    }
    returnList.push_back(ret);
  }
  if (firstError) {
    throw *firstError;
  }
  return std::make_shared<ListDataVariable>(std::move(returnList));
}

OpReturnType ConcurrentExecutorVariable::call_function(int memberFuncIndex,
                                                       const std::vector<OpReturnType>& arguments,
                                                       CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::SYNC:
      return run_sync(arguments, stack);
    case MemberFuncType::RUNPARALLEL:
      return run_parallel(arguments, stack);
  }
  THROW("%s not implemented for nimblenet", DataVariable::get_member_func_string(memberFuncIndex));
}
