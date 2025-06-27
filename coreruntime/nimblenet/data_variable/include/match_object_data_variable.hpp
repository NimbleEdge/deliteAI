#pragma once

#include <regex>

#include "data_variable.hpp"

class MatchObjectDataVariable : public DataVariable {
  std::smatch _smatch;
  std::shared_ptr<std::string> _input;

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
  MatchObjectDataVariable(std::smatch&& smatch, std::shared_ptr<std::string> input)
      : _smatch(std::move(smatch)), _input(std::move(input)) {}

  std::string print() override { return fallback_print(); }
};
