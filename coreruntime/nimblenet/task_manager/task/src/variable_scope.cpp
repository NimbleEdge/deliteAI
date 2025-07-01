/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "variable_scope.hpp"

#include <memory>

#include "data_variable.hpp"
#include "future_data_variable.hpp"
#include "statements.hpp"

StackLocation StackLocation::local(int moduleIndex, int functionIndex, int varIndex) {
  return StackLocation(moduleIndex, functionIndex, varIndex);
}

// Responsibility of caller to ensure that the location is correct
OpReturnType CallStack::get_variable(StackLocation loc) const {
  assert(loc._moduleIndex < _moduleToStackFrameMap.size() &&
         loc._functionIndex < _moduleToStackFrameMap[loc._moduleIndex].size());
  const auto stackFramePtr = _moduleToStackFrameMap[loc._moduleIndex][loc._functionIndex].back();
  return stackFramePtr->get(loc._varIndex);
}

// Caller will ensure the StackLocation is correct
void CallStack::set_variable(StackLocation loc, OpReturnType val) {
  assert(loc._moduleIndex < _moduleToStackFrameMap.size() &&
         loc._functionIndex < _moduleToStackFrameMap[loc._moduleIndex].size());

  if (auto futureVal = std::dynamic_pointer_cast<FutureDataVariable>(val); futureVal) {
    // Internally, the function will call _task->save_future() only once. Hence futures can be
    // passed around after getting created in the global stack frame
    futureVal->save_to_task(*task());
  }

  auto stackFramePtr = _moduleToStackFrameMap[loc._moduleIndex][loc._functionIndex].back();
  stackFramePtr->set(loc._varIndex, val);
}

CallStack::CallStack(const CallStack& other) { *this = other; }

CallStack& CallStack::operator=(const CallStack& other) {
  if (this == &other) {
    return *this;  // Handle self-assignment
  }
  _functionsStack = other._functionsStack;
  _moduleToStackFrameMap = other._moduleToStackFrameMap;
  _commandCenter = other._commandCenter;
  return *this;
}

CallStack CallStack::create_copy_with_deferred_lock() {
  // this is in 2 lines to explicitly call the copy assignment operator
  CallStack newStack(*this);
  newStack.lock = task()->get_script_deferred_lock();
  return newStack;
}

std::shared_ptr<Task> CallStack::task() noexcept { return _commandCenter->get_task(); }

void CallStack::enter_function_frame(int moduleIndex, int functionIndex, int numVariablesInFrame) {
  if (moduleIndex >= _moduleToStackFrameMap.size()) {
    _moduleToStackFrameMap.resize(moduleIndex + 1);
  }
  if (functionIndex >= _moduleToStackFrameMap[moduleIndex].size()) {
    _moduleToStackFrameMap[moduleIndex].resize(functionIndex + 1);
  }
  // Not calling make_shared since StackFrame's constructor is private
  std::shared_ptr<StackFrame> stackFramePtr{
      new StackFrame{moduleIndex, functionIndex, numVariablesInFrame}};
  _moduleToStackFrameMap[moduleIndex][functionIndex].push_back(stackFramePtr);
  _functionsStack.push_back(stackFramePtr);
}

void CallStack::exit_function_frame() {
  if (_functionsStack.size() == 0) {
    THROW("%s", "Attempting to exit function frame when there is currently no function running");
  }
  const auto currentStackFramePtr = _functionsStack.back();
  _functionsStack.pop_back();
  auto& currentFunctionExecs = _moduleToStackFrameMap[currentStackFramePtr->get_module_index()]
                                                     [currentStackFramePtr->get_function_index()];
  if (currentFunctionExecs.size() == 0) {
    THROW("%s", "Function existed in functions stack, but can't find its frame pointer");
  }
  currentFunctionExecs.pop_back();
}

VariableScope::VariableScope(VariableScope* p, bool isNewFunction) {
  _parentScope = p;
  _commandCenter = p->get_commandCenter();
  _moduleIndex = p->_moduleIndex;
  _nextFunctionIndex = p->_nextFunctionIndex;
  if (isNewFunction) {
    _currentFunctionIndex = *_nextFunctionIndex;
    *_nextFunctionIndex += 1;
    _numVariablesStack = std::make_shared<int>(0);
  } else {
    _currentFunctionIndex = p->_currentFunctionIndex;
    _numVariablesStack = p->_numVariablesStack;
  }
}

VariableScope::VariableScope(CommandCenter* commandCenter, int moduleIndex) {
  _commandCenter = commandCenter;
  _moduleIndex = moduleIndex;
  _parentScope = nullptr;
  // Index 0 is used for global scope
  _currentFunctionIndex = 0;
  _nextFunctionIndex = std::make_shared<int>(1);
  _numVariablesStack = std::make_shared<int>(0);
}

StackLocation VariableScope::add_variable(const std::string& variableName) {
  if (_variableNamesIdxMap.find(variableName) != _variableNamesIdxMap.end())
    THROW("Trying to add same variable in scope=%s", variableName.c_str());
  int index = create_new_variable();
  _variableNamesIdxMap[variableName] = index;
  return StackLocation::local(_moduleIndex, current_function_index(), index);
}

VariableScope* VariableScope::add_scope() {
  auto newScope = new VariableScope(this, false);
  _childrenScopes.push_back(newScope);
  return newScope;
}

VariableScope* VariableScope::add_function_scope() {
  auto newScope = new VariableScope(this, true);
  _childrenScopes.push_back(newScope);
  return newScope;
}

int VariableScope::get_variable_index_in_scope(const std::string& variableName) {
  if (_variableNamesIdxMap.find(variableName) != _variableNamesIdxMap.end())
    return _variableNamesIdxMap[variableName];
  else
    return -1;
}

StackLocation VariableScope::get_variable_location_on_stack(const std::string& variableName) {
  auto scope = this;
  while (scope) {
    int index = scope->get_variable_index_in_scope(variableName);
    if (index != -1) {
      return StackLocation::local(scope->current_module_index(), scope->current_function_index(),
                                  index);
    }
    scope = scope->get_parent();
  }
  return StackLocation::null;
}

VariableScope::~VariableScope() {
  for (auto childScope : _childrenScopes) {
    delete childScope;
  }
}

int VariableScope::create_new_variable() {
  int ret = *_numVariablesStack;
  (*_numVariablesStack)++;
  return ret;
}
