/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "future_data_variable.hpp"

#include <memory>

#include "core_utils/fmt.hpp"
#include "task.hpp"

/**
 * @brief Constructs a FutureDataVariable that wraps a std::future
 * @param future The std::future to wrap
 * @param name The name of the future
 * @param dependentJob The job that produced this future
 * @param isTaskInitializing Flag indicating if task is in initialization phase
 * @throw std::runtime_error if task is not in initialization phase
 */
FutureDataVariable::FutureDataVariable(std::future<OpReturnType>&& future, const std::string& name,
                                       std::shared_ptr<BaseJob> dependentJob,
                                       bool isTaskInitializing)
    : _future(std::move(future)), _name(name), _dependentJob(dependentJob) {
  if (!isTaskInitializing) {
    THROW(
        "Cannot create FutureDataVariable after the script has loaded, model and llm loads should "
        "be done in global scope");
  }
}

/**
 * @brief Gets the dependent job associated with this variable
 * @return A shared pointer to the dependent job
 */
std::shared_ptr<BaseJob> FutureDataVariable::get_job() { return _dependentJob; }

/**
 * @brief Checks if the value is available in the future
 * @return True if the value is available, false otherwise
 */
bool FutureDataVariable::is_available() {
  auto waitResult = _future.wait_for(std::chrono::nanoseconds(0));
  if (waitResult == std::future_status::ready) {
    _val = _future.get();
    return true;
  }
  return false;
}

/**
 * @brief Gets the value from the future
 * @return The value from the future
 * @throw std::future_error If value hasn't been set in the future
 */
OpReturnType FutureDataVariable::get() {
  if (!_val) {
    _val = _future.get();
  }
  return _val;
}

/**
 * @brief Gets the value at specified index
 * @param index The index to retrieve the value from
 * @return The value at the specified index
 * @throw std::runtime_error if asset is not loaded
 */
OpReturnType FutureDataVariable::get_int_subscript(int index) {
  if (!_val) {
    THROW("Asset '%s' not loaded", _name.c_str());
  }
  return _val->get_int_subscript(index);
}

/**
 * @brief Calls a function on the underlying value
 * @param memberFuncIndex The index of the member function to call
 * @param arguments The arguments to pass to the function
 * @param stack The call stack for the operation
 * @return The result of the function call
 * @throw std::runtime_error if asset is not loaded
 */
OpReturnType FutureDataVariable::call_function(int memberFuncIndex,
                                               const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  if (!_val) {
    THROW("Asset '%s' not loaded", _name.c_str());
  }
  return get()->call_function(memberFuncIndex, arguments, stack);
}

/**
 * @brief Saves the future operation to the given task
 * @param task The task to save the future operation to
 *
 * This function saves the FutureDataVariable to task so that Task can poll the future. This creates
 * a dependency of ScriptReadyJob on the job saved here, to ensure that task gets ready after this
 * future has been satisfied
 */
void FutureDataVariable::save_to_task(Task& task) {
  if (_savedToTask) return;
  task.save_future(std::static_pointer_cast<FutureDataVariable>(shared_from_this()));
  _savedToTask = true;
}
