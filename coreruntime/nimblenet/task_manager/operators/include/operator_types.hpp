/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>

#include "nimble_net_util.hpp"

/**
 * @brief Determines the higher precedence data type for type promotion
 *
 * Compares two data types and returns the one with higher precedence
 * for automatic type promotion in operations. The precedence order is:
 * BOOLEAN (0) < INT32 (3) < INT64 (4) < FLOAT (5) < DOUBLE (6)
 *
 * @param dataType1 First data type to compare
 * @param dataType2 Second data type to compare
 * @return The data type with higher precedence
 */
inline int get_max_dataType(int dataType1, int dataType2) {
  std::map<int, int> _typeScore = {
      {DATATYPE::BOOLEAN, 0}, {DATATYPE::INT32, 3},  {DATATYPE::INT64, 4},
      {DATATYPE::FLOAT, 5},   {DATATYPE::DOUBLE, 6},
  };
  if (_typeScore[dataType1] < _typeScore[dataType2]) {
    return dataType2;
  } else
    return dataType1;
}
