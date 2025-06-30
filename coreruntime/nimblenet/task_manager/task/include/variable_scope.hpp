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

// index into the Runtime stack. It is a class so we don't get confused whether something
// indexes into the runtime stack or the compile time scope
class StackLocation {
  int _moduleIndex;
  int _functionIndex;
  int _varIndex;

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

class StackFrame {
  using StackFramePtr = std::shared_ptr<StackFrame>;

  std::vector<OpReturnType> _varValues;
  // StackFramePtr _parentFrame;
  int _moduleIndex;
  int _functionIndex;
  std::mutex mutex;

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

class ScopedLock {
  std::unique_lock<std::shared_mutex>& lock;
  bool lockedByMe = false;

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

class ScopedUnlock {
  std::unique_lock<std::shared_mutex>& lock;
  bool unlockedByMe = false;

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

class CallStack {
  // copy and copy assignment is overriden for this class, if any other change made to
  // this class, make sure to update copy assignment operator

  using StackFramePtr = std::shared_ptr<StackFrame>;

  std::vector<StackFramePtr> _functionsStack;
  std::vector<std::vector<std::vector<StackFramePtr>>> _moduleToStackFrameMap;

  CommandCenter* _commandCenter = nullptr;

  auto command_center() noexcept { return _commandCenter; }
  friend class ImportStatement;
  friend class CallStackLockGuard;
  friend class CustomFunctions;
  friend class FunctionDataVariable;
#ifdef GENAI
  friend class LLMDataVariable;
#endif  // GENAI
  CallStack(const CallStack& other);
  std::unique_lock<std::shared_mutex> lock;

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

class VariableScope {
  CommandCenter* _commandCenter = nullptr;
  int _moduleIndex;
  std::vector<VariableScope*> _childrenScopes;
  VariableScope* _parentScope = nullptr;
  // std::vector<OpReturnType> _variableValues;
  std::map<std::string, int> _variableNamesIdxMap;

  // Index to assign to the next function encountered
  std::shared_ptr<int> _nextFunctionIndex;
  // Index of the function defined in the nearest parent scope
  int _currentFunctionIndex;

  // A scope belongs to a stack frame, but a stack frame can have multiple scopes. This
  // is shared across the multiple scopes of a stack and is used to keep count
  // of the number of variables a stack frame has and assign indices appropriately
  std::shared_ptr<int> _numVariablesStack;

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
