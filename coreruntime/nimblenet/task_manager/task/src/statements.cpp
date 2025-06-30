#include "statements.hpp"

#include "exception_data_variable.hpp"

AssignStatement::AssignStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto valueBlock = line.at("value");
  _node = ASTNode::create_node(scope, valueBlock);
  auto targetBlock = line.at("targets").at(0);
  _targetOp = ASTNode::create_node(scope, targetBlock);
}

StatRetType* AssignStatement::execute(CallStack& stack) {
  auto ret = _node->get(stack);
  _targetOp->set(ret, stack);
  return nullptr;
}

AssignStatement::~AssignStatement() {
  delete _node;
  delete _targetOp;
}

ExprStatement::ExprStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto valueBlock = line.at("value");
  _node = ASTNode::create_node(scope, valueBlock);
}

StatRetType* ExprStatement::execute(CallStack& stack) {
  _node->get(stack);
  return nullptr;
}

ExprStatement::~ExprStatement() { delete _node; }

ReturnStatement::ReturnStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto valueBlock = line.at("value");

  _node = ASTNode::create_node(scope, valueBlock);
}

StatRetType* ReturnStatement::execute(CallStack& stack) {
  auto d = _node->get(stack);
  return StatRetType::create_return(d);
}

ReturnStatement::~ReturnStatement() { delete _node; }

StatRetType* execute_codelines(CallStack& stack, const std::vector<Statement*> codeLines) {
  for (auto s : codeLines) {
    try {
      auto ret = s->execute(stack);
      if (ret != nullptr) {
        return ret;
      }
    } catch (std::exception& e) {
      THROW("lineNo=%d, %s", s->get_line(), e.what());
    }
  }
  return nullptr;
}

StatRetType* Body::execute(CallStack& stack) { return execute_codelines(stack, _codeLines); }

FunctionDef::FunctionDef(VariableScope* scope, const json& line, StackLocation&& functionLocation)
    : Statement(line) {
  auto inFunctionScope = scope->add_function_scope();
  _numVariablesStack = inFunctionScope->num_variables_stack();
  _index = inFunctionScope->current_function_index();
  _moduleIndex = inFunctionScope->current_module_index();
  auto arguments = line.at("args").at("args");
  _functionName = line.at("name");
  for (auto arg : arguments) {
    std::string argName = arg.at("arg");
    const auto varLocation = inFunctionScope->add_variable(argName);
    _argumentLocations.push_back(varLocation);
  }
  // Add function to scope before evaluating body to support recursive function calls
  _functionLocation = functionLocation;
  auto bodyJson = line.at("body");
  _body = new Body(inFunctionScope, bodyJson);
  if (line.contains("decorator_list")) {
    auto decorators = line.at("decorator_list");
    for (int i = 0; i < decorators.size(); i++) {
      _decorators.push_back(ASTNode::create_node(scope, decorators[i]));
    }
  }
}

StatRetType* FunctionDef::execute(CallStack& stack) {
  auto functionDataVariable = OpReturnType(new FunctionDataVariable(stack, shared_from_this()));
  for (auto& decorator : _decorators) {
    auto val = decorator->get(stack);
    std::vector<OpReturnType> args = {functionDataVariable};
    functionDataVariable = val->execute_function(args, stack);
  }
  stack.set_variable(_functionLocation, functionDataVariable);
  return nullptr;
}

OpReturnType FunctionDef::call_function(const std::vector<OpReturnType>& arguments,
                                        CallStack& stack) {
  // The lock object is set only in the copy constructor, this will make sure that all function
  // calls occur only after copying the stack.
  assert(stack.is_script_lock_created());
  bool lockTaken = false;

  std::unique_ptr<ScopedLock> scopedLock;
  if (!is_static()) {
    // lock needs to be held since function is not static
    scopedLock = stack.scoped_lock_unique_ptr();
  }
  if (arguments.size() != _argumentLocations.size()) {
    THROW("function arguments number not matching %d given %d expected", arguments.size(),
          _argumentLocations.size());
  }
  stack.enter_function_frame(_moduleIndex, _index, *_numVariablesStack);
  for (int i = 0; i < _argumentLocations.size(); i++) {
    stack.set_variable(_argumentLocations[i], arguments[i]);
  }
  auto ret = _body->execute(stack);
  stack.exit_function_frame();
  if (ret == nullptr || ret->type != RETURNTYPE::RETURN) {
    return OpReturnType(new NoneVariable());
  }
  auto retVal = ret->returnVal;
  delete ret;
  return retVal;
}

#define STAT_REGISTER(statementType, StatementClass) \
  {statementType, [](auto scope, auto line) { return new StatementClass(scope, line); }}

static inline Statement* get_statement_from_line(VariableScope* scope, const nlohmann::json& line) {
  static std::map<std::string, std::function<Statement*(VariableScope*, const nlohmann::json&)>>
      statementFactory = {
          STAT_REGISTER("Assign", AssignStatement),
          STAT_REGISTER("ImportFrom", ImportStatement),
          STAT_REGISTER("Expr", ExprStatement),
          {"FunctionDef",
           [](auto scope, auto line) {
             return RuntimeFunctionDef::create_normal_function_def(scope, line);
           }},
          STAT_REGISTER("ClassDef", RuntimeClassDef),
          STAT_REGISTER("Return", ReturnStatement),
          STAT_REGISTER("Break", BreakStatement),
          STAT_REGISTER("Continue", ContinueStatement),
          STAT_REGISTER("For", ForStatement),
          STAT_REGISTER("While", WhileStatement),
          STAT_REGISTER("If", IfStatement),
          STAT_REGISTER("Assert", AssertStatement),
          STAT_REGISTER("Raise", RaiseStatement),
          STAT_REGISTER("Try", TryStatement),
      };
  std::string lineType = line.at("_type");
  if (statementFactory.find(lineType) == statementFactory.end()) {
    THROW("Could not find implementation for Statement=%s at lineNo=%d", lineType.c_str(),
          line.at("lineno").get<int>());
  }
  return statementFactory.at(lineType)(scope, line);
}

Body::Body(VariableScope* scope, const json& body, Statement* initialStatement) {
  if (initialStatement != nullptr) {
    _codeLines.push_back(initialStatement);
  }
  for (auto line : body) {  // This is not a literal a line, this is a code block of the below
                            // type, it denotes a list element from ast.json
    _codeLines.push_back(get_statement_from_line(scope, line));
  }
}

ClassDef::ClassDef(VariableScope* scope, const json& line) : Statement(line) {
  auto bodyJson = line.at("body");
  auto className = line.at("name");
  _classLocation = scope->add_variable(className);
  auto classVariablesScope = scope->add_scope();
  auto functionCreationScope = scope->add_scope();
  for (auto line : bodyJson) {  // This is not a literal a line, this is a code block of the below
                                // type, it denotes a list element from ast.json
    std::string lineType = line.at("_type");
    if (lineType == "FunctionDef") {
      auto statement = RuntimeFunctionDef::create_class_member_function_def(
          classVariablesScope, functionCreationScope, line);
      _codeLines.push_back(statement);
    } else {
      _codeLines.push_back(get_statement_from_line(classVariablesScope, line));
    }
  }

  auto classVariablesToLocationMap = classVariablesScope->get_all_locations_in_scope();
  // We'll store stackLocations for all variables that were defined here.
  for (auto& [memberVariable, location] : classVariablesToLocationMap) {
    auto memberIndex = DataVariable::add_and_get_member_func_index(memberVariable);
    _memberIndex2LocationMap.insert({memberIndex, location});
  }
}

StatRetType* ClassDef::execute(CallStack& stack) {
  auto classDataVariable = OpReturnType(new ClassDataVariable());
  stack.set_variable(_classLocation, classDataVariable);
  execute_codelines(stack, _codeLines);
  for (auto& [memberIndex, location] : _memberIndex2LocationMap) {
    auto value = stack.get_variable(location);
    classDataVariable->set_member(memberIndex, value);
  }
  return nullptr;
}

ClassDataVariable::ClassDataVariable() {}

OpReturnType ClassDataVariable::execute_function(const std::vector<OpReturnType>& arguments,
                                                 CallStack& stack) {
  // this should call constructor
  auto object = std::make_shared<ObjectDataVariable>(shared_from_this());
  if (_membersMap.find(MemberFuncType::CONSTRUCTOR) != _membersMap.end()) {
    // call constructor if exists
    std::vector<OpReturnType> args = arguments;
    args.insert(args.begin(), object);
    _membersMap[MemberFuncType::CONSTRUCTOR]->execute_function(args, stack);
  }
  return object;
}

OpReturnType ClassDataVariable::call_function(int memberFuncIndex,
                                              const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  // this should call static member function of class
  return get_member(memberFuncIndex)->execute_function(arguments, stack);
}

OpReturnType ClassDataVariable::get_member(int memberIndex) {
  if (_membersMap.find(memberIndex) == _membersMap.end()) {
    THROW("Member %s for class does not exist", get_member_func_string(memberIndex));
  }
  return _membersMap[memberIndex];
}

void ClassDataVariable::set_member(int memberIndex, OpReturnType d) {
  _membersMap[memberIndex] = d;
}

ObjectDataVariable::ObjectDataVariable(OpReturnType classDataVariable) {
  _classDataVariable = classDataVariable;
}

OpReturnType ObjectDataVariable::call_function(int memberFuncIndex,
                                               const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  // this should member function of object of class
  // append object to the start of the args of the function
  if (_membersMap.find(memberFuncIndex) == _membersMap.end()) {
    // not in members of object. so it is a member of the class
    //  we need to add self as argument here
    auto newArgs = arguments;
    newArgs.insert(newArgs.begin(), shared_from_this());
    return _classDataVariable->call_function(memberFuncIndex, newArgs, stack);
  }
  // this case is called when in an object I set a function pointer, and then call it
  // this should be a function data variable
  return _membersMap[memberFuncIndex]->execute_function(arguments, stack);
}

OpReturnType ObjectDataVariable::get_member(int memberIndex) {
  if (_membersMap.find(memberIndex) == _membersMap.end()) {
    // member does not exist in object, check static variables
    return _classDataVariable->get_member(memberIndex);
  }
  return _membersMap[memberIndex];
}

void ObjectDataVariable::set_member(int memberIndex, OpReturnType d) {
  _membersMap[memberIndex] = d;
}

AssertStatement::AssertStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto testJson = line.at("test");
  _testNode = ASTNode::create_node(scope, testJson);
  auto msgJson = line.at("msg");
  if (msgJson.type() != json::value_t::null) {
    _msgNode = ASTNode::create_node(scope, msgJson);
  }
}

StatRetType* AssertStatement::execute(CallStack& stack) {
  auto testVal = _testNode->get(stack);
  if (testVal->get_bool()) {
    return nullptr;
  } else {
    // Throw Assertion Error
    if (_msgNode == nullptr) {
      // This is for the case where there is only an assertion with no msg
      THROW("%s", "Assertion failed");
    }
    auto msgVal = _msgNode->get(stack);
    std::string msgString = msgVal->print();
    THROW("Assertion failed with error: %s", msgString.c_str());
  }
}

AssertStatement::~AssertStatement() {
  delete _testNode;
  delete _msgNode;
}

RaiseStatement::RaiseStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto throwJson = line.at("exc");
  _throwNode = ASTNode::create_node(scope, throwJson);
}

StatRetType* RaiseStatement::execute(CallStack& stack) {
  auto throwValue = _throwNode->get(stack);

  if (throwValue->get_dataType_enum() != DATATYPE::EXCEPTION) {
    THROW("Only Exception() can be thrown, but got %s",
          util::get_string_from_enum(throwValue->get_dataType_enum()));
  }

  auto errorMessage = throwValue->print();
  THROW("%s", errorMessage.c_str());
}

RaiseStatement::~RaiseStatement() { delete _throwNode; }

Handler::Handler(VariableScope* scope, const json& line) : Statement(line) {
  auto handlerBodyJson = line.at("body");
  auto exceptionVariableName = line.at("name");
  if (exceptionVariableName != json::value_t::null) {
    std::string exceptionVar = exceptionVariableName;
    // Exception variable ideally should pe created in a newScope in python
    // except Exception as err: # err is created in a new scope
    // TODO: This is a big change, cannot be done now. adding in same scope for now.
    _exceptionVariableLocation = scope->add_variable(exceptionVar);
  }
  auto typeJson = line.at("type");
  if (typeJson != json::value_t::null) {
    _exceptionType = typeJson.at("id");
  }
  _body = new Body(scope, handlerBodyJson);
}

bool Handler::match_expectation_type(const std::string& type) const {
  if (_exceptionType.has_value()) {
    return type == _exceptionType.value();
  }
  // No ExceptionType means all exceptions should be caught
  return true;
}

StatRetType* Handler::catch_exception(CallStack& stack, OpReturnType exception) {
  if (_exceptionVariableLocation != StackLocation::null) {
    stack.set_variable(_exceptionVariableLocation, exception);
  }
  return _body->execute(stack);
}

TryStatement::TryStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto tryJson = line.at("body");
  _tryBody = new Body(scope, tryJson);
  auto handlerJsons = line.at("handlers");
  for (auto& handlerJson : handlerJsons) {
    auto handler = std::make_shared<Handler>(scope, handlerJson);
    _handlers.push_back(handler);
  }
}

StatRetType* TryStatement::execute(CallStack& stack) {
  try {
    return _tryBody->execute(stack);
  } catch (std::exception& e) {
    auto errorMessage = e.what();
    for (auto handler : _handlers) {
      if (handler->match_expectation_type("Exception")) {
        // Currently only handling 'Exception' Types
        return handler->catch_exception(stack,
                                        std::make_shared<ExceptionDataVariable>(errorMessage));
      }
    }
    // no handler for the exception, throw the exception again
    throw e;
  }
}

StatRetType* ImportStatement::execute(CallStack& stack) {
  auto task = stack.task();
  for (const auto& [moduleName, importName, stackLocation] : _imports) {
    // TODO (puneet): remove "nimbleedge" top-level package name after all deployments are updated
    if ((moduleName == "delitepy") || (moduleName == "nimbleedge")) {
      if (importName == "nimblenet") {
        stack.set_variable(stackLocation,
                           OpReturnType(new NimbleNetDataVariable(stack.command_center())));
      } else if (importName == "nimblenetInternalTesting") {
        stack.set_variable(stackLocation,
                           OpReturnType(new NimbleNetInternalDataVariable(stack.command_center())));
      }
#ifdef REGEX_ENABLED
      else if (importName == "ne_re") {
        stack.set_variable(stackLocation, OpReturnType(new RegexDataVariable()));
      }
#endif
      else {
        THROW("Cannot import=%s from module=%s at lineno=%d", importName.c_str(),
              moduleName.c_str(), get_line());
      }
    } else {
      if (!task->has_module(moduleName)) {
        THROW("Cannot import module=%s at lineno=%d: Module not found", moduleName.c_str(),
              get_line());
      }
      auto module = task->get_module(moduleName, stack);
      if (!module->has_variable(importName)) {
        THROW("Cannot import=%s from module=%s at lineno=%d: import not found in module",
              importName.c_str(), moduleName.c_str(), get_line());
      }
      auto loc = module->get_variable_location(importName);
      stack.set_variable(stackLocation, stack.get_variable(loc));
    }
  }
  return nullptr;
}

InbuiltFunctionsStatement::InbuiltFunctionsStatement(VariableScope* scope) {
  for (auto it = CustomFunctions::_customFuncMap.begin();
       it != CustomFunctions::_customFuncMap.end(); ++it) {
    _locations.push_back(scope->add_variable(it->first));
  }
}

StatRetType* InbuiltFunctionsStatement::execute(CallStack& stack) {
  int i = 0;
  for (auto it = CustomFunctions::_customFuncMap.begin();
       it != CustomFunctions::_customFuncMap.end(); ++it) {
    stack.set_variable(_locations[i++], OpReturnType(new CustomFuncDataVariable(it->second)));
  }
  return nullptr;
}

StatRetType* ForStatement::execute(CallStack& stack) {
  auto iteratorVal = _iterator->get(stack);
  int size = iteratorVal->get_size();
  for (int i = 0; i < size; i++) {
    auto d = iteratorVal->get_int_subscript(i);
    _newVar->set_variable(d, stack);
    auto ret = _body->execute(stack);
    // there might be elements getting added or deleted inside body->execute, so changing the
    // iteration size
    size = iteratorVal->get_size();
    if (ret != nullptr) {
      if (ret->type == RETURNTYPE::BREAK) {
        delete ret;
        break;
      } else if (ret->type == RETURNTYPE::CONTINUE) {
        delete ret;
        continue;
      } else if (ret->type == RETURNTYPE::RETURN) {
        return ret;
      }
    }
  }
  return nullptr;
}

ForStatement::ForStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto forLoopScope = scope->add_scope();
  auto newVarJson = line.at("target");
  _newVar = ASTNode::create_node(forLoopScope, newVarJson);
  auto iterJson = line.at("iter");
  _iterator = ASTNode::create_node(forLoopScope, iterJson);
  auto bodyJson = line.at("body");
  _body = new Body(forLoopScope, bodyJson);
}

ForStatement::~ForStatement() {
  delete _body;
  delete _iterator;
  delete _newVar;
}

WhileStatement::WhileStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto whileLoopScope = scope->add_scope();
  auto testJson = line.at("test");
  _testNode = ASTNode::create_node(scope, testJson);
  auto bodyJson = line.at("body");
  _body = new Body(whileLoopScope, bodyJson);
}

StatRetType* WhileStatement::execute(CallStack& stack) {
  while (_testNode->get(stack)->get_bool()) {
    auto ret = _body->execute(stack);
    if (ret != nullptr) {
      if (ret->type == RETURNTYPE::BREAK) {
        delete ret;
        break;
      } else if (ret->type == RETURNTYPE::CONTINUE) {
        delete ret;
        continue;
      } else if (ret->type == RETURNTYPE::RETURN) {
        return ret;
      }
    }
  }

  return nullptr;
}

WhileStatement::~WhileStatement() {
  delete _body;
  delete _testNode;
}

IfStatement::IfStatement(VariableScope* scope, const json& line) : Statement(line) {
  auto testJson = line.at("test");
  _testNode = ASTNode::create_node(scope, testJson);
  auto trueScope = scope->add_scope();
  auto trueBodyJson = line.at("body");
  _trueBody = new Body(trueScope, trueBodyJson);
  auto elseBodyJson = line.at("orelse");
  auto elseScope = scope->add_scope();
  _elseBody = new Body(elseScope, elseBodyJson);
}

StatRetType* IfStatement::execute(CallStack& stack) {
  auto testVal = _testNode->get(stack);
  if (testVal->get_bool()) {
    return _trueBody->execute(stack);
  } else {
    return _elseBody->execute(stack);
  }
}

IfStatement::~IfStatement() {
  delete _trueBody;
  delete _elseBody;
  delete _testNode;
}
