#pragma once
#include "data_variable.hpp"

// Abstract base class for all iterable data types
class IterableDataVariable : public DataVariable {
 protected:
  // Current position in the iteration
  int _iterPosition = 0;
  bool _iterExhausted = false;

 public:
  // Reset the iterator to start from the beginning
  virtual void reset_iterator() {
    _iterPosition = 0;
    _iterExhausted = false;
  }

  // Get the next value in the iteration, or throw StopIteration
  virtual OpReturnType next(CallStack& stack) override {
    if (_iterExhausted || _iterPosition >= get_size()) {
      _iterExhausted = true;
      THROW("StopIteration");
    }
    return get_int_subscript(_iterPosition++);
  }

  // Check if the iterator is exhausted
  bool is_exhausted() const { return _iterExhausted; }
};

class IterableOverScriptable : public IterableDataVariable {
  OpReturnType _data = nullptr;

 public:
  IterableOverScriptable(OpReturnType data) {
    if (data->get_containerType() != CONTAINERTYPE::LIST &&
        data->get_containerType() != CONTAINERTYPE::TUPLE &&
        data->get_containerType() != CONTAINERTYPE::RANGE) {
      THROW("IterableOverScriptable requires a list or tuple or range got %s",
            data->get_containerType_string());
    }
    _data = data;
  }

  IterableOverScriptable() {}

  OpReturnType next(CallStack& stack) override {
    if (_iterExhausted || _iterPosition >= get_size()) {
      _iterExhausted = true;
      THROW("StopIteration");
    }
    return _data->get_int_subscript(_iterPosition++);
  }

  void set_data(OpReturnType data) {
    if (data->get_containerType() == CONTAINERTYPE::SINGLE &&
        data->get_dataType_enum() == DATATYPE::STRING) {
      _data = data;
      return;
    }

    if (data->get_containerType() != CONTAINERTYPE::LIST &&
        data->get_containerType() != CONTAINERTYPE::TUPLE &&
        data->get_containerType() != CONTAINERTYPE::RANGE) {
      THROW("IterableOverScriptable requires a list or tuple or range got %s",
            data->get_containerType_string());
    }
    _data = data;
  }

  int get_containerType() const override { return _data->get_containerType(); }

  int get_dataType_enum() const override { return _data->get_dataType_enum(); }

  int get_size() override { return _data->get_size(); }

  OpReturnType get_int_subscript(int index) override { return _data->get_int_subscript(index); }

  std::string print() override { return _data->print(); }

  nlohmann::json to_json() const override { return _data->to_json(); }

  bool get_bool() override { return _data->get_bool(); }
};