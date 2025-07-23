/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "enumerate_data_variable.hpp"

#include <stdexcept>
#include <string>

#include "single_variable.hpp"
#include "tuple_data_variable.hpp"

EnumerateDataVariable::EnumerateDataVariable(OpReturnType iterable, int startIndex)
    : _iterable(iterable), _startIndex(startIndex) {
  if (!_iterable->is_iterable() && _iterable->get_containerType() != CONTAINERTYPE::LIST &&
      _iterable->get_containerType() != CONTAINERTYPE::TUPLE) {
    THROW("enumerate expects an iterable argument, provided %s",
          _iterable->get_containerType_string());
  }

  _size = _iterable->get_size();
}

OpReturnType EnumerateDataVariable::get_int_subscript(int index) {
  if (index >= _size || index < 0) {
    THROW("accessing %d of Enumerate with size=%d", index, _size);
  }

  std::vector<OpReturnType> tupleMembers(2);
  tupleMembers[0] = OpReturnType(new SingleVariable<int32_t>(_startIndex + index));
  tupleMembers[1] = _iterable->get_int_subscript(index);

  return OpReturnType(new TupleDataVariable(tupleMembers));
}

void EnumerateDataVariable::reset_iterator() {
  IterableDataVariable::reset_iterator();
  if (_iterable->is_iterable()) {
    static_cast<IterableDataVariable*>(_iterable.get())->reset_iterator();
  }
}

OpReturnType EnumerateDataVariable::next(CallStack& stack) {
  if (_iterExhausted) {
    THROW("StopIteration");
  }

  if (_iterPosition >= _size) {
    _iterExhausted = true;
    THROW("StopIteration");
  }

  OpReturnType value = _iterable->get_int_subscript(_iterPosition);

  std::vector<OpReturnType> tupleMembers(2);
  tupleMembers[0] = OpReturnType(new SingleVariable<int32_t>(_startIndex + _iterPosition));
  tupleMembers[1] = value;

  _iterPosition++;
  return OpReturnType(new TupleDataVariable(tupleMembers));
}

bool EnumerateDataVariable::get_bool() {
  return _size > 0;
}
