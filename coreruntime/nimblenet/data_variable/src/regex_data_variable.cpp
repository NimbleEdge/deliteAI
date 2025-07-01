/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef REGEX_ENABLED

#include "regex_data_variable.hpp"

#include <climits>

#include "match_object_data_variable.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"

OpReturnType RegexDataVariable::regex_match(const std::vector<OpReturnType>& arguments,
                                            CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::REGEX_MATCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_MATCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_MATCH);

  std::smatch match_result;

  // Python's re.match also returns true even if a certain part of the string from beginning is
  // matching whereas c++ regex_match matches the whole string so to overcome this disparity need
  // to use match_continuous flag and regex_search
  std::string pattern = arguments[0]->get_string();
  std::regex re = std::regex(pattern);
  auto input = std::make_shared<std::string>(arguments[1]->get_string());
  if (std::regex_search(*input, match_result, re, std::regex_constants::match_continuous)) {
    return OpReturnType(new MatchObjectDataVariable(std::move(match_result), std::move(input)));
  }
  return OpReturnType(new NoneVariable());
}

OpReturnType RegexDataVariable::regex_search(const std::vector<OpReturnType>& arguments,
                                             CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::REGEX_SEARCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_SEARCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_SEARCH);
  // lookbehind assertion not supported like re.search("(?<=abc)def", "abcdef") by
  // std::regex_search
  std::smatch match_result;
  auto input = std::make_shared<std::string>(arguments[1]->get_string());
  std::regex re(arguments[0]->get_string());
  if (std::regex_search(*input, match_result, re)) {
    return OpReturnType(new MatchObjectDataVariable(std::move(match_result), std::move(input)));
  }
  return OpReturnType(new NoneVariable());
}

OpReturnType RegexDataVariable::regex_fullmatch(const std::vector<OpReturnType>& arguments,
                                                CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::REGEX_FULLMATCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_FULLMATCH);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_FULLMATCH);
  std::smatch match_result;
  auto input = std::make_shared<std::string>(arguments[1]->get_string());
  std::regex re(arguments[0]->get_string());
  if (std::regex_match(*input, match_result, re)) {
    return OpReturnType(new MatchObjectDataVariable(std::move(match_result), std::move(input)));
  }
  return OpReturnType(new NoneVariable());
}

OpReturnType RegexDataVariable::regex_split(const std::vector<OpReturnType>& arguments,
                                            CallStack& stack) {
  // This implementation is different from python's re.split
  // Python behaves differently with empty matches and grouped matches
  // The examples given at https://docs.python.org/3/library/re.html#re.split return different
  // outputs than our implementation, especially for e.g. 5, 6, 7
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 2, 3, MemberFuncType ::REGEX_SPLIT);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_SPLIT);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_SPLIT);
  bool return_matched_groups = false;
  if (arguments.size() == 3) {
    THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[2]->get_dataType_enum(), DATATYPE::BOOLEAN, 2,
                                      MemberFuncType::REGEX_SPLIT);
    return_matched_groups = arguments[2]->get_bool();
  }
  std::string input = arguments[1]->get_string();
  std::regex re = std::regex(arguments[0]->get_string());
  std::sregex_token_iterator iter;

  // The default-constructed std::regex_token_iterator is the end-of-sequence iterator
  const std::sregex_token_iterator end;
  if (return_matched_groups) {
    iter = std::sregex_token_iterator(input.cbegin(), input.cend(), re, {-1, 0});
  } else {
    iter = std::sregex_token_iterator(input.cbegin(), input.cend(), re, -1);
  }
  std::vector<std::string> result;
  while (iter != end) {
    result.push_back(*iter);
    iter++;
  }
  std::vector<int64_t> shape(1, result.size());
  return OpReturnType(new StringTensorVariable(std::move(result), std::move(shape), 1));
}

OpReturnType RegexDataVariable::regex_findall(const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::REGEX_FINDALL);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_FINDALL);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_FINDALL);
  // If no match found --> Empty list
  // If no matching gropus, but matches present --> List with strings
  // If just one matching group --> List with strings
  // If multiple matching groups --> List of tuples with strings
  std::regex regex_pattern(arguments[0]->get_string());
  std::string input = arguments[1]->get_string();
  auto words_begin = std::sregex_iterator(input.begin(), input.end(), regex_pattern);
  auto words_end = std::sregex_iterator();
  std::vector<OpReturnType> result;
  for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
    std::smatch match = *it;
    // No matching groups or just one matching group, return strings
    if (match.size() == 1 || match.size() == 2) {
      // If no matching group then match.size() is 1, so take the whole match
      // If just one matching group then match.size() is 2 and take the grouped match which is
      // present at index 1
      result.push_back(OpReturnType(new SingleVariable<std::string>(match.str(match.size() - 1))));
    } else {
      // More than one matching group, return tuples
      std::vector<OpReturnType> group;
      for (int i = 1; i < match.size(); i++) {
        group.push_back(OpReturnType(new SingleVariable<std::string>(match.str(i))));
      }
      result.push_back(OpReturnType(new TupleDataVariable(group)));
    }
  }

  return OpReturnType(new ListDataVariable(std::move(result)));
}

OpReturnType RegexDataVariable::regex_finditer(const std::vector<OpReturnType>& arguments,
                                               CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 2, MemberFuncType::REGEX_FINDITER);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_FINDITER);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_FINDITER);

  std::regex regex_pattern(arguments[0]->get_string());
  auto input = std::make_shared<std::string>(arguments[1]->get_string());
  auto words_begin = std::sregex_iterator(input->begin(), input->end(), regex_pattern);
  auto words_end = std::sregex_iterator();
  std::vector<OpReturnType> result;
  for (std::sregex_iterator it = words_begin; it != words_end; it++) {
    std::smatch smatch = *it;
    result.push_back(OpReturnType(new MatchObjectDataVariable(std::move(smatch), input)));
  }
  return OpReturnType(new ListDataVariable(std::move(result)));
}

OpReturnType RegexDataVariable::regex_sub(const std::vector<OpReturnType>& arguments,
                                          CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 3, 4, MemberFuncType::REGEX_SUB);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_SUB);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_SUB);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[2]->get_dataType_enum(), DATATYPE::STRING, 2,
                                    MemberFuncType::REGEX_SUB);
  // If argument not present or 0 then replace all occurrences
  int allowed_replacements = 0;
  if (arguments.size() > 3) {
    allowed_replacements = arguments[3]->get_int32();
    if (allowed_replacements < 0) {
      THROW("%s", "re.sub expects max number of replacements to be a non-negative integer.");
    }
  }
  std::string input = arguments[2]->get_string();
  std::string replacement_string = arguments[1]->get_string();
  std::string pattern = arguments[0]->get_string();
  std::regex re(pattern);
  if (allowed_replacements == 0) {
    std::string result = std::regex_replace(input, re, replacement_string);
    return OpReturnType(new SingleVariable<std::string>(result));
  }
  std::string result = input;
  int replacements_made = 0;
  std::smatch match;
  std::string::const_iterator search_start(result.cbegin());

  while (std::regex_search(search_start, result.cend(), match, re)) {
    if (replacements_made >= allowed_replacements) {
      break;
    }
    result.replace(match.position(0) + std::distance(result.cbegin(), search_start),
                   match.length(0), replacement_string);
    replacements_made++;
    search_start = result.cbegin() + std::distance(result.cbegin(), search_start) +
                   match.position(0) + replacement_string.length();
  }
  return OpReturnType(new SingleVariable<std::string>(result));
}

OpReturnType RegexDataVariable::regex_subn(const std::vector<OpReturnType>& arguments,
                                           CallStack& stack) {
  THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(arguments.size(), 3, 4, MemberFuncType::REGEX_SUBN);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::REGEX_SUBN);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::REGEX_SUBN);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[2]->get_dataType_enum(), DATATYPE::STRING, 2,
                                    MemberFuncType::REGEX_SUBN);
  int allowed_replacements = INT_MAX;
  if (arguments.size() > 3) {
    allowed_replacements = arguments[3]->get_int32();
    if (allowed_replacements < 0) {
      THROW("%s", "re.subn expects max number of replacements to be a non-negative integer.");
    }
  }
  std::string input = arguments[2]->get_string();
  std::string replacement_string = arguments[1]->get_string();
  std::string pattern = arguments[0]->get_string();
  std::regex re(pattern);
  std::string result = input;
  int replacements_made = 0;
  std::smatch match;
  std::string::const_iterator search_start(result.cbegin());

  while (std::regex_search(search_start, result.cend(), match, re)) {
    if (replacements_made >= allowed_replacements) {
      break;
    }
    result.replace(match.position(0) + std::distance(result.cbegin(), search_start),
                   match.length(0), replacement_string);
    replacements_made++;
    search_start = result.cbegin() + std::distance(result.cbegin(), search_start) +
                   match.position(0) + replacement_string.length();
  }
  std::vector<OpReturnType> list = {OpReturnType(new SingleVariable<std::string>(result)),
                                    OpReturnType(new SingleVariable<int32_t>(replacements_made))};
  return OpReturnType(new TupleDataVariable(std::move(list)));
}

OpReturnType RegexDataVariable::call_function(int memberFuncIndex,
                                              const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::REGEX_MATCH:
      return regex_match(arguments, stack);
    case MemberFuncType::REGEX_SEARCH:
      return regex_search(arguments, stack);
    case MemberFuncType::REGEX_FULLMATCH:
      return regex_fullmatch(arguments, stack);
    case MemberFuncType::REGEX_SPLIT:
      return regex_split(arguments, stack);
    case MemberFuncType::REGEX_FINDALL:
      return regex_findall(arguments, stack);
    case MemberFuncType::REGEX_FINDITER:
      return regex_finditer(arguments, stack);
    case MemberFuncType::REGEX_SUB:
      return regex_sub(arguments, stack);
    case MemberFuncType::REGEX_SUBN:
      return regex_subn(arguments, stack);
    default:
      THROW("%s not implemented for ne_re", DataVariable::get_member_func_string(memberFuncIndex));
  }
};

#endif  // REGEX_ENABLED
