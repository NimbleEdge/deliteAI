#include "node.hpp"

#include "binary_operators.hpp"
#include "single_variable.hpp"
#include "statements.hpp"

ConstantNode::ConstantNode(VariableScope* scope, const json& constJson)
    : ASTNode(scope, constJson) {
  auto value = constJson.at("value");
  _d = DataVariable::get_SingleVariableFrom_JSON(value);
}

BinNode::BinNode(VariableScope* scope, const json& binOpJson) : ASTNode(scope, binOpJson) {
  auto leftBlock = binOpJson.at("left");
  _left = ASTNode::create_node(scope, leftBlock);
  auto rightBlock = binOpJson.at("right");
  _right = ASTNode::create_node(scope, rightBlock);
  _opType = binOpJson.at("op").at("_type");
}

OpReturnType BinNode::get_value(CallStack& stack) {
  auto d1 = _left->get(stack);
  auto d2 = _right->get(stack);
  auto ret = BinaryOperators::operate(d1, d2, _opType);
  if (ret == nullptr) {
    auto enum1 = util::get_string_from_enum(d1->get_dataType_enum());
    auto enum2 = util::get_string_from_enum(d2->get_dataType_enum());
    throw create_exception("Could not %s, check types left=%s(%s), right=%s(%s)", _opType.c_str(),
                           d1->get_containerType_string(), enum1, d2->get_containerType_string(),
                           enum2);
  }
  return ret;
}

UnaryNode::UnaryNode(VariableScope* scope, const json& unaryOpJson) : ASTNode(scope, unaryOpJson) {
  auto operandBlock = unaryOpJson.at("operand");
  _operand = ASTNode::create_node(scope, operandBlock);
  _opType = unaryOpJson.at("op").at("_type");
  _func = UnaryOperators::get_operator(_opType);
}

OpReturnType UnaryNode::get_value(CallStack& stack) {
  auto d = _operand->get(stack);
  auto ret = _func(d);
  if (ret == nullptr) {
    auto enumString = util::get_string_from_enum(d->get_dataType_enum());
    throw create_exception("Could not %s, check types operand=%s[%s]", _opType.c_str(), enumString,
                           d->get_containerType_string());
  }
  return ret;
}

CompareNode::CompareNode(VariableScope* scope, const json& j) {
  auto comparatorsJson = j.at("comparators");
  for (auto singleCompar : comparatorsJson) {
    _comparators.push_back(create_node(scope, singleCompar));
  }
  auto leftJson = j.at("left");
  _left = create_node(scope, leftJson);
  auto compareFuncJsons = j.at("ops");
  for (auto singleFunc : compareFuncJsons) {
    std::string type = singleFunc.at("_type");
    _opTypes.push_back(type);
    _compareFuncs.push_back(CompareOperators::get_operator(type));
  }
  if (_comparators.size() != _compareFuncs.size()) {
    THROW("No. of operands=%d not equal to no. of comparators=%d", _comparators.size(),
          _compareFuncs.size());
  }
}

OpReturnType CompareNode::get_value(CallStack& stack) {
  auto d = _left->get(stack);
  OpReturnType ret = nullptr;
  for (int i = 0; i < _comparators.size(); i++) {
    auto d2 = _comparators[i]->get(stack);
    ret = _compareFuncs[i](d, d2);
    if (ret == nullptr) {
      auto enumString1 = util::get_string_from_enum(d->get_dataType_enum());
      auto enumString2 = util::get_string_from_enum(d2->get_dataType_enum());
      throw create_exception("Could not %s, check types left=%s[%s], right=%s[%s]",
                             _opTypes[i].c_str(), enumString1, d->get_containerType_string(),
                             enumString2, d2->get_containerType_string());
    }
    if (!ret->get_bool()) {
      return ret;
    }
    d = d2;
  }
  return ret;
}

BoolNode::BoolNode(VariableScope* scope, const json& j) {
  _opType = j.at("op").at("_type");
  _func = BoolOperators::get_operator(_opType);

  for (auto compareJson : j.at("values")) {
    _comparators.push_back(create_node(scope, compareJson));
  }
}

OpReturnType BoolNode::get_value(CallStack& stack) {
  auto left = _comparators[0]->get(stack);
  OpReturnType ret = nullptr;
  for (int i = 1; i < _comparators.size(); i++) {
    if (_opType == "And" && !left->get_bool()) {
      return OpReturnType(new SingleVariable<bool>(false));
    }
    if (_opType == "Or" && left->get_bool()) {
      return OpReturnType(new SingleVariable<bool>(true));
    }
    auto right = _comparators[i]->get(stack);
    ret = _func(left, right);
    if (ret == nullptr) {
      auto enumString1 = util::get_string_from_enum(left->get_dataType_enum());
      auto enumString2 = util::get_string_from_enum(right->get_dataType_enum());
      throw create_exception("Could not %s, check types left=%s[%s], right=%s[%s]", _opType.c_str(),
                             enumString1, left->get_containerType_string(), enumString2,
                             right->get_containerType_string());
    }
    left = right;
  }

  return ret;
}

CallNode::CallNode(VariableScope* scope, const json& callFuncJson) : ASTNode(scope, callFuncJson) {
  auto args = callFuncJson.at("args");
  auto funcNodeJson = callFuncJson.at("func");
  _functionNode = ASTNode::create_node(scope, funcNodeJson);
  for (auto arg : args) {
    _arguments.push_back(ASTNode::create_node(scope, arg));
  }
}

OpReturnType CallNode::get_value(CallStack& stack) {
  std::vector<OpReturnType> argsOfFunctionToCall(_arguments.size());
  for (int i = 0; i < _arguments.size(); i++) {
    argsOfFunctionToCall[i] = _arguments[i]->get(stack);
  }
  // call is only implemented for NameNode and AttributeNode
  return _functionNode->call(argsOfFunctionToCall, stack);
}

NameNode::NameNode(VariableScope* scope, const json& nameOpJson) : ASTNode(scope, nameOpJson) {
  std::string type = nameOpJson.at("ctx").at("_type");
  std::string varName = nameOpJson.at("id");

  if (type == "Store") {
    _type = Type::STORE;
    // Ideally we would only check in this scope and create the variable here if it doesn't exist,
    // instead we want to modify outer scope's variable in this case.
    auto stack_location = _scope->get_variable_location_on_stack(varName);
    if (stack_location == StackLocation::null) {
      stack_location = _scope->add_variable(varName);
    }
    _stackLocation = std::move(stack_location);
  } else {
    _type = Type::LOAD;
    _stackLocation = _scope->get_variable_location_on_stack(varName);
  }
  if (_stackLocation == StackLocation::null) {
    THROW("Variable %s used before definition", varName.c_str());
  }
  _variableName = varName;
}

OpReturnType NameNode::get_value(CallStack& stack) {
  if (_type != Type::LOAD)
    throw create_exception("%s", "should call get only of Load Name variable");
  auto ret = stack.get_variable(_stackLocation);
  if (ret == nullptr) {
    THROW("Local variable %s accessed before assignment", _variableName.c_str());
  }
  return ret;
}

OpReturnType NameNode::call(const std::vector<OpReturnType>& args, CallStack& stack) {
  switch (_type) {
    case Type::LOAD: {
      auto functionDataVariable = stack.get_variable(_stackLocation);
      return functionDataVariable->execute_function(args, stack);
    }
    default:
      throw create_exception("%s", "Should not call variable of Store type");
  };
}

AttributeNode::AttributeNode(VariableScope* scope, const json& attributeNodeJson)
    : ASTNode(scope, attributeNodeJson) {
  std::string type = attributeNodeJson.at("ctx").at("_type");
  auto mainNodeJson = attributeNodeJson.at("value");
  _mainNode = ASTNode::create_node(scope, mainNodeJson);
  std::string attr = attributeNodeJson.at("attr");
  _memberIndex = DataVariable::add_and_get_member_func_index(attr);
  if (_memberIndex == -1) {
    THROW("Member %s does not exist", attr.c_str());
  }
}

OpReturnType AttributeNode::call(const std::vector<OpReturnType>& args, CallStack& stack) {
  auto classVariable = _mainNode->get(stack);
  // this calls member function
  return classVariable->call_function(_memberIndex, args, stack);
}

// STATIC FUNCTIONS BELOW

ASTNode* ASTNode::create_node(VariableScope* scope, const json& j) {
  if (j.type() == json::value_t::null) {
    return new NullNode();
  }
  std::string nodeType = j.at("_type");
  if (nodeType == "Constant") {
    return new ConstantNode(scope, j);
  } else if (nodeType == "BinOp") {
    return new BinNode(scope, j);
  } else if (nodeType == "UnaryOp") {
    return new UnaryNode(scope, j);
  } else if (nodeType == "Compare") {
    return new CompareNode(scope, j);
  } else if (nodeType == "BoolOp") {
    return new BoolNode(scope, j);
  } else if (nodeType == "Call") {
    return new CallNode(scope, j);
  } else if (nodeType == "Name") {
    return new NameNode(scope, j);
  } else if (nodeType == "Attribute") {
    return new AttributeNode(scope, j);
  } else if (nodeType == "List") {
    return new ListNode(scope, j);
  } else if (nodeType == "Tuple") {
    return new TupleNode(scope, j);
  } else if (nodeType == "Subscript") {
    return new SubscriptNode(scope, j);
  } else if (nodeType == "Dict") {
    return new DictNode(scope, j);
  } else if (nodeType == "Slice") {
    return new SliceNode(scope, j);
  } else if (nodeType == "ListComp") {
    return new ListComprehensionNode(scope, j);
  } else if (nodeType == "DictComp") {
    return new DictComprehensionNode(scope, j);
  } else if (nodeType == "GeneratorExp") {
    return new GeneratorExpNode(scope, j);
  }

  else {
    THROW("Could not find implementation for Node=%s at lineNo=%d", nodeType.c_str(),
          j.at("lineno").get<int>());
  }
}

OpReturnType DictNode::get_value(CallStack& stack) {
  std::vector<OpReturnType> keys(_keyNodes.size());
  for (int i = 0; i < _keyNodes.size(); i++) {
    keys[i] = _keyNodes[i]->get(stack);
  }
  std::vector<OpReturnType> values(_valueNodes.size());
  for (int i = 0; i < _valueNodes.size(); i++) {
    values[i] = _valueNodes[i]->get(stack);
  }
  return OpReturnType(new MapDataVariable(keys, values));
}

DictNode::~DictNode() {
  for (auto key : _keyNodes) {
    delete key;
  }
  for (auto value : _valueNodes) {
    delete value;
  }
}

OpReturnType DictComprehensionNode::get_value(CallStack& stack) {
  if (_chainGenerators.empty()) {
    return OpReturnType(new MapDataVariable());
  }

  std::vector<OpReturnType> keys;
  std::vector<OpReturnType> values;
  _chainGenerators[0]->reset_iterator();

  // Iterate through all the generator's values and add them to our result list
  while (true) {
    auto kvTuple = _chainGenerators[0]->get(stack);
    if (kvTuple == nullptr) {
      break;
    }
    keys.push_back(kvTuple->get_int_subscript(0));
    values.push_back(kvTuple->get_int_subscript(1));
  }
  _chainGenerators[0]->reset_iterator();
  return OpReturnType(new MapDataVariable(keys, values));
}
