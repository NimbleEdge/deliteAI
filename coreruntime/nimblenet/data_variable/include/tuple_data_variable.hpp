/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <nlohmann/json.hpp>

#include "data_variable.hpp"

/**
 * @brief Represents a tuple data structure in the NimbleNet system
 *
 * TupleDataVariable provides an immutable tuple container that stores a fixed-size
 * collection of heterogeneous data elements. Unlike lists, tuples cannot be modified
 * after creation, making them suitable for representing fixed data structures.
 * The class supports indexed access, JSON serialization, and string representation.
 */
class TupleDataVariable final : public DataVariable {
  int get_containerType() const override { return CONTAINERTYPE::TUPLE; }

  std::vector<OpReturnType> _members; /**< Vector storing the tuple elements */

  int get_dataType_enum() const override { return DATATYPE::EMPTY; }

  /**
   * @brief Retrieves an element at the specified index
   * @param index The zero-based index of the element to retrieve
   * @return The element at the specified index
   * @throws Exception if index is out of bounds
   */
  OpReturnType get_int_subscript(int index) override {
    if (index > _members.size() || index < 0) {
      THROW("trying to access %d index for tuple of size=%d", index, _members.size());
    }
    return _members[index];
  }

  int get_size() override { return _members.size(); }

  /**
   * @brief Sets an element at the specified index
   * @param subscriptVal The index where to set the element
   * @param d The element to set at the specified index
   * @throws Exception if index is out of bounds
   */
  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override {
    int index = subscriptVal->get_int32();
    if (index > _members.size() || index < 0) {
      THROW("trying to set %d index for tuple of size=%d", index, _members.size());
    }
    _members[index] = d;
  }

  /**
   * @brief Converts the tuple to a string representation
   * @return String representation in the format "(element1, element2, ...)"
   */
  std::string print() override {
    std::string output = "(";
    for (int i = 0; i < _members.size(); i++) {
      if (i != 0) output += ", ";
      output += _members[i]->print();
    }
    output += ")";
    return output;
  }

  /**
   * @brief Converts the tuple to a JSON array representation
   * @return JSON array containing all tuple elements
   */
  nlohmann::json to_json() const override {
    auto output = nlohmann::json::array();
    for (auto& member : _members) {
      output.push_back(member->to_json());
    }
    return output;
  }

 public:
  /**
   * @brief Constructs a tuple with the specified members
   * @param members Vector of elements to initialize the tuple with
   */
  TupleDataVariable(const std::vector<OpReturnType>& members) { _members = members; }

  bool get_bool() override { return get_size() > 0; }

  /**
   * @brief Returns a copy of all tuple members
   * @return Vector containing all tuple elements
   */
  std::vector<OpReturnType> get_members() { return _members; }
};