#include "dp_module.hpp"

DpModule::DpModule(CommandCenter* commandCenter, const std::string& name, int index,
                   const json& astJson, CallStack& stack)
    : _name(name), _index(index) {
  const json& bodyJson = astJson.at("body");
  auto globalScope = new VariableScope(commandCenter, index);
  _body = std::make_unique<Body>(globalScope, bodyJson, new InbuiltFunctionsStatement(globalScope));

  stack.enter_function_frame(index, globalScope->current_function_index(),
                             *globalScope->num_variables_stack());

  CallStack copyStack = stack.create_copy_with_deferred_lock();
  _body->execute(copyStack);
  _variableNamesLocationMap = globalScope->get_all_locations_in_scope();
  delete globalScope;
}

DpModule::~DpModule() = default;

void DpModule::operate(const std::string& functionName, const MapVariablePtr inputs,
                       MapVariablePtr outputs, CallStack& callStack) {
  if (_variableNamesLocationMap.find(functionName) == _variableNamesLocationMap.end()) {
    THROW("func=%s not defined in task", functionName.c_str());
  }

  auto functionDataVariable = callStack.get_variable(_variableNamesLocationMap.at(functionName));

  std::vector<OpReturnType> argsToFunction;
  argsToFunction.push_back(OpReturnType(inputs));

  auto returnValues = functionDataVariable->execute_function(argsToFunction);
  if (returnValues->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("return type of function called from outside has to be map got %s",
          returnValues->get_containerType_string());
  }
  outputs->add_or_update(returnValues);
}

bool DpModule::has_variable(const std::string& name) const {
  return _variableNamesLocationMap.find(name) != _variableNamesLocationMap.end();
}

// caller has to ensure that variable exists
StackLocation DpModule::get_variable_location(const std::string& name) const {
  return _variableNamesLocationMap.at(name);
}
