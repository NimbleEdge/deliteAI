/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "data_variable.hpp"
#include "single_variable.hpp"

typedef OpReturnType (*BoolFuncPtr)(OpReturnType, OpReturnType);

/**
 * @brief Template class for boolean operations
 *
 * Provides a generic framework for implementing boolean operations
 * between two DataVariable objects.
 *
 * @tparam Oper The boolean operator class (e.g., AndOp, OrOp)
 */
template <class Oper>
class BoolOp {
 public:
  static OpReturnType operate(OpReturnType v1, OpReturnType v2);
};

template <class Oper>
OpReturnType BoolOp<Oper>::operate(OpReturnType v1, OpReturnType v2) {
  if (v1->is_single() && v2->is_single()) {
    auto val1 = v1->get_bool();
    auto val2 = v2->get_bool();
    return Oper::operate_single(val1, val2);
  }
  return nullptr;
}

class AndOp {
 public:
  static OpReturnType operate_single(bool v1, bool v2) {
    return OpReturnType(new SingleVariable<bool>(v1 && v2));
  }
};

class OrOp {
 public:
  static OpReturnType operate_single(bool v1, bool v2) {
    return OpReturnType(new SingleVariable<bool>(v1 || v2));
  }
};

class BoolOperators {
  static std::map<std::string, BoolFuncPtr> _boolOpMap;

 public:
  template <class Oper>
  static OpReturnType operate(OpReturnType v1, OpReturnType v2) {
    return BoolOp<Oper>::operate(v1, v2);
  }

  static BoolFuncPtr get_operator(const std::string& opType);
};
