/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "custom_func_data_variable.hpp"

OpReturnType CustomFuncDataVariable::execute_function(const std::vector<OpReturnType>& arguments,
                                                      CallStack& stack) {
  return _func(arguments, stack);
}
