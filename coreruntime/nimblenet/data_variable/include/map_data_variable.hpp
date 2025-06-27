/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <shared_mutex>
#include <string>

#include "data_variable.hpp"
#include "tuple_data_variable.hpp"

#pragma GCC visibility push(default)

class MapDataVariable;
typedef std::shared_ptr<MapDataVariable> MapVariablePtr;

/**
 * @brief Thread-safe map data variable implementation for key-value storage
 *
 * MapDataVariable provides a thread-safe implementation of a map/dictionary
 * data structure that stores string keys mapped to OpReturnType values.
 * It uses a read-write lock (shared_mutex) to allow concurrent reads while
 * ensuring exclusive access for writes.
 *
 * The class supports standard map operations like insertion, lookup, removal,
 * and iteration. It also provides JSON serialization capabilities and
 * integration with the NimbleNet tensor system through CTensors conversion.
 */
class MapDataVariable final : public DataVariable {
  std::map<std::string, OpReturnType> _map; /**< Internal map storing key-value pairs */
  mutable std::shared_mutex _mutex;         /**< Read-write lock for thread safety */

  int get_containerType() const override { return CONTAINERTYPE::MAP; }

  int get_dataType_enum() const override { return DATATYPE::EMPTY; }

  bool get_bool() override;

  int get_size() override;

  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override;

 public:
  void set_value_in_map(const std::string& key, const OpReturnType& d) override;

  bool in(const OpReturnType& elem) override;

  JsonIterator* get_json_iterator() override;

 public:
  std::string print() override { return to_json_str(); }

  nlohmann::json to_json() const override;

  std::string to_json_str() const override;

  const std::map<std::string, OpReturnType>& get_map() override;

  /**
   * @brief Calls member functions on the map variable
   * @param memberFuncIndex Index of the member function to call
   * @param arguments Vector of arguments to pass to the function
   * @param stack Call stack for execution context
   * @return Result of the function call
   *
   * Supports operations like POP (remove and return value) and KEYS (return list of keys)
   */
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

 public:
  OpReturnType get_string_subscript(const std::string& key) override;

  /**
   * @brief Constructs a map from separate key and value vectors
   * @param keys Vector of key variables (will be converted to strings)
   * @param values Vector of value variables
   */
  MapDataVariable(const std::vector<OpReturnType>& keys, const std::vector<OpReturnType>& values);

  /**
   * @brief Constructs a map from CTensors structure
   * @param inputs CTensors structure containing named tensors and variables
   */
  MapDataVariable(const CTensors& inputs);

  /**
   * @brief Move constructor from an existing map
   * @param m R-value reference to a map to move from
   */
  MapDataVariable(std::map<std::string, OpReturnType>&& m) { _map = std::move(m); }

  /**
   * @brief Merges another map variable into this one
   * @param mapVariable Map variable to merge
   * @return True if merge was successful
   */
  bool add_or_update(OpReturnType mapVariable);

  MapDataVariable() {}

  /**
   * @brief Converts the map contents to CTensors structure
   * @param cTensors Pointer to CTensors structure to populate
   *
   * This method is used for model output conversion, where map keys become
   * tensor names and values become the actual tensors. Special keys like
   * exit status are filtered out during conversion.
   */
  void convert_to_cTensors(CTensors* cTensors);
};

#pragma GCC visibility pop
