template <>
constexpr inline bool is_numeric<float>() {
  return true;
}

template <>
constexpr inline bool is_numeric<double>() {
  return true;
}

template <>
constexpr inline bool is_numeric<int64_t>() {
  return true;
}

template <>
constexpr inline bool is_numeric<int32_t>() {
  return true;
}

template <>
constexpr inline bool is_numeric<bool>() {
  return false;
}

template <>
constexpr inline bool is_numeric<std::string>() {
  return false;
}

template <>
constexpr inline bool is_numeric<nlohmann::json>() {
  return false;
}

template <class T>
constexpr inline bool is_string() {
  return std::is_same_v<ne::remove_cvref_t<T>, std::string>;
}

template <>
constexpr inline bool is_integer<float>() {
  return false;
}

template <>
constexpr inline bool is_integer<double>() {
  return false;
}

template <>
constexpr inline bool is_integer<int64_t>() {
  return true;
}

template <>
constexpr inline bool is_integer<int32_t>() {
  return true;
}

template <>
constexpr inline bool is_integer<bool>() {
  return false;
}

template <>
constexpr inline bool is_integer<std::string>() {
  return false;
}

template <>
constexpr inline bool is_integer<nlohmann::json>() {
  return false;
}

template <>
constexpr inline int get_dataType_enum<float>() {
  return DATATYPE::FLOAT;
}

template <>
constexpr inline int get_dataType_enum<int32_t>() {
  return DATATYPE::INT32;
}

template <>
constexpr inline int get_dataType_enum<int64_t>() {
  return DATATYPE::INT64;
}

template <>
constexpr inline int get_dataType_enum<double>() {
  return DATATYPE::DOUBLE;
}

template <>
constexpr inline int get_dataType_enum<bool>() {
  return DATATYPE::BOOLEAN;
}

template <>
constexpr inline int get_dataType_enum<std::string>() {
  return DATATYPE::STRING;
}

template <>
constexpr inline int get_dataType_enum<nlohmann::json>() {
  return DATATYPE::JSON;
}

template <>
inline int32_t DataVariable::get<int32_t>() {
  return get_int32();
}

template <>
inline float DataVariable::get<float>() {
  return get_float();
}

template <>
inline int64_t DataVariable::get<int64_t>() {
  return get_int64();
}

template <>
inline double DataVariable::get<double>() {
  return get_double();
}

template <>
inline std::string DataVariable::get<std::string>() {
  return get_string();
}

template <>
inline bool DataVariable::get<bool>() {
  return get_bool();
}

// TODO: Need to remove get_json_data(), will also have to remove
// BaseTypedTensorVariable<nlohmann::json>
template <>
inline nlohmann::json DataVariable::get<nlohmann::json>() {
  return get_json_data();
}