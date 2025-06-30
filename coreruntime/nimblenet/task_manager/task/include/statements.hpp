/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "json.hpp"
#include "nimble_net_data_variable.hpp"
#include "nimble_net_internal_data_variable.hpp"
#include "node.hpp"
#include "regex_data_variable.hpp"

class VariableScope;

enum RETURNTYPE {
  BREAK = 1,
  CONTINUE = 2,
  RETURN = 3,
};

enum DECORATOR_TYPE { ADD_EVENT_HOOK, CONCURRENT_METHOD, PRE_ADD_EVENT_HOOK };

static std::unordered_map<std::string, DECORATOR_TYPE> _decoratorNameMap = {
    {"add_event", DECORATOR_TYPE::ADD_EVENT_HOOK},
    {"concurrent", DECORATOR_TYPE::CONCURRENT_METHOD},
    {"pre_add_event", DECORATOR_TYPE::PRE_ADD_EVENT_HOOK}};

class StatRetType {
  StatRetType(int t, OpReturnType d) {
    type = t;
    returnVal = d;
  }

 public:
  OpReturnType returnVal = nullptr;
  int type = -1;

  static StatRetType* create_return(OpReturnType d) {
    return new StatRetType(RETURNTYPE::RETURN, d);
  }

  static StatRetType* create_break() { return new StatRetType(RETURNTYPE::BREAK, nullptr); }

  static StatRetType* create_continue() { return new StatRetType(RETURNTYPE::CONTINUE, nullptr); }
};

class Statement {
  Statement() { _lineNo = 0; }
  friend class InbuiltFunctionsStatement;

 protected:
  int _lineNo = -1;

 public:
  virtual ~Statement() = default;

  Statement(const json& line) { _lineNo = line.at("lineno"); }

  int get_line() { return _lineNo; }

  virtual StatRetType* execute(CallStack& stack) = 0;
};

/*
DELITEPY_DOC_BLOCK_BEGIN

## Assign statement
<del>Works as in Python.</del>
DELITEPY_DOC_BLOCK_END
*/
class AssignStatement : public Statement {
  ASTNode* _node = nullptr;      // Value node
  ASTNode* _targetOp = nullptr;  // Target node

 public:
  AssignStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~AssignStatement();
};

/*
DELITEPY_DOC_BLOCK_BEGIN

## Expression statement
<del>Works as in Python.</del>
DELITEPY_DOC_BLOCK_END
*/
class ExprStatement : public Statement {
  ASTNode* _node = nullptr;

 public:
  ExprStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ExprStatement();
};

/*
DELITEPY_DOC_BLOCK_BEGIN

## Return statement
<del>Works as in Python.</del>
DELITEPY_DOC_BLOCK_END
*/
class ReturnStatement : public Statement {
  ASTNode* _node = nullptr;

 public:
  ReturnStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ReturnStatement();
};

class BreakStatement : public Statement {
 public:
  BreakStatement(VariableScope* scope, const json& line) : Statement(line) {}

  StatRetType* execute(CallStack& stack) override { return StatRetType::create_break(); };
};

class ContinueStatement : public Statement {
 public:
  ContinueStatement(VariableScope* scope, const json& line) : Statement(line) {}

  StatRetType* execute(CallStack& stack) override { return StatRetType::create_continue(); };
};

class Body {
  std::vector<Statement*> _codeLines;

 public:
  Body(VariableScope* scope, const json& body, Statement* initialStatement = nullptr);

  StatRetType* execute(CallStack& stack);

  ~Body() {
    for (auto line : _codeLines) {
      delete line;
    }
  }
};

/**
 * @brief Represents a function definition in the AST
 *
 * FunctionDef encapsulates a Python function definition, including its body,
 * arguments, decorators, and execution context. It manages the function's
 * scope, argument locations, and provides the interface for function calls.
 */
class FunctionDef : public Statement, public std::enable_shared_from_this<FunctionDef> {
  int _moduleIndex;  /**< Index of the module containing this function */
  int _index;        /**< Unique index assigned to each function in the Task */
  Body* _body = nullptr;  /**< The function's body containing all statements */
  bool _static = false;   /**< Whether this function is static */
  std::string _functionName;  /**< Name of the function */
  std::vector<StackLocation> _argumentLocations;  /**< Stack locations of function arguments */
  std::shared_ptr<int> _numVariablesStack;  /**< Shared counter for variables in function's stack frame */
  std::vector<ASTNode*> _decorators;  /**< Function decorators */
  StackLocation _functionLocation = StackLocation::null;  /**< Location of the function in the stack */
  // ^This is  maintained by VariableScope, we just use it here on execution
  StackLocation _stackLocation{StackLocation::null};  /**< Stack location for the function itself */

  void set_static() { _static = true; }

  friend class FunctionDataVariable;

 public:
  FunctionDef(VariableScope* scope, const json& line, StackLocation&& functionLocation);
  OpReturnType call_function(const std::vector<OpReturnType>& arguments, CallStack& stack);

  StatRetType* execute(CallStack& stack) override;

  std::string get_function_name() const { return _functionName; }

  int get_num_arguments() const { return _argumentLocations.size(); }

  auto index() const noexcept { return _index; }

  bool is_static() const { return _static; }

  virtual ~FunctionDef() { delete _body; }
};

/**
 * @brief RAII wrapper for managing call stack locks during function execution
 *
 * CallStackLockGuard ensures proper lock management when executing functions
 * by temporarily transferring the lock from the original stack to a copy stack
 * and restoring it when the guard goes out of scope.
 */
class CallStackLockGuard {
  CallStack& _originalStack;  /**< Reference to the original call stack */
  CallStack _copyStack;       /**< Copy of the call stack for function execution */

 public:
  CallStackLockGuard(CallStack& originalStack, CallStack& functionStack)
      : _originalStack(originalStack), _copyStack(functionStack) {
    _copyStack.lock = std::move(_originalStack.lock);
  }

  CallStack& get_copy_stack() { return _copyStack; }

  ~CallStackLockGuard() { _originalStack.lock = std::move(_copyStack.lock); }
};

/**
 * @brief Data variable representing a function object
 *
 * FunctionDataVariable wraps a FunctionDef and provides the interface for
 * calling functions. It manages the execution context and ensures proper
 * lock handling during function calls.
 */
class FunctionDataVariable final : public DataVariable {
  std::shared_ptr<FunctionDef> _def;  /**< The function definition */
  CallStack _stack;                   /**< Call stack for function execution */

  int get_dataType_enum() const final { return DATATYPE::FUNCTION; }

  int get_containerType() const final { return CONTAINERTYPE::FUNCTIONDEF; }

  bool get_bool() final { return true; }

  nlohmann::json to_json() const override { return "[Function]"; }

  void set_static() { _def->set_static(); }
  friend class CustomFunctions;

 public:
  FunctionDataVariable(CallStack& stack, std::shared_ptr<FunctionDef> def) : _stack(stack) {
    _def = def;
  }

  OpReturnType execute_function(const std::vector<OpReturnType>& arguments,
                                CallStack& stack) override {
    CallStackLockGuard guard(stack, _stack);
    auto ret = _def->call_function(arguments, guard.get_copy_stack());
    return ret;
  }

  OpReturnType execute_function(const std::vector<OpReturnType>& arguments) override {
    CallStack copyStack = _stack.create_copy_with_deferred_lock();
    auto ret = _def->call_function(arguments, copyStack);
    return ret;
  }

  std::string print() override { return fallback_print(); }
};

class ImportStatement : public Statement {
  // Example:
  // from moduleA import get_A as ga
  //
  // module = moduleA
  // name = get_A
  struct ImportObject {
    std::string module;  /**< Name of the module being imported from */
    std::string name;    /**< Name of the object being imported */
    StackLocation loc;   /**< Stack location where the imported object is stored */
  };

  std::vector<ImportObject> _imports;  /**< List of import objects */

 public:
  ImportStatement(VariableScope* scope, const json& line) : Statement(line) {
    auto module = line.at("module");
    auto nameJsonArray = line.at("names");
    for (auto nameJson : nameJsonArray) {
      std::string importName = nameJson.at("name");
      std::string varName = importName;
      auto aliasName = nameJson.at("asname");
      if (aliasName.type() != json::value_t::null) {
        varName = aliasName;
      }
      const auto stackLocation = scope->add_variable(varName);
      _imports.push_back({module, importName, stackLocation});
    }
  }

  StatRetType* execute(CallStack& stack) override;
};

class ForStatement : public Statement {
  Body* _body = nullptr;      /**< Body of the for loop */
  ASTNode* _iterator = nullptr;  /**< The iterable being looped over */
  ASTNode* _newVar = nullptr;    /**< Target variable for assignment */

 public:
  ForStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ForStatement();
};

class WhileStatement : public Statement {
  Body* _body = nullptr;     /**< Body of the while loop */
  ASTNode* _testNode = nullptr;  /**< Condition expression for the loop */

 public:
  WhileStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~WhileStatement();
};

class IfStatement : public Statement {
  Body* _trueBody = nullptr;   /**< Body executed when condition is true */
  Body* _elseBody = nullptr;   /**< Body executed when condition is false */
  ASTNode* _testNode = nullptr;  /**< Condition expression */

 public:
  IfStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~IfStatement();
};

class AssertStatement : public Statement {
  ASTNode* _testNode = nullptr;  /**< Expression to assert */
  ASTNode* _msgNode = nullptr;   /**< Optional error message */

 public:
  AssertStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~AssertStatement();
};

class RaiseStatement : public Statement {
  ASTNode* _throwNode = nullptr;  /**< Exception object to raise */

 public:
  RaiseStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~RaiseStatement();
};

class Handler : public Statement {
  Body* _body = nullptr;  /**< Body of the exception handler */
  std::optional<std::string> _exceptionType;  /**< Type of exception to catch */
  StackLocation _exceptionVariableLocation = StackLocation::null;  /**< Location to store the exception */

 public:
  Handler(VariableScope* scope, const json& line);

  StatRetType* catch_exception(CallStack& stack, OpReturnType exception);

  StatRetType* execute(CallStack& stack) override {
    THROW("%s", "Should not be called");
    return nullptr;
  }

  bool match_expectation_type(const std::string& type) const;

  virtual ~Handler() { delete _body; }
};

class TryStatement : public Statement {
  Body* _tryBody = nullptr;
  std::vector<std::shared_ptr<Handler>> _handlers;

 public:
  TryStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~TryStatement() { delete _tryBody; }
};

class InbuiltFunctionsStatement : public Statement {
  std::vector<StackLocation> _locations;

 public:
  InbuiltFunctionsStatement(VariableScope* scope);

  StatRetType* execute(CallStack& stack) override;
};

/**
 * @brief Represents a class definition in the AST
 *
 * ClassDef encapsulates a Python class definition, including its body,
 * member variables, and methods. It manages the class's scope and provides
 * the interface for class instantiation and member access.
 */
class ClassDef : public Statement, public std::enable_shared_from_this<ClassDef> {
  StackLocation _classLocation = StackLocation::null;  /**< Location of the class in the stack */
  std::map<int, StackLocation> _memberIndex2LocationMap;  /**< Maps member indices to their stack locations */
  std::vector<Statement*> _codeLines;  /**< Statements in the class body */

 public:
  ClassDef(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ClassDef() {}
};

/**
 * @brief Data variable representing a class object
 *
 * ClassDataVariable represents the class itself (not an instance) and
 * provides access to class methods and static members. It serves as a
 * template for creating class instances.
 */
class ClassDataVariable final : public DataVariable {
  // map from member index to datavariable
  std::map<int, OpReturnType> _membersMap;  /**< Map of member indices to their values */

  int get_dataType_enum() const final { return DATATYPE::NONE; }

  int get_containerType() const final { return CONTAINERTYPE::CLASS; }

  bool get_bool() final { return true; }

  nlohmann::json to_json() const override { return "[Class]"; }

 public:
  ClassDataVariable();

  OpReturnType execute_function(const std::vector<OpReturnType>& arguments,
                                CallStack& stack) override;

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  std::string print() override { return fallback_print(); }

  OpReturnType get_member(int memberIndex) override;

  void set_member(int memberIndex, OpReturnType d) override;
};

/**
 * @brief Data variable representing a class instance
 *
 * ObjectDataVariable represents an instance of a class and provides
 * access to instance methods and member variables. Each instance has
 * its own copy of member variables.
 */
class ObjectDataVariable final : public DataVariable {
  OpReturnType _classDataVariable;  /**< Reference to the class definition */
  // map from member index to datavariable
  std::map<int, OpReturnType> _membersMap;  /**< Map of member indices to their values */

  int get_dataType_enum() const final { return DATATYPE::NONE; }

  int get_containerType() const final { return CONTAINERTYPE::CLASS; }

  bool get_bool() final { return true; }

  nlohmann::json to_json() const override { return "[ClassObject]"; }

 public:
  ObjectDataVariable(OpReturnType classDataVariable);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  std::string print() override { return fallback_print(); }

  OpReturnType get_member(int memberIndex) override;

  void set_member(int memberIndex, OpReturnType d) override;
};

/*
 * Helper statement class whose only job is to insert the function variable into the stack, which
 * helps in having first class function support
 */
class RuntimeClassDef : public Statement {
  std::shared_ptr<ClassDef> _classDef;  /**< The class definition to execute */

 public:
  RuntimeClassDef(VariableScope* scope, const json& line) : Statement(line) {
    _classDef = std::make_shared<ClassDef>(scope, line);
  }

  StatRetType* execute(CallStack& stack) override { return _classDef->execute(stack); }
};

/**
 * @brief Helper statement for runtime function definition
 *
 * RuntimeFunctionDef is a wrapper around FunctionDef that handles the
 * creation and insertion of function variables into the stack at runtime.
 * This enables first-class function support in the language.
 */
class RuntimeFunctionDef : public Statement {
  std::shared_ptr<FunctionDef> _functionDef;  /**< The function definition to execute */

  RuntimeFunctionDef(VariableScope* scope, const json& line, StackLocation&& functionLocation)
      : Statement(line) {
    _functionDef = std::make_shared<FunctionDef>(scope, line, std::move(functionLocation));
  }

 public:
  static RuntimeFunctionDef* create_class_member_function_def(VariableScope* classVariablesScope,
                                                              VariableScope* functionCreationScope,
                                                              const json& line) {
    auto funcName = line.at("name");
    auto location = classVariablesScope->add_variable(funcName);
    return new RuntimeFunctionDef(functionCreationScope, line, std::move(location));
  }

  static RuntimeFunctionDef* create_normal_function_def(VariableScope* scope, const json& line) {
    auto funcName = line.at("name");
    auto location = scope->add_variable(funcName);
    return new RuntimeFunctionDef(scope, line, std::move(location));
  }

  StatRetType* execute(CallStack& stack) override { return _functionDef->execute(stack); }
};
