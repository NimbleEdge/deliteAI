/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimble_net/c_tensor.h"

#include <cstdlib>
#include <cstring>

#include "nimble_net_util.hpp"

namespace {

template <typename T>
T* c_tensor_create_singular_numeric_data(T value) {
  T* data = static_cast<T*>(malloc(sizeof(T)));
  if (data) {
    *data = value;
  }
  return data;
}

template <typename T>
T c_tensor_get_singular_numeric_data(void* data) {
  return *static_cast<T*>(data);
}

void c_tensor_delete_singular_numeric_data(void* data) { free(data); }

// =================================================================================================

void c_tensor_delete_string_data(CTensor* tensor) {
  char** str_arr = static_cast<char**>(tensor->data);

  int str_arr_len = 1;
  for (int i = 0; i < tensor->shapeLength; ++i) {
    str_arr_len *= tensor->shape[i];
  }

  for (int i = 0; i < str_arr_len; ++i) {
    free(str_arr[i]);
  }
  free(str_arr);
}

}  // namespace

// =================================================================================================

void c_tensor_init(CTensor* tensor) {
  tensor->name = nullptr;
  tensor->data = nullptr;
  tensor->dataType = DATATYPE::UNKNOWN;
  tensor->shape = nullptr;
  tensor->shapeLength = 0;
}

// =================================================================================================

bool* c_tensor_create_boolean_data(bool value) {
  return c_tensor_create_singular_numeric_data<bool>(value);
}

bool c_tensor_get_boolean_data(void* data) {
  return c_tensor_get_singular_numeric_data<bool>(data);
}

// =================================================================================================

int32_t* c_tensor_create_int32_data(int32_t value) {
  return c_tensor_create_singular_numeric_data<int32_t>(value);
}

int32_t c_tensor_get_int32_data(void* data) {
  return c_tensor_get_singular_numeric_data<int32_t>(data);
}

// =================================================================================================

int64_t* c_tensor_create_int64_data(int64_t value) {
  return c_tensor_create_singular_numeric_data<int64_t>(value);
}

int64_t c_tensor_get_int64_data(void* data) {
  return c_tensor_get_singular_numeric_data<int64_t>(data);
}

// =================================================================================================

float* c_tensor_create_float_data(float value) {
  return c_tensor_create_singular_numeric_data<float>(value);
}

float c_tensor_get_float_data(void* data) {
  return c_tensor_get_singular_numeric_data<float>(data);
}

// =================================================================================================

double* c_tensor_create_double_data(double value) {
  return c_tensor_create_singular_numeric_data<double>(value);
}

double c_tensor_get_double_data(void* data) {
  return c_tensor_get_singular_numeric_data<double>(data);
}

// =================================================================================================

char** c_tensor_create_string_data(const char* str) {
  char** data = static_cast<char**>(malloc(sizeof(char*)));
  if (data) {
    data[0] = strdup(str);
  }
  return data;
}

char* c_tensor_get_string_data(void* data) { return static_cast<char**>(data)[0]; }

// =================================================================================================

bool c_tensor_delete_data(CTensor* tensor) {
  if (!tensor->data) {
    return true;
  }

  switch (tensor->dataType) {
    case DATATYPE::BOOLEAN:
    case DATATYPE::INT32:
    case DATATYPE::INT64:
    case DATATYPE::FLOAT:
    case DATATYPE::DOUBLE: {
      c_tensor_delete_singular_numeric_data(tensor->data);
      return true;
    }

    case DATATYPE::STRING: {
      c_tensor_delete_string_data(tensor);
      return true;
    }

    default:
      break;
  }

  return false;
}
