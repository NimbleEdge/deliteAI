#pragma once

#include "data_variable.hpp"

class FrontendDataVariable : public DataVariable {
 protected:
  int get_dataType_enum() const final { return DATATYPE::FE_OBJ; }

 public:
  static OpReturnType create(void* data);
};