#pragma once

#include <memory>
#include <string>

#include "data_variable.hpp"

class BaseSingleVariable : public DataVariable {
  int get_containerType() const final { return CONTAINERTYPE::SINGLE; }

  int get_size() override { return 1; }
};

// for JSON single variable, should use JSONSingleVariable
template <class T,
          typename = std::enable_if_t<ne::is_not_same_v<ne::remove_cvref_t<T>, nlohmann::json>>>
class SingleVariable final : public BaseSingleVariable {
  T val;

 public:
  SingleVariable(T v) { val = v; }

  SingleVariable(const void* vPtr) { val = *((T*)vPtr); }

  bool is_numeric() override { return ::is_numeric<T>(); }

  bool is_string() override { return false; }

  bool is_integer() override { return ::is_integer<T>(); }

  OpReturnType unary_sub() override { return OpReturnType(new SingleVariable<T>(-val)); }

  int get_dataType_enum() const override { return ::get_dataType_enum<T>(); }

  int32_t get_int32() override { return int32_t(val); }

  float get_float() override { return float(val); }

  double get_double() override { return double(val); }

  int64_t get_int64() override { return int64_t(val); }

  uint8_t get_uint8() override { return uint8_t(val); }

  int8_t get_int8() override { return int8_t(val); }

  bool get_bool() override { return val; }

  std::string print() override {
    std::stringstream ss;
    ss << val;
    return ss.str();
  }

  nlohmann::json to_json() const override { return val; }

  void* get_raw_ptr() override { return (void*)&val; }
};

template <>
class SingleVariable<std::string> final : public BaseSingleVariable {
  std::string val;
  std::vector<char*> strPtr;
  int cached_char_count;  // Character count
  std::vector<int> char_to_byte_map;

  // Build the character-to-byte mapping
  void build_char_to_byte_map();

 public:
  SingleVariable(const std::string& v);

  SingleVariable(const char* vPtr);

  int get_dataType_enum() const override { return DATATYPE::STRING; }

  bool is_string() override { return true; }

  std::string get_string() const override { return val; }

  bool get_bool() override { return val != ""; }

  std::string print() override { return get_string(); }

  nlohmann::json to_json() const override { return get_string(); }

  float cast_float() override { return std::stof(val); }

  int32_t cast_int32() override { return std::stoi(val); }

  int64_t cast_int64() override { return std::stol(val); }

  double cast_double() override { return std::stod(val); }

  // Can't use get_string_ptr() as it returns char**
  void* get_raw_ptr() override { return static_cast<void*>((char*)val.c_str()); }

  char** get_string_ptr() override;

  const char* get_c_str() const { return val.c_str(); }

  // Return charcter length as size
  int get_size() override { return cached_char_count; }

  // Get the number of Unicode characters
  int get_char_count() const { return cached_char_count; }

  // O(1) lookup from character index to byte position
  int char_idx_to_byte_pos(int char_idx) const;

  OpReturnType get_int_subscript(int argument) override;

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  // Override get_subscript to handle both integer indices and slice objects
  OpReturnType get_subscript(const OpReturnType& subscriptVal) override;

  // Method to handle slice objects with Unicode awareness
  OpReturnType get_slice_subscript(const OpReturnType& sliceObj);

  bool in(const OpReturnType& str) override {
    return val.find(str->get_string()) != std::string::npos;
  }
};

template <>
class SingleVariable<std::wstring> final : public BaseSingleVariable {
  std::wstring _val;
  std::wstring_convert<std::codecvt_utf8<wchar_t>> _utf8_converter;

 public:
  SingleVariable(const std::wstring& v) { _val = v; }

  int get_dataType_enum() const override { return DATATYPE::UNICODE_STRING; }

  bool is_string() override { return false; }

  bool get_bool() override { return !_val.empty(); }

  std::string print() override { return _utf8_converter.to_bytes(_val); }

  nlohmann::json to_json() const override;

  OpReturnType get_int_subscript(int argument) override;

  int get_size() override { return _val.size(); }
};

template <typename JSONType,
          typename = std::enable_if_t<std::is_same_v<ne::remove_cvref_t<JSONType>, nlohmann::json>>>
class JSONSingleVariable final : public BaseSingleVariable {
  JSONType _val;

 public:
  JSONSingleVariable(const std::remove_reference_t<JSONType>& v) : _val(v) {}

  JSONSingleVariable(std::remove_reference_t<JSONType>&& v) : _val(std::move(v)) {}

  int get_dataType_enum() const override { return DATATYPE::JSON; }

  bool get_bool() override { return true; }

  json get_json_data() override { return _val; }

  const json& get_json_ref() const { return _val; }

  void* get_raw_ptr() override {
    return static_cast<void*>(const_cast<ne::remove_cvref_t<JSONType>*>(&_val));
    // THROW("%s","get_raw_ptr not supported for JSONSingleVariable, use get_json_ref instead");
  }

  const void* get_raw_ptr() const { return static_cast<const void*>(&_val); }

  std::string print() override { return _val.dump(); }

  nlohmann::json to_json() const override { return _val; }

  int get_size() override { return 1; }

  OpReturnType get_string_subscript(const std::string& key) override {
    if (_val.find(key) != _val.end()) {
      return DataVariable::get_SingleVariableFrom_JSON(_val[key]);
    }
    THROW("Could not find key=%s in json", key.c_str());
  }

  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) override {
    if constexpr (std::is_const_v<std::remove_reference_t<JSONType>>) {
      THROW_UNSUPPORTED("set_subscript");
    } else {
      std::string key = subscriptVal->get_string();

      typename ne::remove_cvref_t<JSONType>::value_t value;

      if (d->get_containerType() == CONTAINERTYPE::SINGLE) {
        if (d->is_string()) {
          _val[key] = d->get_string();
        } else if (d->is_integer()) {
          _val[key] = d->get_int64();
        } else {
          _val[key] = d->get_float();
        }
      } else if (d->get_containerType() == CONTAINERTYPE::VECTOR) {
        auto tensorPtr = d->get_raw_ptr();
        json jsonArray = json::array();
        if (d->is_string()) {
          for (int i = 0; i < d->get_numElements(); i++) {
            jsonArray.push_back(d->get_int_subscript(i)->get_string());
          }
          _val[key] = jsonArray;
        } else if (d->is_integer()) {
          for (int i = 0; i < d->get_numElements(); i++) {
            jsonArray.push_back(d->get_int_subscript(i)->get_int32());
          }
          _val[key] = jsonArray;
        } else {
          for (int i = 0; i < d->get_numElements(); i++) {
            jsonArray.push_back(d->get_int_subscript(i)->get_float());
          }
          _val[key] = jsonArray;
        }
      } else {
        THROW("Only single variables/tensors can be set as a value to json object, provided=%s",
              d->get_containerType_string());
      }
    }
  }

  template <typename T>
  static inline bool find_in_json_array(const nlohmann::json& j, T val) {
    for (const auto& x : j) {
      if (x.get<T>() == val) return true;
    }
    return false;
  }

  bool in(const OpReturnType& elem) override {
    if (elem->get_containerType() != CONTAINERTYPE::SINGLE) {
      THROW("%s",
            "in and not in comparators expects single argument when checking for a key in json "
            "object.");
    }

    if (_val.is_array()) {
      switch (elem->get_dataType_enum()) {
        case DATATYPE::INT32:
        case DATATYPE::INT64:
          return find_in_json_array(_val, elem->get_int64());
        case DATATYPE::FLOAT:
          return find_in_json_array(_val, elem->get_float());
        case DATATYPE::DOUBLE:
          return find_in_json_array(_val, elem->get_double());
        case DATATYPE::STRING:
          return find_in_json_array(_val, elem->get_string());
        case DATATYPE::BOOLEAN:
          return find_in_json_array(_val, elem->get_bool());
        default:
          THROW("%s", "Unsupported type");
      }
    }

    if (_val.find(elem->get_string()) != _val.end()) {
      return true;
    }
    return false;
  }
};
