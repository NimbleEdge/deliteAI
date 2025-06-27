/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "data_variable.hpp"
#include "iterable_data_variable.hpp"
#include "single_variable.hpp"

class RangeDataVariable final : public IterableDataVariable {
  int _range;

 public:
  RangeDataVariable(int range) { _range = range; }

  int get_dataType_enum() const override { return DATATYPE::INT64; }

  int get_containerType() const override { return CONTAINERTYPE::RANGE; }

  int get_size() override { return _range; }

  OpReturnType get_int_subscript(int index) override {
    if (index > _range || index < 0) {
      THROW("accessing %d of Range with size=%d", index, _range);
    }
    return OpReturnType(new SingleVariable<int64_t>(index));
  }

  bool get_bool() override { return _range; }

  std::string print() override { return fallback_print(); }

  nlohmann::json to_json() const override { return "[Range]"; }
};
