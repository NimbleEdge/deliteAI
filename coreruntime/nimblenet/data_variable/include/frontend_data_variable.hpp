/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"

class FrontendDataVariable : public DataVariable {
 protected:
  int get_dataType_enum() const final { return DATATYPE::FE_OBJ; }

 public:
  static OpReturnType create(void* data);
};