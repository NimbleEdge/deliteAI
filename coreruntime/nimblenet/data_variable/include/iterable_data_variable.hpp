/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"

/**
 * @brief Abstract base class for all iterable data types
 *
 * IterableDataVariable provides a unified interface for iterating over
 * different types of data structures. It maintains iteration state including
 * current position and exhaustion status, and provides methods to reset
 * iteration and check if iteration is complete.
 */
class IterableDataVariable : public DataVariable {
 protected:
  int _iterPosition = 0;    /**< Current position in the iteration */
  bool _iterExhausted = false; /**< Flag indicating if iteration has been exhausted */

 public:
  /**
   * @brief Indicates whether the variable is iterable
   *
   * @return Always returns true
   */
  bool is_iterable() const override { return true; }

  /**
   * @brief Resets the iterator to start from the beginning
   */
  virtual void reset_iterator() {
    _iterPosition = 0;
    _iterExhausted = false;
  }

  /**
   * @brief Retrieves the next value in the iteration
   *
   * @param stack The current execution call stack
   * @return The next element in the iteration
   * @throws StopIteration if the iterator is exhausted
   */
  virtual OpReturnType next(CallStack& stack) override {
    if (_iterExhausted || _iterPosition >= get_size()) {
      _iterExhausted = true;
      THROW("StopIteration");
    }
    return get_int_subscript(_iterPosition++);
  }

  /**
   * @brief Checks if the iterator is exhausted
   *
   * @return true if iteration is complete, false otherwise
   */
  bool is_exhausted() const { return _iterExhausted; }
};

/**
 * @brief Iterator wrapper for scriptable data containers
 *
 * IterableOverScriptable provides iteration capabilities over scriptable
 * data types such as lists, tuples, and ranges. It acts as a proxy that
 * delegates iteration operations to the underlying data container while
 * maintaining the iteration state.
 */
class IterableOverScriptable : public IterableDataVariable {
  OpReturnType _data = nullptr; /**< Reference to the underlying data container */

 public:
  /**
   * @brief Constructs an iterator over a scriptable data container
   * @param data The data container to iterate over (must be LIST, TUPLE, or RANGE)
   * @throws Exception if data is not a supported container type
   */
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

  /**
   * @brief Sets the data container to iterate over
   * @param data The data container (must be LIST, TUPLE, RANGE, or single STRING)
   * @throws Exception if data is not a supported container type
   */
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
