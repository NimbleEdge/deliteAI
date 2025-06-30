#pragma once

#include <map>

#include "nimble_net_util.hpp"

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
