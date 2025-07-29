/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "compare_operators.hpp"

std::map<std::string, CompareFuncPtr> CompareOperators::_compareOpMap = {
    {"Eq", CompareOperators::operate<EqualOp>},
    {"Gt", CompareOperators::operate<GreaterOp>},
    {"GtE", CompareOperators::operate<GreaterEqualOp>},
    {"Lt", CompareOperators::operate<LessThanOp>},
    {"LtE", CompareOperators::operate<LessThanEqualOp>},
    {"In", CompareOperators::in},
    {"NotEq", CompareOperators::operate<NotEqualOp>},
    {"NotIn", CompareOperators::notIn},
    {"Is", CompareOperators::is},
    {"IsNot", CompareOperators::isNot},
};

CompareFuncPtr CompareOperators::get_operator(const std::string& opType) {
  if (_compareOpMap.find(opType) == _compareOpMap.end()) {
    THROW("compareOp=%s not found", opType.c_str());
  }
  return _compareOpMap[opType];
}
