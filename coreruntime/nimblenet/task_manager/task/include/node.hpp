#pragma once

#include "binary_operators.hpp"
#include "bool_operators.hpp"
#include "compare_operators.hpp"
#include "core_utils/fmt.hpp"
#include "custom_functions.hpp"
#include "iterable_data_variable.hpp"
#include "list_data_variable.hpp"
#include "map_data_variable.hpp"
#include "tuple_data_variable.hpp"
#include "unary_operators.hpp"
#include "variable_scope.hpp"

class ASTNode {
 protected:
  VariableScope* _scope = nullptr;
  int _lineNo = -1;
  std::string _id;

 public:
  static ASTNode* create_node(VariableScope* scope, const json& j);
  static ASTNode* create_BinNode(VariableScope* scope, const json& binOpJson);
  virtual OpReturnType get_value(CallStack& stack) = 0;

  virtual void set_variable(OpReturnType d, CallStack& stack) {
    throw create_exception("%s", "cannot assign");
  }

  virtual OpReturnType call(const std::vector<OpReturnType>& args, CallStack& stack) {
    THROW("%s", "Cannot call variable");
  }

  OpReturnType get(CallStack& stack) {
    try {
      auto ret = get_value(stack);
      return ret;
    } catch (std::exception& e) {
      throw create_exception("%s", e.what());
    }
  }

  void set(OpReturnType d, CallStack& stack) {
    try {
      set_variable(d, stack);
    } catch (std::exception& e) {
      throw create_exception("%s", e.what());
    }
  }

  ASTNode(VariableScope* scope, const json& binOpJson) {
    _lineNo = binOpJson.at("lineno");
    _scope = scope;
  }

  ASTNode() {}

  std::runtime_error create_exception(const char* format, ...) {
    NE_VARIADIC_FMT(format);
    auto runTIMEERROR = std::runtime_error(buf.str);
    return runTIMEERROR;
  }

  virtual ~ASTNode() = default;
};

class NullNode : public ASTNode {
 public:
  NullNode() {}

  OpReturnType get_value(CallStack&) override { return OpReturnType(new NoneVariable()); }
};

class ConstantNode : public ASTNode {
  OpReturnType _d = nullptr;

 public:
  ConstantNode(VariableScope* scope, const json& constJson);

  OpReturnType get_value(CallStack&) override { return _d; }
};

class BinNode : public ASTNode {
 protected:
  ASTNode* _left = nullptr;
  ASTNode* _right = nullptr;
  std::string _opType;

 public:
  BinNode(VariableScope* scope, const json& binOpJson);
  OpReturnType get_value(CallStack& stack) override;

  virtual ~BinNode() {
    delete _left;
    delete _right;
  }
};

class UnaryNode : public ASTNode {
 protected:
  ASTNode* _operand = nullptr;
  UnaryOpFuncPtr _func = nullptr;
  std::string _opType;

 public:
  UnaryNode(VariableScope* scope, const json& unaryOpJson);
  OpReturnType get_value(CallStack& stack) override;

  virtual ~UnaryNode() { delete _operand; }
};

class CompareNode : public ASTNode {
 protected:
  std::vector<ASTNode*> _comparators;
  std::vector<CompareFuncPtr> _compareFuncs;
  ASTNode* _left = nullptr;
  std::vector<std::string> _opTypes;

 public:
  CompareNode(VariableScope* scope, const json& compareOpJson);
  OpReturnType get_value(CallStack& stack) override;

  virtual ~CompareNode() {
    delete _left;
    for (auto comp : _comparators) {
      delete comp;
    }
  }
};

class BoolNode : public ASTNode {
  BoolFuncPtr _func;
  std::string _opType;
  std::vector<ASTNode*> _comparators;

 public:
  BoolNode(VariableScope* scope, const json& boolOpJson);
  OpReturnType get_value(CallStack& stack) override;

  virtual ~BoolNode() {
    for (auto comp : _comparators) {
      delete comp;
    }
  }
};

class CallNode : public ASTNode {
  std::vector<ASTNode*> _arguments;
  ASTNode* _functionNode = nullptr;

 public:
  CallNode(VariableScope* scope, const json& callFuncJson);
  OpReturnType get_value(CallStack& stack) override;

  virtual ~CallNode() {
    for (auto arg : _arguments) {
      delete arg;
    }
    delete _functionNode;
  }
};

class ListNode : public ASTNode {
  std::vector<ASTNode*> _membersInList;

 public:
  ListNode(VariableScope* scope, const json& listNodeJson) : ASTNode(scope, listNodeJson) {
    auto jsonArray = listNodeJson.at("elts");
    for (auto itemJson : jsonArray) {
      _membersInList.push_back(create_node(scope, itemJson));
    }
  }

  OpReturnType get_value(CallStack& stack) override {
    std::vector<OpReturnType> membersOfList(_membersInList.size());
    for (int i = 0; i < _membersInList.size(); i++) {
      membersOfList[i] = _membersInList[i]->get(stack);
    }
    return OpReturnType(new ListDataVariable(std::move(membersOfList)));
  }

  virtual ~ListNode() {
    for (auto mem : _membersInList) {
      delete mem;
    }
  }
};

class TupleNode : public ASTNode {
  std::vector<ASTNode*> _membersInTuple;
  bool _store = false;

 public:
  TupleNode(VariableScope* scope, const json& tupleNodeJson) : ASTNode(scope, tupleNodeJson) {
    auto jsonArray = tupleNodeJson.at("elts");
    std::string type = tupleNodeJson.at("ctx").at("_type");
    if (type == "Store") {
      _store = true;
    }
    for (auto itemJson : jsonArray) {
      _membersInTuple.push_back(create_node(scope, itemJson));
    }
  }

  void set_variable(OpReturnType d, CallStack& stack) override {
    if (!_store) throw create_exception("%s", "cannot set rvalue variable");
    if (d->get_containerType() == CONTAINERTYPE::TUPLE) {
      auto tupleVariable = (TupleDataVariable*)d.get();
      auto tupleVec = tupleVariable->get_members();
      if (_membersInTuple.size() != tupleVec.size()) {
        throw create_exception("expecting %d targets but %d vals returned", _membersInTuple.size(),
                               tupleVec.size());
      }
      for (int i = 0; i < tupleVec.size(); i++) {
        _membersInTuple[i]->set(tupleVec[i], stack);
      }
    }
  }

  OpReturnType get_value(CallStack& stack) override {
    if (_store) throw create_exception("%s", "should not call get of store Tuple variable");
    std::vector<OpReturnType> membersOfTuple(_membersInTuple.size());
    for (int i = 0; i < _membersInTuple.size(); i++) {
      membersOfTuple[i] = _membersInTuple[i]->get(stack);
    }
    auto ret = OpReturnType(new TupleDataVariable(membersOfTuple));
    if (ret == nullptr) {
      throw create_exception("%s", "could not create tuple");
    }
    return ret;
  }

  virtual ~TupleNode() {
    for (auto mem : _membersInTuple) {
      delete mem;
    }
  }
};

class NameNode : public ASTNode {
  StackLocation _stackLocation = StackLocation::null;
  enum class Type { STORE, LOAD };
  Type _type = Type::LOAD;
  std::string _variableName;

 public:
  NameNode(VariableScope* scope, const json& nameOpJson);

  void set_variable(OpReturnType d, CallStack& stack) override {
    if (_type != Type::STORE)
      throw create_exception("%s", "can only call set for store name variable");
    stack.set_variable(_stackLocation, d);
  }

  OpReturnType get_value(CallStack& stack) override;
  OpReturnType call(const std::vector<OpReturnType>& args, CallStack& stack) override;
};

class AttributeNode : public ASTNode {
  int _memberIndex = -1;
  // this mainNode is the variable whose attribute is called or accessed.
  ASTNode* _mainNode = nullptr;

 public:
  AttributeNode(VariableScope* scope, const json& nameOpJson);

  void set_variable(OpReturnType d, CallStack& stack) override {
    // when setting obj.member
    auto object = _mainNode->get(stack);
    object->set_member(_memberIndex, d);
  }

  OpReturnType get_value(CallStack& stack) override {
    // when accessing obj.member
    auto object = _mainNode->get(stack);
    return object->get_member(_memberIndex);
  }

  OpReturnType call(const std::vector<OpReturnType>& args, CallStack& stack) override;

  ~AttributeNode() { delete _mainNode; }
};

class SliceNode : public ASTNode {
  ASTNode* _lower = nullptr;  // Lower bound of slice (or nullptr if not specified)
  ASTNode* _upper = nullptr;  // Upper bound of slice (or nullptr if not specified)
  ASTNode* _step = nullptr;   // Step value of slice (or nullptr if not specified)

 public:
  SliceNode(VariableScope* scope, const json& sliceJson) : ASTNode(scope, sliceJson) {
    // Parse the lower bound if it exists
    if (sliceJson.contains("lower") && !sliceJson["lower"].is_null()) {
      _lower = create_node(scope, sliceJson.at("lower"));
    }

    // Parse the upper bound if it exists
    if (sliceJson.contains("upper") && !sliceJson["upper"].is_null()) {
      _upper = create_node(scope, sliceJson.at("upper"));
    }

    // Parse the step if it exists
    if (sliceJson.contains("step") && !sliceJson["step"].is_null()) {
      _step = create_node(scope, sliceJson.at("step"));
    }
  }

  OpReturnType get_value(CallStack& stack) override {
    // Create a slice object with the bounds and step
    OpReturnType lowerVal = _lower ? _lower->get(stack) : OpReturnType(new NoneVariable());
    OpReturnType upperVal = _upper ? _upper->get(stack) : OpReturnType(new NoneVariable());
    OpReturnType stepVal = _step ? _step->get(stack) : OpReturnType(new NoneVariable());

    // Return a new slice variable
    return OpReturnType(new ListSliceVariable(lowerVal, upperVal, stepVal));
  }

  virtual ~SliceNode() {
    delete _lower;
    delete _upper;
    delete _step;
  }
};

class SubscriptNode : public ASTNode {
  bool _store = false;
  // assuming slice is a constant
  ASTNode* _sliceNode = nullptr;
  ASTNode* _mainNode = nullptr;

 public:
  SubscriptNode(VariableScope* scope, const json& subOpJson) : ASTNode(scope, subOpJson) {
    std::string type = subOpJson.at("ctx").at("_type");
    if (type == "Store") {
      _store = true;
    }
    auto sliceJson = subOpJson.at("slice");
    auto valueJson = subOpJson.at("value");
    // Check if this is a slice operation
    if (sliceJson.contains("_type") && sliceJson.at("_type") == "Slice") {
      _sliceNode = new SliceNode(scope, sliceJson);
    } else {
      _sliceNode = create_node(scope, sliceJson);
    }
    _mainNode = create_node(scope, valueJson);
  }

  void set_variable(OpReturnType d, CallStack& stack) override {
    if (!_store) throw create_exception("%s", "cannot set rvalue variable");
    auto subscript = _sliceNode->get(stack);
    auto mainData = _mainNode->get(stack);
    mainData->set_subscript(subscript, d);
  }

  OpReturnType get_value(CallStack& stack) override {
    auto subscript = _sliceNode->get(stack);
    auto mainData = _mainNode->get(stack);

    if (subscript->get_containerType() == CONTAINERTYPE::SLICE) {
      if (mainData->get_containerType() == CONTAINERTYPE::LIST) {
        return mainData->get_subscript(subscript);
      } else if (mainData->get_containerType() == CONTAINERTYPE::SINGLE &&
                 mainData->get_dataType_enum() == DATATYPE::STRING) {
        return mainData->get_subscript(subscript);
      } else {
        throw create_exception("%s", "cannot subscript non-list or non-string variable");
      }
    }

    if (subscript->get_dataType_enum() == DATATYPE::STRING) {
      return mainData->get_string_subscript(subscript->get_string());
    } else {
      return mainData->get_int_subscript(subscript->get_int32());
    }
  }

  virtual ~SubscriptNode() {
    delete _sliceNode;
    delete _mainNode;
  }
};

class DictNode : public ASTNode {
  std::vector<ASTNode*> _keyNodes;
  std::vector<ASTNode*> _valueNodes;

 public:
  DictNode(VariableScope* scope, const json& dictNodeJson) : ASTNode(scope, dictNodeJson) {
    auto keysJsonArray = dictNodeJson.at("keys");
    auto valuesJsonArray = dictNodeJson.at("values");
    for (auto keyJson : keysJsonArray) {
      _keyNodes.push_back(create_node(scope, keyJson));
    }
    for (auto valueJson : valuesJsonArray) {
      _valueNodes.push_back(create_node(scope, valueJson));
    }
    if (_keyNodes.size() != _valueNodes.size()) {
      THROW("keys=%d not equal to values=%d", _keyNodes.size(), _valueNodes.size());
    }
  }

  ~DictNode() override;

  OpReturnType get_value(CallStack& stack) override;
};

class SingleGeneratorNode : public ASTNode {
 private:
  VariableScope* _generatorScope = nullptr;
  ASTNode* _iterableNode = nullptr;
  ASTNode* _targetNode = nullptr;
  std::vector<ASTNode*> _elementNodes;
  std::vector<ASTNode*> _conditionNodes;
  SingleGeneratorNode* _nextGenerator = nullptr;
  IterableOverScriptable* _iterable = nullptr;
  OpReturnType _cachedItem = nullptr;

 public:
  SingleGeneratorNode(VariableScope* generatorScope, const json& genJson)
      : ASTNode(generatorScope, genJson) {
    _generatorScope = generatorScope;
    // Parse the iterable and configure the generator
    _iterableNode = create_node(generatorScope, genJson.at("iter"));
    // Get target (which could be a name or tuple)
    auto targetJson = genJson.at("target");
    std::string targetType = targetJson.at("_type");

    if (targetType == "Name") {
      // Simple target like 'x' in [x for x in range(5)]
      _targetNode = create_node(generatorScope, targetJson);
    } else if (targetType == "Tuple") {
      // Tuple target like '(x,y)' in [x*y for x,y in pairs]
      _targetNode = create_node(generatorScope, targetJson);
    } else {
      throw create_exception("Unsupported target type %s in comprehension", targetType.c_str());
    }

    // Add any conditions
    if (genJson.contains("ifs") && !genJson.at("ifs").empty()) {
      for (auto& ifJson : genJson.at("ifs")) {
        _conditionNodes.push_back(create_node(generatorScope, ifJson));
      }
    }
    _iterable = new IterableOverScriptable();
  }

  VariableScope* get_scope() const { return _generatorScope; }

  void add_element_node(ASTNode* elementNode) { _elementNodes.push_back(elementNode); }

  void set_next_generator(SingleGeneratorNode* nextGenerator) { _nextGenerator = nextGenerator; }

  void reset_iterator() {
    _iterable->reset_iterator();
    _cachedItem = nullptr;
    if (_nextGenerator != nullptr) {
      _nextGenerator->reset_iterator();
    }
  }

  OpReturnType get_next_and_reset_next_generator(CallStack& stack) {
    try {
      OpReturnType item = _iterable->next(stack);
      if (_nextGenerator != nullptr) {
        _nextGenerator->reset_iterator();
      }
      return item;
    } catch (const std::runtime_error& e) {
      if (std::string(e.what()) == "StopIteration") {
        if (_nextGenerator != nullptr) {
          _nextGenerator->reset_iterator();
        }
        return nullptr;
      }
      throw;
    }
  }

  OpReturnType get_value(CallStack& stack) override {
    // Get the iterable for this generator
    _iterable->set_data(_iterableNode->get(stack));
    // Prepare result
    std::vector<OpReturnType> resultItems;
    // Get current item
    if (_cachedItem == nullptr) {
      _cachedItem = get_next_and_reset_next_generator(stack);
    }
    while (_cachedItem != nullptr) {
      // Assign to target variable
      _targetNode->set_variable(_cachedItem, stack);
      // Check conditions
      bool conditionsPassed = true;
      for (auto& condNode : _conditionNodes) {
        auto result = condNode->get(stack);
        if (!result->get_bool()) {
          conditionsPassed = false;
          break;
        }
      }

      // Skip if conditions aren't met
      if (!conditionsPassed) {
        _cachedItem = get_next_and_reset_next_generator(stack);
        continue;
      }

      // Process this item
      if (_nextGenerator != nullptr) {
        auto res = _nextGenerator->get(stack);
        if (res == nullptr) {
          // _nextGenerator is exhausted, so we need to get the next item from the current generator
          _cachedItem = get_next_and_reset_next_generator(stack);
          continue;
        }
        return res;
      } else if (!_elementNodes.empty()) {
        for (auto& elem : _elementNodes) {
          resultItems.push_back(elem->get(stack));
        }
        _cachedItem = get_next_and_reset_next_generator(stack);
        return OpReturnType(new TupleDataVariable(std::move(resultItems)));
      }
      THROW("No next generator or element nodes found");
    }
    // If we get here, we've exhausted the generator
    return nullptr;
  }

  ~SingleGeneratorNode() {
    delete _iterableNode;
    delete _targetNode;
    delete _nextGenerator;
    for (auto& cond : _conditionNodes) {
      delete cond;
    }
    delete _iterable;
    for (auto& elem : _elementNodes) {
      delete elem;
    }
  }
};

class ComprehensionNode : public ASTNode {
 protected:
  std::vector<SingleGeneratorNode*> _chainGenerators;  // Chain of generator data variables

 public:
  // Constructor that handles all generators in the chain
  ComprehensionNode(VariableScope* scope, const json& comprehensionJson)
      : ASTNode(scope, comprehensionJson) {
    auto generatorScope = scope;

    // Extract generators from the comprehension JSON
    auto generatorsJson = comprehensionJson.at("generators");

    // Create a scope for the generator's variables
    for (const auto& generatorJson : generatorsJson) {
      // Make sure the generator has a valid lineno
      json genJson = generatorJson;
      if (!genJson.contains("lineno") && genJson.contains("target") &&
          genJson.at("target").contains("lineno")) {
        genJson["lineno"] = genJson.at("target").at("lineno");
      }
      generatorScope = generatorScope->add_scope();
      _chainGenerators.push_back(new SingleGeneratorNode(generatorScope, genJson));
    }

    // Link generators in the chain
    if (!_chainGenerators.empty()) {
      // Set up the generator chain
      for (size_t i = 0; i < _chainGenerators.size() - 1; i++) {
        _chainGenerators[i]->set_next_generator(_chainGenerators[i + 1]);
      }
    }
  }

  ASTNode* create_element_node(const json& eltJson) {
    if (!_chainGenerators.empty()) {
      auto scope = _chainGenerators.back()->get_scope();
      auto elementNode = create_node(scope, eltJson);
      _chainGenerators.back()->add_element_node(elementNode);
      return elementNode;
    }
    return nullptr;
  }

  // Process this generator chain, returning the first generator
  OpReturnType get_value(CallStack& stack) override { return _chainGenerators[0]->get(stack); }

  ~ComprehensionNode() {
    for (auto gen : _chainGenerators) {
      delete gen;
    }
  }
};

// List comprehension: [expr for var in iterable]
class ListComprehensionNode : public ComprehensionNode {
  ASTNode* _elementNode = nullptr;

 public:
  ListComprehensionNode(VariableScope* scope, const json& comprehensionJson)
      : ComprehensionNode(scope, comprehensionJson) {
    // Extract the element expression (result expression)
    auto eltJson = comprehensionJson.at("elt");
    _elementNode = create_element_node(eltJson);
  }

  // Run the full comprehension loop and return a complete list
  OpReturnType get_value(CallStack& stack) override {
    if (_chainGenerators.empty()) {
      return OpReturnType(new ListDataVariable());
    }
    std::vector<OpReturnType> resultItems;
    _chainGenerators[0]->reset_iterator();
    // Iterate through all the generator's values and add them to our result list
    while (true) {
      auto response = _chainGenerators[0]->get(stack);
      if (response == nullptr) {
        break;
      }
      resultItems.push_back(response->get_int_subscript(0));
    }
    _chainGenerators[0]->reset_iterator();
    return OpReturnType(new ListDataVariable(std::move(resultItems)));
  }
};

// Dictionary comprehension: {key_expr: value_expr for var in iterable}
class DictComprehensionNode : public ComprehensionNode {
 private:
  ASTNode* _keyNode = nullptr;    // Key expression
  ASTNode* _valueNode = nullptr;  // Value expression

 public:
  DictComprehensionNode(VariableScope* scope, const json& dictCompJson)
      : ComprehensionNode(scope, dictCompJson) {
    // Extract the key and value expressions
    auto keyJson = dictCompJson.at("key");
    _keyNode = create_element_node(keyJson);
    auto valueJson = dictCompJson.at("value");
    _valueNode = create_element_node(valueJson);
  }

  // Run the full comprehension loop and return a dictionary
  OpReturnType get_value(CallStack& stack) override;

  ~DictComprehensionNode() {
    delete _keyNode;
    delete _valueNode;
    // Parent destructor will clean up the generators
  }
};

// Generator expression: (expr for var in iterable)
class GeneratorExpNode : public ComprehensionNode {
 public:
  GeneratorExpNode(VariableScope* scope, const json& comprehensionJson)
      : ComprehensionNode(scope, comprehensionJson) {}
};
