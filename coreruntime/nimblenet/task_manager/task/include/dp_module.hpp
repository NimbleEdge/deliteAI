#pragma once

#include <map>
#include <memory>
#include <string>

#include "ne_fwd.hpp"
#include "statements.hpp"
#include "variable_scope.hpp"

typedef std::shared_ptr<MapDataVariable> MapVariablePtr;

class DpModule {
  std::unique_ptr<Body> _body;
  std::map<std::string, StackLocation> _variableNamesLocationMap;
  std::string _name;
  int _index;

 public:
  DpModule(CommandCenter* commandCenter, const std::string& name, int index, const json& astJson,
           CallStack& stack);
  ~DpModule();
  void operate(const std::string& functionName, const MapVariablePtr inputs, MapVariablePtr outputs,
               CallStack& stack);
  bool has_variable(const std::string& name) const;
  StackLocation get_variable_location(const std::string& name) const;
};
