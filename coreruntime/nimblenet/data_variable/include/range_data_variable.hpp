/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "data_variable.hpp"
#include "iterable_data_variable.hpp"
#include "single_variable.hpp"

/**
 * @brief Represents a range of integers from 0 to range-1
 *
 * RangeDataVariable provides a lightweight container that represents a sequence
 * of integers starting from 0 up to (but not including) the specified range value.
 * This is similar to Python's range() function behavior. The class supports
 * iteration, indexing, and conversion to boolean (true if range > 0).
 *
 * Example usage:
 * - RangeDataVariable(5) represents [0, 1, 2, 3, 4]
 * - RangeDataVariable(0) represents an empty range
 * - RangeDataVariable(1) represents [0]
 */
class RangeDataVariable final : public IterableDataVariable {
  int _range; /**< The upper bound of the range (exclusive) */

 public:
  /**
   * @brief Constructs a range from 0 to range-1
   * @param range The upper bound of the range (exclusive)
   */
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
