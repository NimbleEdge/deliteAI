/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <shared_mutex>

#include "data_variable.hpp"
#include "ne_fwd.hpp"

#ifdef GENAI
#include "llm_data_variable.hpp"
#endif  // GENAI

class ImportStatement;
class FunctionDef;
class VariableScope;
class CallStack;
class Task;

/**
 * @brief Represents a location in the runtime stack for variable access
 *
 * This class encapsulates the three-part index needed to locate a variable
 * in the runtime stack: module index, function index, and variable index.
 * It prevents confusion between runtime stack indices and compile-time scope indices.
 */
class StackLocation {
  int _moduleIndex;    /**< Index of the module in the task */
  int _functionIndex;  /**< Index of the function within the module */
  int _varIndex;       /**< Index of the variable within the function's stack frame */

  constexpr StackLocation(int moduleIndex, int functionIndex, int varIndex)
      : _moduleIndex(moduleIndex), _functionIndex(functionIndex), _varIndex(varIndex) {}
  friend class CallStack;
  friend class VariableScope;
  // helper functions to create a local or global location on the stack
  static StackLocation local(int moduleIndex, int functionIndex, int index);

 public:
  bool operator==(const StackLocation& o) const noexcept {
    return _moduleIndex == o._moduleIndex && _functionIndex == o._functionIndex &&
           _varIndex == o._varIndex;
  }

  bool operator!=(const StackLocation& o) const noexcept { return !(*this == o); }

  static const StackLocation null;
};

inline constexpr StackLocation StackLocation::null = {-1, -1, 0};

class CallStack;

/**
 * @brief Represents a single stack frame for function execution
 *
 * Each stack frame contains the local variables for a function call,
 * along with metadata about the module and function being executed.
 * Thread-safe access to variable values is provided through mutex protection.
 */
class StackFrame {
  using StackFramePtr = std::shared_ptr<StackFrame>;

  std::vector<OpReturnType> _varValues;  /**< Storage for variable values in this frame */
  // StackFramePtr _parentFrame;
  int _moduleIndex;    /**< Index of the module this frame belongs to */
  int _functionIndex;  /**< Index of the function this frame represents */
  std::mutex mutex;    /**< Mutex for thread-safe access to variable values */

 public:
  int get_module_index() const { return _moduleIndex; }

  int get_function_index() const { return _functionIndex; }

  StackFrame(int moduleIndex, int functionIndex, int numVariables)
      : _moduleIndex(moduleIndex), _functionIndex(functionIndex), _varValues(numVariables) {}

  OpReturnType get(int varIndex) {
    std::lock_guard<std::mutex> locker(mutex);
    assert(_varValues.size() > varIndex);
    return _varValues[varIndex];
  }

  void set(int varIndex, OpReturnType val) {
    std::lock_guard<std::mutex> locker(mutex);
    assert(_varValues.size() > varIndex);
    _varValues[varIndex] = val;
  }
};

/**
 * @brief RAII wrapper for acquiring a shared mutex lock
 *
 * Automatically acquires the lock if not already held and releases it
 * when the object goes out of scope.
 */
class ScopedLock {
  std::unique_lock<std::shared_mutex>& lock;  /**< Reference to the lock to manage */
  bool lockedByMe = false;                    /**< Whether this object acquired the lock */

 public:
  ScopedLock(std::unique_lock<std::shared_mutex>& l) : lock(l) {
    assert(lock.mutex());
    if (!lock.owns_lock()) {
      // does not own lock
      lockedByMe = true;
      lock.lock();
    }
  }

  ScopedLock(const ScopedLock& l) = delete;
  ScopedLock& operator=(const ScopedLock& other) = delete;

  ~ScopedLock() {
    if (lockedByMe) {
      lock.unlock();
    }
  }
};

/**
 * @brief RAII wrapper for temporarily releasing a shared mutex lock
 *
 * Temporarily unlocks the mutex if it's currently held and re-acquires it
 * when the object goes out of scope.
 */
class ScopedUnlock {
  std::unique_lock<std::shared_mutex>& lock;  /**< Reference to the lock to manage */
  bool unlockedByMe = false;                  /**< Whether this object released the lock */

 public:
  ScopedUnlock(std::unique_lock<std::shared_mutex>& l) : lock(l) {
    assert(bool(lock.mutex()));
    if (lock.owns_lock()) {
      // does not own lock
      unlockedByMe = true;
      lock.unlock();
    }
  }

  ScopedUnlock(const ScopedUnlock& l) = delete;
  ScopedUnlock& operator=(const ScopedUnlock& other) = delete;

  ~ScopedUnlock() {
    if (unlockedByMe) {
      lock.lock();
    }
  }
};

/**
 * @brief Manages the runtime call stack for task execution
 *
 * The CallStack maintains the execution context for a task, including
 * all active function calls and their local variables. It provides
 * thread-safe access to variables through stack locations and manages
 * the lifecycle of stack frames.
 */
class CallStack {
  // copy and copy assignment is overriden for this class, if any other change made to
  // this class, make sure to update copy assignment operator

  using StackFramePtr = std::shared_ptr<StackFrame>;

  std::vector<StackFramePtr> _functionsStack;  /**< Current call stack of active functions */
  std::vector<std::vector<std::vector<StackFramePtr>>> _moduleToStackFrameMap;  /**< 3D map: module -> function -> stack frames */

  CommandCenter* _commandCenter = nullptr;  /**< Reference to the command center for task access */

  auto command_center() noexcept { return _commandCenter; }
  friend class ImportStatement;
  friend class CallStackLockGuard;
  friend class CustomFunctions;
  friend class FunctionDataVariable;
#ifdef GENAI
  friend class LLMDataVariable;
#endif  // GENAI
  CallStack(const CallStack& other);
  std::unique_lock<std::shared_mutex> lock;  /**< Shared mutex for thread-safe operations */

 public:
  CallStack(CommandCenter* commandCenter) : _commandCenter(commandCenter) {}

  OpReturnType get_variable(StackLocation loc) const;
  void set_variable(StackLocation loc, OpReturnType val);

  CallStack& operator=(const CallStack& other);

  CallStack create_copy_with_deferred_lock();

  ScopedUnlock scoped_unlock() { return ScopedUnlock(lock); }

  ScopedLock scoped_lock() { return ScopedLock(lock); }

  std::unique_ptr<ScopedLock> scoped_lock_unique_ptr() {
    return std::make_unique<ScopedLock>(lock);
  }

  bool is_script_lock_created() const { return lock.mutex(); }

  void enter_function_frame(int moduleIndex, int functionIndex, int numVariablesInFrame);
  void exit_function_frame();

  std::shared_ptr<Task> task() noexcept;
};

/**
 * @brief Manages variable scoping and lifetime during compilation and execution
 *
 * VariableScope tracks variable declarations and their stack locations during
 * the compilation phase. It maintains a hierarchical structure of scopes
 * (global, function, local) and provides mapping from variable names to
 * their runtime stack locations.
 */
class VariableScope {
  CommandCenter* _commandCenter = nullptr;  /**< Reference to the command center */
  int _moduleIndex;                         /**< Index of the module this scope belongs to */
  std::vector<VariableScope*> _childrenScopes;  /**< Child scopes created from this scope */
  VariableScope* _parentScope = nullptr;        /**< Parent scope in the hierarchy */
  // std::vector<OpReturnType> _variableValues;
  std::map<std::string, int> _variableNamesIdxMap;  /**< Maps variable names to their indices in this scope */

  // Index to assign to the next function encountered
  std::shared_ptr<int> _nextFunctionIndex;  /**< Shared counter for assigning unique function indices */
  // Index of the function defined in the nearest parent scope
  int _currentFunctionIndex;  /**< Index of the function this scope belongs to */

  // A scope belongs to a stack frame, but a stack frame can have multiple scopes. This
  // is shared across the multiple scopes of a stack and is used to keep count
  // of the number of variables a stack frame has and assign indices appropriately
  std::shared_ptr<int> _numVariablesStack;  /**< Shared counter for variables in the stack frame */

  VariableScope(VariableScope* p, bool isNewFunction);

  int get_variable_index_in_scope(const std::string& variableName);

  CommandCenter* get_commandCenter() { return _commandCenter; }

  friend Task;

 public:
  auto get_all_locations_in_scope() const {
    std::map<std::string, StackLocation> locationMap;
    for (auto& [variableName, index] : _variableNamesIdxMap) {
      locationMap.insert(
          {variableName, StackLocation::local(_moduleIndex, _currentFunctionIndex, index)});
    }
    return locationMap;
  }

  VariableScope(CommandCenter* commandCenter, int moduleIndex);

  VariableScope* get_parent() { return _parentScope; }

  StackLocation add_variable(const std::string& variableName);

  VariableScope* add_scope();
  // Returns new scope and a shared pointer to number of variables in function's stack frame
  VariableScope* add_function_scope();

  StackLocation get_variable_location_on_stack(const std::string& variableName);

  auto current_module_index() const noexcept { return _moduleIndex; }

  auto current_function_index() const noexcept { return _currentFunctionIndex; }

  // Returns a shared pointer so that this particular information can be stored by the function
  auto num_variables_stack() const noexcept { return _numVariablesStack; }

  // OpReturnType get_variable(int index) { return _variableValues[index]; }
  // void set_variable(int index, OpReturnType d) { _variableValues[index] = d; }
  /// number of variables stored in this scope
  auto num_variables() const noexcept { return _variableNamesIdxMap.size(); }

  int create_new_variable();

  friend class ImportStatement;
  friend class DecoratorStatement;

  ~VariableScope();
};
