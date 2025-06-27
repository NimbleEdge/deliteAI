#pragma once

#include <map>
#include <shared_mutex>
#include <string>

#include "data_variable.hpp"
#include "tuple_data_variable.hpp"

#pragma GCC visibility push(default)

class MapDataVariable;
typedef std::shared_ptr<MapDataVariable> MapVariablePtr;

class MapDataVariable final : public DataVariable {
  std::map<std::string, OpReturnType> _map;
  mutable std::shared_mutex _mutex;  // Read-Write lock

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

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

 public:
  OpReturnType get_string_subscript(const std::string& key) override;

  MapDataVariable(const std::vector<OpReturnType>& keys, const std::vector<OpReturnType>& values);

  MapDataVariable(const CTensors& inputs);

  MapDataVariable(std::map<std::string, OpReturnType>&& m) { _map = std::move(m); }

  bool add_or_update(OpReturnType mapVariable);

  MapDataVariable() {}

  void convert_to_cTensors(CTensors* cTensors);
};

#pragma GCC visibility pop
