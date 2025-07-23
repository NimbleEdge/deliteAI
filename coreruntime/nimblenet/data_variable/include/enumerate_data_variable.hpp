/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "iterable_data_variable.hpp"

/**
 * @brief DataVariable that yields index-value pairs for iterables
 *
 * EnumerateDataVariable wraps an existing iterable and produces a sequence of
 * (index, value) pairs. Each call to `next()` returns a tuple containing the current
 * index and the corresponding element from the underlying iterable.
 */
class EnumerateDataVariable final : public IterableDataVariable {
 private:
  OpReturnType _iterable; /**< The underlying iterable to enumerate over */
  int _startIndex = 0; /**< Starting index for enumeration */
  int _size = -1; /**< Cached size of the underlying iterable */

 public:
  EnumerateDataVariable(OpReturnType iterable, int startIndex = 0);

  int get_dataType_enum() const override { return _iterable->get_dataType_enum(); }

  int get_containerType() const override { return _iterable->get_containerType(); }

  std::string print() override { return fallback_print(); }

  nlohmann::json to_json() const override { return "[Enumerate]"; }

  int get_size() override { return _size; }

  OpReturnType get_int_subscript(int index) override;

  void reset_iterator() override;

  OpReturnType next(CallStack& stack) override;

  bool get_bool() override;
};
