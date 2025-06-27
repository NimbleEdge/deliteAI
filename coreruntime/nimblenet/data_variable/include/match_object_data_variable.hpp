/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <regex>

#include "data_variable.hpp"

/**
 * @brief DataVariable implementation for regex match objects
 *
 * This class represents the result of a regex match operation, similar to Python's
 * re.Match object. It provides access to matched groups, their positions, and spans
 * through member functions that mirror Python's regex match object API.
 *
 * The class stores the regex match results and the original input string, allowing
 * access to matched groups by index, start/end positions, and spans.
 */
class MatchObjectDataVariable : public DataVariable {
  std::smatch _smatch; /**< The regex match results containing all matched groups */
  std::shared_ptr<std::string> _input; /**< Shared pointer to the original input string that was matched */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_REGEX_MATCHOBJECT; }

  OpReturnType match_group(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType match_groups(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType match_start(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType match_end(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType match_span(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  nlohmann::json to_json() const override { return "[RegexMatchObject]"; }

 public:
  /**
   * @brief Constructs a MatchObjectDataVariable with regex match results
   * @param smatch The regex match results (moved)
   * @param input Shared pointer to the original input string (moved)
   */
  MatchObjectDataVariable(std::smatch&& smatch, std::shared_ptr<std::string> input)
      : _smatch(std::move(smatch)), _input(std::move(input)) {}

  std::string print() override { return fallback_print(); }
};
