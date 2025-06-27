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

/**
 * @brief DataVariable implementation for regular expression operations
 *
 * RegexDataVariable provides Python-like regex functionality within the NimbleNet
 * runtime system. It implements common regex operations such as matching, searching,
 * splitting, and substitution, with behavior designed to be compatible with Python's
 * re module patterns.
 *
 * The class supports operations like:
 * - Pattern matching with match(), search(), and fullmatch()
 * - String splitting with split()
 * - Pattern finding with findall() and finditer()
 * - String substitution with sub() and subn()
 *
 * All operations return appropriate DataVariable types that can be used
 * in the NimbleNet execution environment.
 */
class RegexDataVariable : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_REGEX; }

  nlohmann::json to_json() const override { return "[Regex]"; }

  /**
   * @brief Matches a pattern at the beginning of a string
   * @param arguments Vector containing [pattern, string] arguments
   * @param stack Current call stack for execution context
   * @return MatchObjectDataVariable if match found, NoneVariable otherwise
   */
  OpReturnType regex_match(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Searches for a pattern anywhere in a string
   * @param arguments Vector containing [pattern, string] arguments
   * @param stack Current call stack for execution context
   * @return MatchObjectDataVariable if match found, NoneVariable otherwise
   */
  OpReturnType regex_search(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Matches a pattern against the entire string
   * @param arguments Vector containing [pattern, string] arguments
   * @param stack Current call stack for execution context
   * @return MatchObjectDataVariable if full match found, NoneVariable otherwise
   */
  OpReturnType regex_fullmatch(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Splits a string by a regex pattern
   * @param arguments Vector containing [pattern, string, return_groups?] arguments
   * @param stack Current call stack for execution context
   * @return StringTensorVariable containing split results
   */
  OpReturnType regex_split(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Finds all non-overlapping matches of a pattern
   * @param arguments Vector containing [pattern, string] arguments
   * @param stack Current call stack for execution context
   * @return ListDataVariable containing all matches (strings or tuples for groups)
   */
  OpReturnType regex_findall(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Returns an iterator of match objects
   * @param arguments Vector containing [pattern, string] arguments
   * @param stack Current call stack for execution context
   * @return ListDataVariable containing MatchObjectDataVariable instances
   */
  OpReturnType regex_finditer(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Substitutes pattern matches with replacement string
   * @param arguments Vector containing [pattern, replacement, string, count?] arguments
   * @param stack Current call stack for execution context
   * @return SingleVariable containing the substituted string
   */
  OpReturnType regex_sub(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Substitutes pattern matches and returns count of replacements
   * @param arguments Vector containing [pattern, replacement, string, count?] arguments
   * @param stack Current call stack for execution context
   * @return TupleDataVariable containing [substituted_string, replacement_count]
   */
  OpReturnType regex_subn(const std::vector<OpReturnType>& arguments, CallStack& stack);

  /**
   * @brief Dispatches regex function calls based on member function index
   * @param memberFuncIndex Index identifying which regex operation to perform
   * @param arguments Arguments for the regex operation
   * @param stack Current call stack for execution context
   * @return Result of the regex operation as appropriate DataVariable type
   */
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

 public:
  RegexDataVariable() {}

  std::string print() override { return fallback_print(); }
};

#endif
