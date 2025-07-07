/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <memory>
#include <string>

#include "ne_fwd.hpp"
#include "statements.hpp"
#include "variable_scope.hpp"

typedef std::shared_ptr<MapDataVariable> MapVariablePtr;

/**
 * @brief Represents a Python module in the task execution system
 *
 * DpModule encapsulates a Python module's AST and provides execution capabilities.
 * It manages the module's global variables, functions, and provides an interface
 * for calling functions defined within the module.
 */
class DpModule {
  std::unique_ptr<Body> _body;  /**< The module's main body containing all statements */
  std::map<std::string, StackLocation> _variableNamesLocationMap;  /**< Maps variable names to their stack locations */
  std::string _name;  /**< Name of the module */
  int _index;         /**< Unique index of this module within the task */

 public:
  DpModule(CommandCenter* commandCenter, const std::string& name, int index, const json& astJson,
           CallStack& stack);
  ~DpModule();
  void operate(const std::string& functionName, const MapVariablePtr inputs, MapVariablePtr outputs,
               CallStack& stack);
  bool has_variable(const std::string& name) const;
  StackLocation get_variable_location(const std::string& name) const;
};
