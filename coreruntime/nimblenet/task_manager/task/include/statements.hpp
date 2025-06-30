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

class FunctionDef : public Statement, public std::enable_shared_from_this<FunctionDef> {
  int _moduleIndex;
  int _index;  // unique index assigned to each function in the Task
  Body* _body = nullptr;
  bool _static = false;
  std::string _functionName;
  std::vector<StackLocation> _argumentLocations;
  std::shared_ptr<int> _numVariablesStack;
  std::vector<ASTNode*> _decorators;
  StackLocation _functionLocation = StackLocation::null;
  // ^This is  maintained by VariableScope, we just use it here on execution
  StackLocation _stackLocation{StackLocation::null};

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

class CallStackLockGuard {
  CallStack& _originalStack;
  CallStack _copyStack;

 public:
  CallStackLockGuard(CallStack& originalStack, CallStack& functionStack)
      : _originalStack(originalStack), _copyStack(functionStack) {
    _copyStack.lock = std::move(_originalStack.lock);
  }

  CallStack& get_copy_stack() { return _copyStack; }

  ~CallStackLockGuard() { _originalStack.lock = std::move(_copyStack.lock); }
};

class FunctionDataVariable final : public DataVariable {
  std::shared_ptr<FunctionDef> _def;
  CallStack _stack;

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
    std::string module;
    std::string name;
    StackLocation loc;
  };

  std::vector<ImportObject> _imports;

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
  Body* _body = nullptr;
  ASTNode* _iterator = nullptr;
  ASTNode* _newVar = nullptr;

 public:
  ForStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ForStatement();
};

class WhileStatement : public Statement {
  Body* _body = nullptr;
  ASTNode* _testNode = nullptr;

 public:
  WhileStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~WhileStatement();
};

class IfStatement : public Statement {
  Body* _trueBody = nullptr;
  Body* _elseBody = nullptr;
  ASTNode* _testNode = nullptr;

 public:
  IfStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~IfStatement();
};

class AssertStatement : public Statement {
  ASTNode* _testNode = nullptr;
  ASTNode* _msgNode = nullptr;

 public:
  AssertStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~AssertStatement();
};

class RaiseStatement : public Statement {
  ASTNode* _throwNode = nullptr;

 public:
  RaiseStatement(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~RaiseStatement();
};

class Handler : public Statement {
  Body* _body = nullptr;
  std::optional<std::string> _exceptionType;
  StackLocation _exceptionVariableLocation = StackLocation::null;

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

class ClassDef : public Statement, public std::enable_shared_from_this<ClassDef> {
  StackLocation _classLocation = StackLocation::null;
  std::map<int, StackLocation> _memberIndex2LocationMap;
  std::vector<Statement*> _codeLines;

 public:
  ClassDef(VariableScope* scope, const json& line);

  StatRetType* execute(CallStack& stack) override;

  virtual ~ClassDef() {}
};

class ClassDataVariable final : public DataVariable {
  // map from member index to datavariable
  std::map<int, OpReturnType> _membersMap;

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

class ObjectDataVariable final : public DataVariable {
  OpReturnType _classDataVariable;
  // map from member index to datavariable
  std::map<int, OpReturnType> _membersMap;

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
  std::shared_ptr<ClassDef> _classDef;

 public:
  RuntimeClassDef(VariableScope* scope, const json& line) : Statement(line) {
    _classDef = std::make_shared<ClassDef>(scope, line);
  }

  StatRetType* execute(CallStack& stack) override { return _classDef->execute(stack); }
};

class RuntimeFunctionDef : public Statement {
  std::shared_ptr<FunctionDef> _functionDef;

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
