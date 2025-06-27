/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"

/**
 * @brief Specialized data variable for frontend objects
 *
 * FrontendDataVariable represents objects that originate from or are destined
 * for frontend applications. This class provides a concrete implementation
 * of DataVariable specifically for handling frontend object types (FE_OBJ).
 *
 * The class overrides the data type identification to return DATATYPE::FE_OBJ
 * and provides a static factory method for creating frontend data variables.
 */
class FrontendDataVariable : public DataVariable {
 protected:
  /**
   * @brief Returns the data type enum for frontend objects
   * @return DATATYPE::FE_OBJ constant
   */
  int get_dataType_enum() const final { return DATATYPE::FE_OBJ; }

 public:
  /**
   * @brief Creates a new FrontendDataVariable instance
   * @param data Pointer to the frontend object data
   * @return OpReturnType containing the created FrontendDataVariable
   */
  static OpReturnType create(void* data);
};