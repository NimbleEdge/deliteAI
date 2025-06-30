/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "bool_operators.hpp"

std::map<std::string, BoolFuncPtr> BoolOperators::_boolOpMap = {
    {"And", BoolOperators::operate<AndOp>}, {"Or", BoolOperators::operate<OrOp>}};

BoolFuncPtr BoolOperators::get_operator(const std::string& opType) {
  if (_boolOpMap.find(opType) == _boolOpMap.end()) {
    THROW("boolOp=%s not found", opType.c_str());
  }
  return _boolOpMap[opType];
}
