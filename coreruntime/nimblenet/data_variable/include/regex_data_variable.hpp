/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef REGEX_ENABLED
#pragma once

#include <memory>
#include <regex>
#include <vector>

#include "data_variable.hpp"
#include "list_data_variable.hpp"
#include "tuple_data_variable.hpp"

class RegexDataVariable : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_REGEX; }

  nlohmann::json to_json() const override { return "[Regex]"; }

  OpReturnType regex_match(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_search(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_fullmatch(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_split(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_findall(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_finditer(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_sub(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType regex_subn(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

 public:
  RegexDataVariable() {}

  std::string print() override { return fallback_print(); }
};

#endif
