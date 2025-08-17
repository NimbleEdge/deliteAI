/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimble_net/c_tensor.h"

#include "nimble_net_util.hpp"

void c_tensor_init(CTensor* tensor) {
  tensor->name = nullptr;
  tensor->data = nullptr;
  tensor->dataType = DATATYPE::UNKNOWN;
  tensor->shape = nullptr;
  tensor->shapeLength = 0;
}
