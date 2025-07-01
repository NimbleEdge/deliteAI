/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ios_reference_data_variable.hpp"

#include "data_variable.hpp"
#include "executor_structs.h"
#include "ios_helper.hpp"
#include "nimble_net_util.hpp"
#include "util.hpp"

IOSReferenceDataVariable::IOSReferenceDataVariable(IosObject&& obj) : _iosObj{std::move(obj)} {}

IOSReferenceDataVariable::~IOSReferenceDataVariable() { IOSHelper::release(_iosObj); }

OpReturnType IOSReferenceDataVariable::get_string_subscript(const std::string& key) {
  CTensor child = IOSHelper::get_string_subscript(_iosObj, key);

  OpReturnType ret;
  if (child.shape) {
    ret = DataVariable::create_tensor(child, CreateTensorType::COPY);
  } else {
    ret = DataVariable::create_single_variable(child);
  }
  IOSHelper::deallocate_cTensor(&child);
  return ret;
}

OpReturnType IOSReferenceDataVariable::get_int_subscript(int idx) {
  CTensor child = IOSHelper::get_int_subscript(_iosObj, idx);

  OpReturnType ret;
  if (child.shape) {
    ret = DataVariable::create_tensor(child, CreateTensorType::COPY);
  } else {
    ret = DataVariable::create_single_variable(child);
  }
  IOSHelper::deallocate_cTensor(&child);
  return ret;
}

int IOSReferenceDataVariable::get_size() { return IOSHelper::get_size(_iosObj); }

void* IOSReferenceDataVariable::get_raw_ptr() { return &_iosObj; }

void IOSReferenceDataVariable::set_subscript(const OpReturnType& subscript,
                                             const OpReturnType& val) {
  if (subscript->is_string()) {
    IOSHelper::set_subscript(_iosObj, subscript->get_string(), val);
  } else {
    IOSHelper::set_subscript(_iosObj, subscript->get_int32(), val);
  }
}

std::string IOSReferenceDataVariable::common_print() const { return IOSHelper::to_string(_iosObj); }

OpReturnType IOSReferenceDataVariable::arrange(const OpReturnType argument) {
  if (argument->get_containerType() != CONTAINERTYPE::VECTOR &&
      argument->get_containerType() != CONTAINERTYPE::LIST) {
    THROW("Argument of arrange should be a tensor/list, provided %s",
          argument->get_containerType_string());
  }
  if (argument->get_containerType() == CONTAINERTYPE::VECTOR && argument->get_shape().size() != 1) {
    THROW("Argument of arrange if tensor, should be of dimension 1, provided %d dimensions",
          argument->get_shape().size());
  }

  int mySize = get_size();
  int argSize = argument->get_size();
  if (argSize > mySize) {
    THROW(
        "Elements present in argument of arrange should less than or equal to elements present in "
        "tensor, provided %d elements for a tensor of size %d",
        argSize, mySize);
  }

  std::vector<int> indices;
  indices.reserve(argSize);
  for (int i = 0; i < argSize; i++) {
    OpReturnType index = argument->get_int_subscript(i);
    if (!index->is_integer()) {
      THROW("Element present in argument of arrange at index=%d should be of type int, provided %s",
            i, util::get_string_from_enum(index->get_dataType_enum()));
    }
    if (index->get_int32() < 0 || index->get_int32() >= mySize) {
      THROW("Tried to access %d index of the list", index->get_int32());
    }
    indices.emplace_back(index->get_int32());
  }

  IosObject newObj = IOSHelper::arrange(_iosObj, indices);
  return OpReturnType{new IOSReferenceDataVariable(std::move(newObj))};
}

bool IOSReferenceDataVariable::in(const OpReturnType& elem) {
  const auto key = elem->get_string();
  return IOSHelper::in(_iosObj, key);
}

OpReturnType IOSReferenceDataVariable::call_function(int memberFuncIndex,
                                                     const std::vector<OpReturnType>& arguments,
                                                     CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::KEYS: {
      THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, MemberFuncType::KEYS);
      CTensor cTensor = IOSHelper::get_keys(_iosObj);
      if (!cTensor.shape) {
        THROW("Expected a tensor from IOS, instead got null shape. Data type: %s",
              util::get_string_from_enum(cTensor.dataType));
      }
      auto ret = DataVariable::create_tensor(cTensor, CreateTensorType::COPY);
      IOSHelper::deallocate_cTensor(&cTensor);
      return ret;
    }
  }
  return DataVariable::call_function(memberFuncIndex, arguments, stack);
}
