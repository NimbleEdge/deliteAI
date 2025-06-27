/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <nlohmann/json.hpp>

#include "data_variable.hpp"

class TupleDataVariable final : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::TUPLE; }

  std::vector<OpReturnType> _members;

  int get_dataType_enum() const override { return DATATYPE::EMPTY; }

  OpReturnType get_int_subscript(int index) override {
    if (index > _members.size() || index < 0) {
      THROW("trying to access %d index for tuple of size=%d", index, _members.size());
    }
    return _members[index];
  }

  int get_size() override { return _members.size(); }

  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override {
    int index = subscriptVal->get_int32();
    if (index > _members.size() || index < 0) {
      THROW("trying to set %d index for tuple of size=%d", index, _members.size());
    }
    _members[index] = d;
  }

  std::string print() override {
    std::string output = "(";
    for (int i = 0; i < _members.size(); i++) {
      if (i != 0) output += ", ";
      output += _members[i]->print();
    }
    output += ")";
    return output;
  }

  nlohmann::json to_json() const override {
    auto output = nlohmann::json::array();
    for (auto& member : _members) {
      output.push_back(member->to_json());
    }
    return output;
  }

 public:
  TupleDataVariable(const std::vector<OpReturnType>& members) { _members = members; }

  bool get_bool() override { return get_size() > 0; }

  std::vector<OpReturnType> get_members() { return _members; }
};