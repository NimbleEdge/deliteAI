/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "match_object_data_variable.hpp"

#include "single_variable.hpp"
#include "tuple_data_variable.hpp"

OpReturnType MatchObjectDataVariable::match_group(const std::vector<OpReturnType>& arguments,
                                                  CallStack& stack) {
  // For all the arguments check that they are integers and not less than 0 and less than equal to
  // number of groups
  for (int i = 0; i < arguments.size(); i++) {
    if (arguments[i]->get_int32() < 0 || arguments[i]->get_int32() >= _smatch.size()) {
      THROW(
          "match.group() expects all arguments to be >=0 and < total number of groups. Argument "
          "at "
          "%d position is invalid.",
          i);
    }
  }
  // By default return complete match
  int index = 0;
  if (arguments.size() == 1) {
    index = arguments[0]->get_int32();
  }
  // If only one or no argument then return that match
  if (arguments.size() <= 1) {
    if (_smatch[index].matched) {
      // Use character position aware extraction
      std::string match_str = get_char_match(index);
      return OpReturnType(new SingleVariable<std::string>(match_str));
    } else {
      return OpReturnType(new NoneVariable());
    }
  }
  // If multiple arguments, return tuple with all the matches
  std::vector<OpReturnType> groups;
  for (int i = 0; i < arguments.size(); i++) {
    index = arguments[i]->get_int32();
    if (_smatch[index].matched) {
      // Use character position aware extraction for each group
      std::string match_str = get_char_match(index);
      groups.push_back(OpReturnType(new SingleVariable<std::string>(match_str)));
    } else {
      groups.push_back(OpReturnType(new NoneVariable()));
    }
  }
  return OpReturnType(new TupleDataVariable(groups));
}

OpReturnType MatchObjectDataVariable::match_groups(const std::vector<OpReturnType>& arguments,
                                                   CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 0, 1,
                                     MemberFuncType::REGEX_MATCHOBJECT_GROUPS);
  // Python's match.groups() supports all types of default values as an argument, we currently
  // have string
  if (arguments.size() == 1) {
    THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                      MemberFuncType::REGEX_MATCHOBJECT_GROUPS);
  }

  OpReturnType defaultValue;
  if (arguments.size() == 1) {
    defaultValue = OpReturnType(new SingleVariable<std::string>(arguments[0]->get_string()));
  }

  std::vector<OpReturnType> matched_groups;
  for (int i = 0; i < _smatch.size(); i++) {
    if (_smatch[i].matched) {
      // Use character position aware extraction for each group
      std::string match_str = get_char_match(i);
      matched_groups.push_back(OpReturnType(new SingleVariable<std::string>(match_str)));
    } else {
      // If no default value then return None
      if (arguments.size() == 0) {
        matched_groups.push_back(OpReturnType(new NoneVariable()));
      } else {
        matched_groups.push_back(defaultValue);
      }
    }
  }

  return OpReturnType(new TupleDataVariable(
      std::vector<OpReturnType>(matched_groups.begin() + 1, matched_groups.end())));
}

OpReturnType MatchObjectDataVariable::match_start(const std::vector<OpReturnType>& arguments,
                                                  CallStack& stack) {
  // Takes in a single integer argument
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 0, 1,
                                     MemberFuncType::REGEX_MATCHOBJECT_START);
  int index = 0;
  if (arguments.size() > 0) {
    index = arguments[0]->get_int32();
  }
  if (index < 0 || index >= _smatch.size()) {
    THROW("%s", "match.start() expects argument to be >=0 and < total number of groups.");
  }
  if (!_smatch[index].matched) {
    return OpReturnType(new SingleVariable<int32_t>(-1));
  }
  // Convert byte position to character position
  int char_pos = byte_to_char_pos(_smatch.position(index));
  return OpReturnType(new SingleVariable<int32_t>(char_pos));
}

OpReturnType MatchObjectDataVariable::match_end(const std::vector<OpReturnType>& arguments,
                                                CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 0, 1, MemberFuncType::REGEX_MATCHOBJECT_END);
  int index = 0;
  if (arguments.size() > 0) {
    index = arguments[0]->get_int32();
  }
  if (index < 0 || index >= _smatch.size()) {
    THROW("%s", "match.end() expects argument to be >=0 and < total number of groups.");
  }
  if (!_smatch[index].matched) {
    return OpReturnType(new SingleVariable<int32_t>(-1));
  }
  // Convert byte position to character position
  int byte_end = _smatch.position(index) + _smatch.length(index);
  int char_pos = byte_to_char_pos(byte_end);
  return OpReturnType(new SingleVariable<int32_t>(char_pos));
}

OpReturnType MatchObjectDataVariable::match_span(const std::vector<OpReturnType>& arguments,
                                                 CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 0, 1,
                                     MemberFuncType::REGEX_MATCHOBJECT_SPAN);
  int index = 0;
  if (arguments.size() > 0) {
    index = arguments[0]->get_int32();
  }
  if (index < 0 || index >= _smatch.size()) {
    THROW("%s", "match.span() expects argument to be >=0 and < total number of groups.");
  }
  std::vector<OpReturnType> span;
  if (!_smatch[index].matched) {
    span.push_back(OpReturnType(new SingleVariable<int32_t>(-1)));
    span.push_back(OpReturnType(new SingleVariable<int32_t>(-1)));
  } else {
    // Convert byte positions to character positions
    int byte_start = _smatch.position(index);
    int byte_end = byte_start + _smatch.length(index);
    int char_start = byte_to_char_pos(byte_start);
    int char_end = byte_to_char_pos(byte_end);

    span.push_back(OpReturnType(new SingleVariable<int32_t>(char_start)));
    span.push_back(OpReturnType(new SingleVariable<int32_t>(char_end)));
  }
  return OpReturnType(new TupleDataVariable(span));
}

OpReturnType MatchObjectDataVariable::call_function(int memberFuncIndex,
                                                    const std::vector<OpReturnType>& arguments,
                                                    CallStack& stack) {
  // TODO: Following attributes/functions of match object are not implemented m.lastindex,
  // m.lastgroup, m.re, m.string, m.pos, m.endpos, m.groupdict()
  switch (memberFuncIndex) {
    case MemberFuncType::REGEX_MATCHOBJECT_GROUP:
      return match_group(arguments, stack);
    case MemberFuncType::REGEX_MATCHOBJECT_GROUPS:
      return match_groups(arguments, stack);
    case MemberFuncType::REGEX_MATCHOBJECT_START:
      return match_start(arguments, stack);
    case MemberFuncType::REGEX_MATCHOBJECT_END:
      return match_end(arguments, stack);
    case MemberFuncType::REGEX_MATCHOBJECT_SPAN:
      return match_span(arguments, stack);
    default:
      THROW("%s not implemented for RegexMatchObject",
            DataVariable::get_member_func_string(memberFuncIndex));
  }
}

// Private helper methods for MatchObjectDataVariable
SingleVariable<std::string>* MatchObjectDataVariable::get_string_var() const {
  return static_cast<SingleVariable<std::string>*>(_input_string.get());
}

int MatchObjectDataVariable::byte_to_char_pos(int byte_pos) const {
  auto string_var = get_string_var();
  if (!string_var) return -1;
  int char_count = string_var->get_char_count();
  for (int char_idx = 0; char_idx < char_count; char_idx++) {
    int curr_byte_pos = string_var->char_idx_to_byte_pos(char_idx);
    if (curr_byte_pos >= byte_pos) {
      return char_idx;
    }
  }
  return char_count;
}

std::string MatchObjectDataVariable::get_char_match(int index) const {
  if (!_smatch[index].matched) {
    return "";
  }
  auto string_var = get_string_var();
  if (!string_var) return "";
  int byte_start = _smatch.position(index);
  int byte_end = byte_start + _smatch.length(index);
  int char_start = byte_to_char_pos(byte_start);
  int char_end = byte_to_char_pos(byte_end);
  if (char_start == char_end) return "";
  std::string result;
  for (int i = char_start; i < char_end; i++) {
    int byte_pos = string_var->char_idx_to_byte_pos(i);
    result += util::utf8::extract_char(string_var->get_string(), byte_pos);
  }
  return result;
}
