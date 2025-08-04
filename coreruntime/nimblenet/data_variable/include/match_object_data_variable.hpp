/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <regex>
#include <utility>

#include "core_utils/fmt.hpp"
#include "data_variable.hpp"
#include "single_variable.hpp"

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
  OpReturnType _input_string; /**< SingleVariable<std::string> containing the original input string */

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
   * @param input_string The input string as an OpReturnType (SingleVariable<std::string>)
   */
  MatchObjectDataVariable(std::smatch&& smatch, const OpReturnType& input_string)
      : _smatch(std::move(smatch)), _input_string(input_string) {
    if (!input_string || input_string->get_dataType_enum() != DATATYPE::STRING) {
      THROW("%s", "MatchObjectDataVariable requires a string variable");
    }
  }

  std::string print() override { return fallback_print(); }

 private:
  SingleVariable<std::string>* get_string_var() const;
  int byte_to_char_pos(int byte_pos) const;
  std::string get_char_match(int index) const;
};
