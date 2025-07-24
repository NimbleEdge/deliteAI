/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <codecvt>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "data_variable_enums.hpp"
#include "executor_structs.h"
#include "json.hpp"
#include "ne_fwd.hpp"
#include "ne_type_traits.hpp"
#include "nimble_net_util.hpp"
#include "nlohmann/json_fwd.hpp"
#include "util.hpp"

class SingleType;
class VectorType;
class CallStack;

/**
 * @brief Iterator for traversing JSON-like data structures (maps and arrays)
 *
 * This struct provides a unified interface for iterating over both map and array
 * data structures. It maintains separate iterators for each type and uses the
 * isArray flag to determine which iterator set is active.
 */
struct JsonIterator {
  std::map<std::string, OpReturnType>::iterator map_begin; /**< Iterator to beginning of map */
  std::map<std::string, OpReturnType>::iterator map_end;   /**< Iterator to end of map */
  std::vector<OpReturnType>::iterator list_begin;          /**< Iterator to beginning of list */
  std::vector<OpReturnType>::iterator list_end;            /**< Iterator to end of list */
  bool isArray = false;                                     /**< Flag indicating if this is an array iterator */

  /**
   * @brief Constructor for map iteration
   * @param beginIt Iterator to the beginning of the map
   * @param endIt Iterator to the end of the map
   */
  JsonIterator(std::map<std::string, OpReturnType>::iterator beginIt,
               std::map<std::string, OpReturnType>::iterator endIt)
      : map_begin(beginIt), map_end(endIt), isArray(false) {}

  /**
   * @brief Constructor for array iteration
   * @param beginIt Iterator to the beginning of the array
   * @param endIt Iterator to the end of the array
   */
  JsonIterator(std::vector<OpReturnType>::iterator beginIt,
               std::vector<OpReturnType>::iterator endIt)
      : list_begin(beginIt), list_end(endIt), isArray(true) {}
};

template <class T>
constexpr inline int get_dataType_enum();

template <class T>
constexpr inline bool is_numeric();

template <class T>
constexpr inline bool is_string();

template <class T>
constexpr inline bool is_integer();

#define THROW_UNSUPPORTED(funcName)                                                   \
  THROW("%s not supported for variable %s(%s)", funcName, get_containerType_string(), \
        util::get_string_from_enum(get_dataType_enum()));

#define THROW_ARGUMENTS_NOT_MATCH(argsSize, expectedSize, funcIndex)                               \
  do {                                                                                             \
    if (argsSize != expectedSize) {                                                                \
      THROW("%s expects %d argument %d given for variable %s(%s)",                                 \
            get_member_func_string(funcIndex), expectedSize, argsSize, get_containerType_string(), \
            util::get_string_from_enum(get_dataType_enum()));                                      \
    }                                                                                              \
  } while (0)

#define THROW_ARGUMENTS_MISMATCH_FUNCTION_NAME(argsSize, expectedSize, functionName)            \
  do {                                                                                          \
    if (argsSize != expectedSize) {                                                             \
      THROW("%s expects %d argument %d given for variable %s(%s)", functionName, expectedSize); \
    }                                                                                           \
  } while (0)

#define THROW_OPTIONAL_ARGUMENTS_NOT_MATCH(argsSize, expectedSize1, expectedSize2, funcIndex) \
  do {                                                                                        \
    if (argsSize != expectedSize1 && argsSize != expectedSize2) {                             \
      THROW("%s expects %d or %d argument(s), %d given for variable %s(%s)",                  \
            get_member_func_string(funcIndex), expectedSize1, expectedSize2, argsSize,        \
            get_containerType_string(), util::get_string_from_enum(get_dataType_enum()));     \
    }                                                                                         \
  } while (0)

#define THROW_OPTIONAL_ARGUMENTS_NOT_MATCH_FUNCTION_NAME(argsSize, expectedSize1, expectedSize2, functionName) \
  do {                                                                                                         \
    if ((argsSize) != (expectedSize1) && (argsSize) != (expectedSize2)) {                                      \
      THROW("%s expects %d or %d argument(s), %d provided",                                                    \
            functionName, expectedSize1, expectedSize2, argsSize);                                             \
    }                                                                                                          \
  } while (0)

#define THROW_ARGUMENT_DATATYPE_NOT_MATCH(dataType, expectedDataType, argIndex, funcIndex)       \
  do {                                                                                           \
    if (dataType != expectedDataType) {                                                          \
      THROW("%s expects argument at index %d to be of type %s. Given %s type.",                  \
            get_member_func_string(funcIndex), argIndex,                                         \
            util::get_string_from_enum(expectedDataType), util::get_string_from_enum(dataType)); \
    }                                                                                            \
  } while (0)

enum class CreateTensorType { MOVE, COPY };

/**
 * @brief Base class for all data variables in the NimbleNet system
 *
 * DataVariable is the core abstraction for handling different types of data
 * in the NimbleNet runtime. It provides a unified interface for operations
 * on various data types including scalars, tensors, lists, maps, and more.
 * The class uses the enable_shared_from_this pattern to allow safe shared_ptr
 * returns from member functions.
 *
 * The class implements a virtual function interface where derived classes
 * override specific methods based on their data type and container type.
 * Unsupported operations throw exceptions with descriptive error messages.
 */
class DataVariable : public std::enable_shared_from_this<DataVariable> {
  /* std::enable_shared_from_this<DataVariable> so that shared_ptr can be returned in member
   function call using shared_from_this() */

  static std::map<std::string, int> _memberFuncMap;        /**< Mapping from function names to indices */
  static std::map<int, std::string> _inverseMemberFuncMap; /**< Mapping from indices to function names */

 public:
  static int add_and_get_member_func_index(const std::string& memberFuncString);
  static int get_member_func_index(const std::string& memberFuncString);
  static const char* get_member_func_string(int funcIndex);
  static OpReturnType create_tensor(int dType, const std::vector<int64_t>& shape);
  static OpReturnType create_tensor(const CTensor& c, CreateTensorType type);
  static OpReturnType create_single_variable(const CTensor& c);

  virtual const std::map<std::string, OpReturnType>& get_map() { THROW_UNSUPPORTED("get_map"); }

  virtual int get_containerType() const = 0;

  virtual void* get_raw_ptr() { THROW_UNSUPPORTED("get_raw_ptr"); }

  virtual char** get_string_ptr() { THROW_UNSUPPORTED("get_string_ptr"); }

  virtual OpReturnType execute_function(const std::vector<OpReturnType>& arguments,
                                        CallStack& stack) {
    THROW_UNSUPPORTED("execute_function");
  }

  virtual OpReturnType execute_function(const std::vector<OpReturnType>& arguments) {
    THROW_UNSUPPORTED("execute_function");
  }

  virtual OpReturnType get_member(int memberIndex) { THROW_UNSUPPORTED("get_member"); }

  virtual void set_member(int memberIndex, OpReturnType d) { THROW_UNSUPPORTED("set_member"); }

  virtual OpReturnType sort(const OpReturnType argument) { THROW_UNSUPPORTED("sort"); }

  virtual OpReturnType argsort(const OpReturnType argument) { THROW_UNSUPPORTED("argsort"); }

  virtual OpReturnType topk(const std::vector<OpReturnType>& arguments) {
    THROW_UNSUPPORTED("topk");
  }

  virtual OpReturnType arrange(const OpReturnType argument) { THROW_UNSUPPORTED("arrange"); }

  virtual void init() { THROW_UNSUPPORTED("init"); }

  virtual bool is_numeric() { return false; }

  virtual bool is_string() { return false; }

  virtual bool is_integer() { return false; }

  virtual bool is_none() { return false; }
  
  virtual bool is_iterable() const { return false; }

  const char* get_containerType_string() const;

  /**
   * @brief Converts the variable to a CTensor representation
   * @param name The name to assign to the tensor
   * @param rawPtr Raw pointer to the data
   * @return CTensor representation of the variable
   */
  CTensor to_cTensor(char* name, void* rawPtr);

  virtual OpReturnType to_tensor(OpReturnType d) { THROW_UNSUPPORTED("to_tensor"); }

  virtual OpReturnType append(OpReturnType d) { THROW_UNSUPPORTED("append"); }

  /**
   * @brief Calls a member function by index with arguments
   * @param memberFuncIndex The index of the member function to call
   * @param arguments The arguments to pass to the function
   * @param stack The call stack for execution context
   * @return OpReturnType containing the function result
   */
  virtual OpReturnType call_function(int memberFuncIndex,
                                     const std::vector<OpReturnType>& arguments, CallStack& stack);

  virtual OpReturnType unary_sub() { THROW_UNSUPPORTED("unary_sub"); }

  virtual bool in(const OpReturnType& elem) { THROW_UNSUPPORTED("in"); }

  virtual bool notIn(const OpReturnType& elem) { return !in(elem); }

  virtual OpReturnType get_int_subscript(int val) { THROW_UNSUPPORTED("get_int_subscript"); }

  virtual json get_json_data() { THROW_UNSUPPORTED("get_json"); }

  virtual OpReturnType get_string_subscript(const std::string& val) {
    THROW_UNSUPPORTED("get_string_subscript");
  }

  virtual OpReturnType get_subscript(const OpReturnType& subscriptVal) {
    THROW_UNSUPPORTED("get_subscript");
  }

  virtual OpReturnType next(CallStack& stack) { THROW_UNSUPPORTED("next"); }

  virtual void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) {
    THROW_UNSUPPORTED("set_subscript");
  }

  virtual int get_dataType_enum() const = 0;

  template <typename T>
  inline T get();

  bool is_single() { return get_containerType() == CONTAINERTYPE::SINGLE; }

  virtual std::string print() = 0;

  std::string fallback_print() {
    return std::string("<") + get_containerType_string() + "(" +
           util::get_string_from_enum(get_dataType_enum()) + ")" + ">";
  }

  virtual nlohmann::json to_json() const = 0;

  // Need as to_json() of proto objects will be slow, so ProtoDataVariable will override this method
  // to return a stringified json directly in kotlin, instead of going through JNI layer
  virtual std::string to_json_str() const { return to_json().dump(); }

  // for string "2.0" get_float won't convert but cast_float will work
  virtual float cast_float() { return get_float(); }

  virtual int32_t cast_int32() { return get_int32(); }

  virtual int64_t cast_int64() { return get_int64(); }

  virtual double cast_double() { return get_double(); }

  virtual uint8_t cast_uint8() { return get_uint8(); }

  virtual int8_t cast_int8() { return get_int8(); }

  virtual int32_t get_int32() { THROW_UNSUPPORTED("get_int32"); }

  virtual float get_float() { THROW_UNSUPPORTED("get_float"); }

  virtual int64_t get_int64() { THROW_UNSUPPORTED("get_int64"); }

  virtual double get_double() { THROW_UNSUPPORTED("get_double"); }

  virtual int8_t get_int8() { THROW_UNSUPPORTED("get_int8"); }

  virtual uint8_t get_uint8() { THROW_UNSUPPORTED("get_uint8"); }

  virtual std::string get_string() const { THROW_UNSUPPORTED("get_string"); }

  virtual bool get_bool() = 0;

  virtual int get_size() { THROW_UNSUPPORTED("get_size"); }

  virtual bool reshape(const std::vector<int64_t>& val) { THROW_UNSUPPORTED("reshape"); }

  virtual const std::vector<int64_t>& get_shape() { THROW_UNSUPPORTED("get_shape"); }

  // Implement this function only in DataVariables which have shape stored in them.
  virtual int get_numElements() { THROW_UNSUPPORTED("get_numElements"); }

  virtual void set_value_in_map(const std::string& key, const OpReturnType& d) {
    THROW_UNSUPPORTED("set_value_in_map");
  }

  /**
   * @brief Creates a single variable from a JSON value
   * @param value The JSON value to convert
   * @return OpReturnType containing the created variable
   */
  static OpReturnType get_SingleVariableFrom_JSON(const nlohmann::json& value);

  /**
   * @brief Creates a list variable from a JSON array
   * @param value The JSON array to convert
   * @return OpReturnType containing the created list
   */
  static OpReturnType get_list_from_json_array(nlohmann::json&& value);

  /**
   * @brief Creates a map variable from a JSON object
   * @param value The JSON object to convert
   * @return OpReturnType containing the created map
   */
  static OpReturnType get_map_from_json_object(nlohmann::json&& value);

  /**
   * @brief Returns an iterator for JSON-like traversal
   * @return JsonIterator pointer for iterating over the data
   */
  virtual JsonIterator* get_json_iterator() { THROW_UNSUPPORTED("get_json_iterator"); }

  virtual ~DataVariable() = default;
};

#include "data_variable_templates.ipp"

/**
 * @brief Represents a null/none value in the data variable system
 *
 * This class provides a concrete implementation of DataVariable for representing
 * null or undefined values. It overrides the minimum required virtual functions
 * to provide appropriate behavior for a none-type variable.
 */
class NoneVariable final : public DataVariable {
  bool is_none() override { return true; }

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  int get_dataType_enum() const override { return DATATYPE::NONE; }

  bool get_bool() override { return false; }

  std::string print() override { return fallback_print(); }

  nlohmann::json to_json() const override { return nlohmann::json{}; }
};

/**
 * @brief Represents a slice specification for list operations
 *
 * ListSliceVariable encapsulates the start, stop, and step parameters used
 * for slicing operations on list-like data structures. It provides methods
 * to resolve these parameters relative to a given list size, handling
 * negative indices and default values appropriately.
 */
class ListSliceVariable final : public DataVariable {
  OpReturnType _start; /**< Start index of the slice */
  OpReturnType _stop;  /**< Stop index of the slice */
  OpReturnType _step;  /**< Step size for the slice */

 public:
  /**
   * @brief Constructor with explicit start, stop, and step values
   * @param start The start index
   * @param stop The stop index
   * @param step The step size
   */
  ListSliceVariable(OpReturnType start, OpReturnType stop, OpReturnType step)
      : _start(start), _stop(stop), _step(step) {}

  /**
   * @brief Default constructor creating a full slice ([:])
   *
   * Creates a slice that represents the entire range with default
   * start (beginning), stop (end), and step (1) values.
   */
  ListSliceVariable()
      : _start(OpReturnType(new NoneVariable())),
        _stop(OpReturnType(new NoneVariable())),
        _step(OpReturnType(new NoneVariable())) {}

  int get_containerType() const override { return CONTAINERTYPE::SLICE; }

  int get_dataType_enum() const override { return DATATYPE::EMPTY; }

  bool get_bool() override { return true; }

  /**
   * @brief Resolves the start index relative to the list size
   * @param size The size of the list being sliced
   * @return The resolved start index
   */
  int get_start(int size) const;

  /**
   * @brief Resolves the stop index relative to the list size
   * @param size The size of the list being sliced
   * @return The resolved stop index
   */
  int get_stop(int size) const;

  /**
   * @brief Gets the step size for the slice
   * @return The step size (defaults to 1 if not specified)
   */
  int get_step() const;

  std::string print() override;

  nlohmann::json to_json() const override;
};
