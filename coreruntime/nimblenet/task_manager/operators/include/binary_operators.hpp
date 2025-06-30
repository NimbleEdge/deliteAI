#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <type_traits>

#include "data_variable.hpp"
#include "logger.hpp"
#include "ne_type_traits.hpp"
#include "nimble_net_util.hpp"
#include "operator_types.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"
#include "util.hpp"

class BaseBinOp {
 public:
  // Helper function to compare two DataVariable objects for equality
  static bool compare_equal(const OpReturnType& a, const OpReturnType& b) {
    // If the container types are different, they're not equal
    if (a->get_containerType() != b->get_containerType()) {
      return false;
    }

    // If they're both single variables, compare their values
    if (a->is_single() && b->is_single()) {
      // For basic types, try a simple comparison
      if (a->get_dataType_enum() == b->get_dataType_enum()) {
        switch (a->get_dataType_enum()) {
          case DATATYPE::INT32:
            return a->get_int32() == b->get_int32();
          case DATATYPE::INT64:
            return a->get_int64() == b->get_int64();
          case DATATYPE::FLOAT:
            return a->get_float() == b->get_float();
          case DATATYPE::DOUBLE:
            return a->get_double() == b->get_double();
          case DATATYPE::STRING:
            return a->get_string() == b->get_string();
          case DATATYPE::BOOLEAN:
            return a->get_bool() == b->get_bool();
          default:
            // For other types, we need a more complex comparison
            return a->print() == b->print();
        }
      }
      return false;
    }

    // For lists, compare each element
    if (a->get_containerType() == CONTAINERTYPE::LIST &&
        b->get_containerType() == CONTAINERTYPE::LIST) {
      if (a->get_size() != b->get_size()) {
        return false;
      }

      for (int i = 0; i < a->get_size(); i++) {
        if (!compare_equal(a->get_int_subscript(i), b->get_int_subscript(i))) {
          return false;
        }
      }
      return true;
    }

    // For maps, compare each key-value pair
    if (a->get_containerType() == CONTAINERTYPE::MAP &&
        b->get_containerType() == CONTAINERTYPE::MAP) {
      if (a->get_size() != b->get_size()) {
        return false;
      }

      const auto& map_a = a->get_map();
      const auto& map_b = b->get_map();

      for (const auto& [key, value] : map_a) {
        auto it_b = map_b.find(key);
        if (it_b == map_b.end()) {
          return false;  // Key not found in map b
        }
        if (!compare_equal(value, it_b->second)) {
          return false;  // Values don't match
        }
      }
      return true;
    }

    // For other container types, compare their string representation
    return a->print() == b->print();
  }

  OpReturnType perform_operation(OpReturnType val1, OpReturnType val2, std::string& opType) {
    if (opType == "Add") {
      return add(val1, val2);
    } else if (opType == "Sub") {
      return sub(val1, val2);
    } else if (opType == "Mult") {
      return mult(val1, val2);
    } else if (opType == "Div") {
      return div(val1, val2);
    } else if (opType == "Pow") {
      return pow(val1, val2);
    } else if (opType == "Mod") {
      return mod(val1, val2);
    }
    return nullptr;
  }

  virtual OpReturnType add(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual OpReturnType sub(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual OpReturnType mult(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual OpReturnType div(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual OpReturnType pow(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual OpReturnType mod(OpReturnType val1, OpReturnType val2) const { return nullptr; }

  virtual ~BaseBinOp() = default;
};

// Special implementation for modulo operation
template <typename T,
          typename = std::enable_if_t<ne::is_one_of_v<T, float, int32_t, double, int64_t>>>
struct ModOperator {
  static T compute(T a, T b) {
    auto result = std::fmod(a, b);  // Works for integer types
    if (result < 0 && b > 0) {
      result += b;
    }
    return result;
  }
};

template <typename T,
          typename = std::enable_if_t<ne::is_one_of_v<T, float, int32_t, double, int64_t>>>
class NumericBinOp : public BaseBinOp {
 public:
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() + val2->get<T>()));
  }

  OpReturnType sub(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() - val2->get<T>()));
  }

  OpReturnType mult(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(val1->get<T>() * val2->get<T>()));
  }

  OpReturnType div(OpReturnType val1, OpReturnType val2) const override {
    if (val2->get<T>() == (T)0) {
      THROW("%s", "Division by zero will result in undefined behaviour.");
    }
    return OpReturnType(new SingleVariable<T>(val1->get<T>() / val2->get<T>()));
  }

  OpReturnType pow(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<T>(std::pow(val1->get<T>(), val2->get<T>())));
  }

  OpReturnType mod(OpReturnType val1, OpReturnType val2) const override {
    if (val2->get<T>() == (T)0) {
      THROW("%s", "Modulo by zero error.");
    }
    T result = ModOperator<T>::compute(val1->get<T>(), val2->get<T>());
    return OpReturnType(new SingleVariable<T>(result));
  }
};

class StringBinOp : public BaseBinOp {
 public:
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override {
    return OpReturnType(new SingleVariable<std::string>(val1->get_string() + val2->get_string()));
  }
};

class ListBinOp : public BaseBinOp {
 public:
  // List concatenation (binary add)
  OpReturnType add(OpReturnType val1, OpReturnType val2) const override;

  // List repetition (binary mult)
  OpReturnType mult(OpReturnType val1, OpReturnType val2) const override;
};

class BinaryOperators {
 public:
  static OpReturnType operate(OpReturnType v1, OpReturnType v2, std::string& opType) {
    // First, check for list operations
    if (v1->get_containerType() == CONTAINERTYPE::LIST ||
        v2->get_containerType() == CONTAINERTYPE::LIST) {
      static ListBinOp listOp;
      return listOp.perform_operation(v1, v2, opType);
    } else if (auto t1 = std::dynamic_pointer_cast<BaseTensorVariable>(v1),
               t2 = std::dynamic_pointer_cast<BaseTensorVariable>(v2);
               t1 && t2) {
      THROW("%s", "tensor ops not supported");
    } else if (v1->is_numeric() && v2->is_numeric()) {
      auto returnType = get_max_dataType(v1->get_dataType_enum(), v2->get_dataType_enum());
      switch (returnType) {
        case DATATYPE::FLOAT: {
          NumericBinOp<float> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::INT32: {
          NumericBinOp<int32_t> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::DOUBLE: {
          NumericBinOp<double> n;
          return n.perform_operation(v1, v2, opType);
        }
        case DATATYPE::INT64: {
          NumericBinOp<int64_t> n;
          return n.perform_operation(v1, v2, opType);
        }
      }
    } else if (v1->is_string() && v2->is_string()) {
      StringBinOp s;
      return s.perform_operation(v1, v2, opType);
    }

    return nullptr;
  }
};
