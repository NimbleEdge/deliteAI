#pragma once
#include "data_variable.hpp"
#include "single_variable.hpp"

typedef OpReturnType (*UnaryOpFuncPtr)(OpReturnType);

class UnaryOperators {
  static std::map<std::string, UnaryOpFuncPtr> _unaryOpMap;

 public:
  static UnaryOpFuncPtr get_operator(const std::string& opType);

  static OpReturnType inverse_bool(OpReturnType v) {
    bool val = v->get_bool();
    return OpReturnType(new SingleVariable<bool>(!val));
  }

  static OpReturnType unary_sub(OpReturnType v) { return v->unary_sub(); }
};
