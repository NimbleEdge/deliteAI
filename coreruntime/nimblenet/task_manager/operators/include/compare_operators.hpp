#pragma once
#include "data_variable.hpp"
#include "operator_types.hpp"
#include "single_variable.hpp"

typedef OpReturnType (*CompareFuncPtr)(OpReturnType, OpReturnType);

template <template <typename T> class Oper, typename T>
class CompareOp {
 public:
  static OpReturnType operate(OpReturnType v1, OpReturnType v2);
};

template <typename T>
class EqualOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 == val2));
  }
};

template <typename T>
class GreaterOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 > val2));
  }
};

template <typename T>
class GreaterEqualOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 >= val2));
  }
};

template <typename T>
class LessThanOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 < val2));
  }
};

template <typename T>
class LessThanEqualOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 <= val2));
  }
};

template <typename T>
class NotEqualOp {
 public:
  static OpReturnType operate_single(T val1, T val2) {
    return OpReturnType(new SingleVariable<bool>(val1 != val2));
  }
};

class CompareOperators {
  static std::map<std::string, CompareFuncPtr> _compareOpMap;

 public:
  template <template <typename T> class Oper>
  static OpReturnType operate(OpReturnType v1, OpReturnType v2) {
    if (v1->is_numeric() && v2->is_numeric()) {
      auto returnType = get_max_dataType(v1->get_dataType_enum(), v2->get_dataType_enum());

      switch (returnType) {
        case DATATYPE::FLOAT:
          return CompareOp<Oper, float>::operate(v1, v2);
        case DATATYPE::INT32:
          return CompareOp<Oper, int32_t>::operate(v1, v2);
        case DATATYPE::DOUBLE:
          return CompareOp<Oper, double>::operate(v1, v2);
        case DATATYPE::INT64:
          return CompareOp<Oper, int64_t>::operate(v1, v2);
      }
    } else if (v1->is_string() && v2->is_string()) {
      return CompareOp<Oper, std::string>::operate(v1, v2);
    }
    // TODO: How to add comparing a variable with None for e.g. if var == None:
    return nullptr;
  }

  static CompareFuncPtr get_operator(const std::string& opType);

  static OpReturnType in(OpReturnType v1, OpReturnType v2) {
    return OpReturnType(new SingleVariable<bool>(v2->in(v1)));
  }

  static OpReturnType notIn(OpReturnType v1, OpReturnType v2) {
    return OpReturnType(new SingleVariable<bool>(v2->notIn(v1)));
  }
};

template <template <typename Type> class Operation, class T>
OpReturnType CompareOp<Operation, T>::operate(OpReturnType v1, OpReturnType v2) {
  if (v1->is_single() && v2->is_single()) {
    auto val1 = v1->get<T>();
    auto val2 = v2->get<T>();
    return Operation<T>::operate_single(val1, val2);
  }
  return nullptr;
}
