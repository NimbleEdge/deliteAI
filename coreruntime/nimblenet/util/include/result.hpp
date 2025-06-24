/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <variant>

namespace ne {

template <typename Out, typename Err>
class Result {
  bool _isError;
  std::variant<Out, Err> _data;

 public:
  Result(Out out) : _data(out), _isError(false) {}

  Result(Err err) : _data(err), _isError(true) {}

  bool is_error() const noexcept { return _isError; }

  Out get_data() { return std::get<Out>(_data); }

  Err get_error() { return std::get<Err>(_data); }

  NimbleNetStatus* populate_data_or_return_error(Out* data) {
    if (is_error()) {
      return get_error();
    }
    *data = get_data();
    return nullptr;
  }
};

}  // namespace ne