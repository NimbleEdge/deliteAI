/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <type_traits>

#include "binary_operators.hpp"
#include "data_variable.hpp"
#include "single_variable.hpp"
#include "tensor_data_variable.hpp"

// think of better way to do this, if required
template <class T>
T get_element(OpReturnType d, const std::vector<int64_t>& shape, int index, int size) {
  int elements = index;
  auto elem = d;
  int curSize = size;
  for (int i = 0; i < shape.size(); i++) {
    if (shape[i] != elem->get_size()) {
      THROW("%s", "Shape of list not consistent");
    }
    curSize = curSize / shape[i];
    int dimIndex = elements / curSize;
    elements -= dimIndex * curSize;
    elem = elem->get_int_subscript(dimIndex);
  }
  return elem->get<T>();
}

/**
 * @brief Static utility class for list-to-tensor conversion operations
 *
 * ListOperators provides static methods to convert ListDataVariable objects
 * into tensor representations. It handles different data types including
 * numeric types and strings, with specialized handling for string tensors.
 * The class uses template specialization to efficiently extract elements
 * from nested list structures and create appropriately typed tensors.
 */
class ListOperators {
 public:
  /**
   * @brief Converts a list to a tensor of the specified type
   * @tparam T The target data type for the tensor
   * @param list The list to convert
   * @param shape The shape of the resulting tensor
   * @param size Total number of elements in the tensor
   * @return OpReturnType containing the created tensor
   * @note Uses move semantics for the shape parameter to avoid copying
   */
  template <class T>
  static OpReturnType operate(OpReturnType list, std::vector<int64_t>&& shape, int size) {
    T* data = (T*)malloc(size * sizeof(T));
    for (int i = 0; i < size; i++) {
      data[i] = get_element<T>(list, shape, i, size);
    }
    return OpReturnType(new TensorVariable(data, static_cast<DATATYPE>(get_dataType_enum<T>()),
                                           shape, CreateTensorType::MOVE));
  }

  static OpReturnType operate_string(OpReturnType list, std::vector<int64_t>&& shape, int size);
  static OpReturnType create_tensor(int dataType, OpReturnType list);
};

class ListDataVariable : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::LIST; }

  std::vector<OpReturnType> _members; /**< Vector containing the list elements */
  std::vector<int64_t> _shape;        /**< Shape information for the list */

  int get_dataType_enum() const override { return DATATYPE::EMPTY; }

  OpReturnType get_int_subscript(int index) override {
    int index_ = index;
    if (index_ < 0) {
      index_ += _members.size();
    }
    if (index_ >= _members.size() || index_ < 0) {
      THROW("trying to access %d index for list of size=%d", index, _members.size());
    }
    return _members[index_];
  }

  int get_size() override { return _members.size(); }

  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override {
    int index = subscriptVal->get_int32();
    if (index >= _members.size() || index < 0) {
      THROW("trying to set %d index for list of size=%d", index, _members.size());
    }
    _members[index] = d;
  }

  // Override get_subscript to handle both integer indices and slice objects
  OpReturnType get_subscript(const OpReturnType& subscriptVal) override {
    // If the subscript is a SliceVariable, handle slicing
    if (subscriptVal->get_containerType() == CONTAINERTYPE::SLICE) {
      return get_slice_subscript(subscriptVal);
    }
    // Otherwise, just delegate to the integer index handler
    return get_int_subscript(subscriptVal->get_int32());
  }

  /**
   * @brief Handles slice operations on the list
   * @param sliceObj The slice object containing start, stop, and step parameters
   * @return OpReturnType containing a new list with the sliced elements
   * @details This method implements Python-style slicing with support for:
   *          - Negative indices (counting from the end)
   *          - Positive and negative step values
   *          - Automatic bounds checking and clamping
   *          - Efficient memory allocation using reserve()
   *          - Move semantics to avoid unnecessary copying
   */
  OpReturnType get_slice_subscript(const OpReturnType& sliceObj) {
    // Cast to SliceVariable to use direct member access
    const ListSliceVariable* slice = static_cast<const ListSliceVariable*>(sliceObj.get());

    // Extract slice parameters (start, stop, step)
    const int size = _members.size();
    int start = slice->get_start(size);
    int stop = slice->get_stop(size);
    int step = slice->get_step();

    // Optimize: Calculate slice size to pre-allocate memory
    std::vector<OpReturnType> slicedMembers;

    int sliceSize = 0;
    if (step > 0) {
      // For positive step
      sliceSize = (stop > start) ? (stop - start + step - 1) / step : 0;
    } else {
      // For negative step
      sliceSize = (start > stop) ? (start - stop - step - 1) / (-step) : 0;
    }

    sliceSize = std::max(0, std::min(sliceSize, size));
    slicedMembers.reserve(sliceSize);

    if (step > 0) {
      for (int i = start; i < stop; i += step) {
        if (i >= 0 && i < size) {
          slicedMembers.push_back(_members[i]);
        }
      }
    } else {
      for (int i = start; i > stop; i += step) {
        if (i >= 0 && i < size) {
          slicedMembers.push_back(_members[i]);
        }
      }
    }

    // Create a new list with the sliced elements - move semantics to avoid copying
    return OpReturnType(new ListDataVariable(std::move(slicedMembers)));
  }

  std::string print() override {
    std::string output = "[";
    for (int i = 0; i < _members.size(); i++) {
      if (i != 0) output += ",";
      output += _members[i]->print();
    }
    output += "]";
    return output;
  }

  nlohmann::json to_json() const override {
    auto output = nlohmann::json::array();
    for (const auto& member : _members) {
      output.push_back(member->to_json());
    }
    return output;
  }

  const std::vector<int64_t>& get_shape() override { return _shape; }

  OpReturnType arrange(const OpReturnType argument) override {
    if (argument->get_containerType() != CONTAINERTYPE::VECTOR &&
        argument->get_containerType() != CONTAINERTYPE::LIST) {
      THROW("Argument of arrange should be a tensor/list, provided %s",
            argument->get_containerType_string());
    }
    if (argument->get_containerType() == CONTAINERTYPE::VECTOR &&
        argument->get_shape().size() != 1) {
      THROW("Argument of arrange if tensor, should be of dimension 1, provided %d dimensions",
            argument->get_shape().size());
    }
    if (_shape.size() != 1) {
      THROW("arrange expects tensor to be of 1 dimension. Given %d dimensions.", _shape.size());
    }
    int size = argument->get_size();
    if (size > _shape[0]) {
      THROW(
          "Elements present in argument of arrange should less than or equal to elements present "
          "in "
          "tensor, provided %d elements for a tensor of size %d",
          size, _shape[0]);
    }
    OpReturnType list = OpReturnType(new ListDataVariable());
    for (int i = 0; i < size; i++) {
      OpReturnType index = argument->get_int_subscript(i);
      if (!index->is_integer()) {
        THROW(
            "Element present in argument of arrange at index=%d should be of type int, provided %s",
            i, util::get_string_from_enum(index->get_dataType_enum()));
      }
      if (index->get_int32() < 0 || index->get_int32() >= _shape[0]) {
        THROW("Tried to access %d index of the tensor.", index->get_int32());
      }
      list->append(_members[index->get_int32()]);
    }
    return list;
  }

  int get_numElements() override { return _members.size(); }

  JsonIterator* get_json_iterator() override {
    return new JsonIterator(_members.begin(), _members.end());
  }

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override {
    switch (memberFuncIndex) {
      case MemberFuncType::POP: {
        THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, memberFuncIndex);
        int index = arguments[0]->get_int32();
        if (index >= _members.size() || index < 0) {
          THROW("Trying to delete %d index of list of size=%d", index, _members.size());
        }
        auto value = _members[index];
        _members.erase(_members.begin() + index);
        return value;
      }
      default: {
        return DataVariable::call_function(memberFuncIndex, arguments, stack);
      }
    }
  }

 public:
  ListDataVariable(std::vector<OpReturnType>&& members) {
    _members = std::move(members);
    _shape.push_back(_members.size());
  }

  ListDataVariable() { _shape.push_back(0); }

  template <class T, class = std::enable_if_t<!std::is_same_v<T, OpReturnType>>>
  ListDataVariable(const std::vector<T>& input) {
    for (int i = 0; i < input.size(); i++) {
      _members.push_back(OpReturnType(new SingleVariable<T>(input[i])));
    }
    _shape.push_back(_members.size());
  }

  bool get_bool() override { return get_size() > 0; }

  std::vector<OpReturnType> get_members() { return _members; }

  OpReturnType append(OpReturnType d) override {
    _members.push_back(d);
    _shape.back()++;
    return shared_from_this();
  }

  /*
  DELITEPY_DOC_BLOCK_BEGIN

def tensor(list: list[any], dtype: str) -> Tensor:
    """
    Creates and return a tensor from the list provided and the desired data type of the tensor. The list should have all the elements of the same data types, as supported by dtype argument.

    Parameters
    ----------
    list : list[any]
        List to be used to create the tensor.
    dtype : str
        Data type with which to create the tensor.

    Returns
    ----------
    tensor : Tensor
        Returns the tensor of the shape and datatype filled with values from the list.
    """
    pass
  DELITEPY_DOC_BLOCK_END
  */
  OpReturnType to_tensor(OpReturnType d) override {
    std::string dataType = d->get_string();
    int dType = util::get_enum_from_string(dataType.c_str());
    if (dType == -1) {
      THROW("%s is not a dataType", dataType.c_str());
    }
    if (dType != DATATYPE::INT32 && dType != DATATYPE::INT64 && dType != DATATYPE::STRING &&
        dType != DATATYPE::DOUBLE && dType != DATATYPE::FLOAT) {
      THROW("%s dataType is not supported for nm.tensor()", util::get_string_from_enum(dType));
    }
    return ListOperators::create_tensor(dType, shared_from_this());
  }

  bool in(const OpReturnType& elem) override {
    for (int i = 0; i < _members.size(); i++) {
      if (BaseBinOp::compare_equal(get_int_subscript(i), elem)) {
        return true;
      }
    }
    return false;
  }
};
