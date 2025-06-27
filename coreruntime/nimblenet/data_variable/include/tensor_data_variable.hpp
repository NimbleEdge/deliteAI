/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include "data_variable.hpp"
#include "data_variable_enums.hpp"
#include "ne_fwd.hpp"
#include "util.hpp"

/**
 * @brief Base class for tensor data variables with shape and element count management
 *
 * This class provides the fundamental tensor functionality including shape management,
 * element counting, and basic tensor operations. It serves as the base for all
 * typed tensor implementations.
 */
class BaseTensorVariable : public DataVariable {
 protected:
  std::vector<int64_t> shape; /**< Tensor dimensions/shape */
  int numElements = 0;        /**< Total number of elements in the tensor */

  void* get_raw_ptr() override = 0;

  bool get_bool() final { return numElements; }

  int get_size() final { return shape.size() ? shape[0] : 1; }

  bool reshape(const std::vector<int64_t>& shape_) final;

 public:
  int get_numElements() override { return numElements; }

  const std::vector<int64_t>& get_shape() override { return shape; }

  virtual OpReturnType filter_key(const std::string& key, int dataTypEnum) {
    THROW_UNSUPPORTED("filter_key");
  }
};

/**
 * @brief Typed tensor variable with data type-specific operations
 *
 * This class extends BaseTensorVariable with type-specific functionality including
 * element size management, subscript operations, and tensor manipulation methods.
 * It provides a unified interface for different numeric data types.
 */
class BaseTypedTensorVariable : public BaseTensorVariable {
 protected:
  const DATATYPE _dataType; /**< The data type of tensor elements */
  const int _elemSize;      /**< Size of each element in bytes */

 protected:
  int get_dataType_enum() const final { return _dataType; }

  /**
   * @brief Helper function to get size in bytes corresponding to data type
   * @param dataType The data type enum value
   * @return Size in bytes for the given data type
   */
  static int get_elem_size(DATATYPE dataType);

  BaseTypedTensorVariable(DATATYPE dataType)
      : _dataType(dataType), _elemSize(get_elem_size(dataType)) {}

  bool in(const OpReturnType& elem) override;

  std::string print() override;

  nlohmann::json to_json() const override;

  OpReturnType get_int_subscript(int index) final;
  OpReturnType get_string_subscript(const std::string& key) override;
  OpReturnType sort(const OpReturnType argument) override;
  OpReturnType argsort(const OpReturnType argument) override;

  OpReturnType topk(const std::vector<OpReturnType>& arguments) override;
  OpReturnType arrange(const OpReturnType argument) override;

 private:
  void set_json_subscript(const OpReturnType& subscriptVal, const OpReturnType& d);

 public:
  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) final;

  void* get_raw_ptr_at_idx(int idx) { return static_cast<char*>(get_raw_ptr()) + idx * _elemSize; }

  auto get_elem_size() const noexcept { return _elemSize; }

  template <typename T>
  const T* begin() noexcept {
    return static_cast<T*>(get_raw_ptr());
  }

  template <typename T>
  const T* end() noexcept {
    return static_cast<T*>(get_raw_ptr()) + get_numElements();
  }
};

/**
 * @brief Slice view of a typed tensor variable
 *
 * This class provides a view into a portion of an existing tensor without copying data.
 * It maintains a reference to the original tensor and provides access to a slice
 * starting from a specified index.
 */
class SliceVariable : public BaseTypedTensorVariable {
  std::shared_ptr<BaseTypedTensorVariable> origTensor = nullptr; /**< Reference to original tensor */
  int startIndex = 0;                                            /**< Starting index of the slice */

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  void* get_raw_ptr() final { return origTensor->get_raw_ptr_at_idx(startIndex); }

 public:
  SliceVariable(std::shared_ptr<BaseTypedTensorVariable> origTensor_, DATATYPE dataType,
                const std::vector<int64_t>& shape_, int startIndex_, int size_);
};

/**
 * @brief Concrete tensor variable with memory management
 *
 * This class represents a complete tensor with its own memory allocation.
 * It handles memory management through construction and destruction, supporting
 * both copy and move semantics for data initialization.
 */
class TensorVariable : public BaseTypedTensorVariable {
  void* variable = nullptr; /**< Raw pointer to tensor data */

 public:
  void* get_raw_ptr() final { return variable; }

 public:
  TensorVariable(DATATYPE dataType) : BaseTypedTensorVariable(dataType) {}

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  bool is_string() override { return false; }

  static OpReturnType copy_tensor_from_raw_data(void* data, DATATYPE dataType,
                                                const std::vector<int64_t>& shape_) {
    return std::make_shared<TensorVariable>(data, dataType, shape_, CreateTensorType::COPY);
  }

  TensorVariable(void* data, DATATYPE dataType, const std::vector<int64_t>& shape,
                 CreateTensorType type);

  TensorVariable(void* data, DATATYPE dataType, int s, CreateTensorType type)
      : TensorVariable(data, dataType, std::vector<int64_t>{s}, type) {}

  TensorVariable(const std::vector<int64_t>& shape_, DATATYPE dataType);

  virtual ~TensorVariable() { free(variable); }
};

/**
 * @brief Specialized tensor variable for string data
 *
 * This class handles string tensors with specialized operations for string manipulation,
 * sorting, and indexing. It maintains both the string data and shape information
 * for multi-dimensional string arrays.
 */
class StringTensorVariable : public DataVariable {
 protected:
  std::vector<std::string> _data;     /**< Vector containing string elements */
  std::vector<int64_t> _shape;        /**< Tensor dimensions */
  int _numElements = 0;               /**< Total number of string elements */
  std::vector<char*> stringPtrs;      /**< Pointers to string data for C interface */

  void* get_raw_ptr() override { return (void*)_data.data(); }

  const void* get_raw_ptr() const { return (const void*)_data.data(); }

  char** get_string_ptr() final;

  bool get_bool() final { return _numElements; }

  int get_size() final { return _shape.size() ? _shape[0] : 1; }

  int get_dataType_enum() const override { return DATATYPE::STRING; }

  int get_containerType() const override { return CONTAINERTYPE::VECTOR; }

  bool reshape(const std::vector<int64_t>& shape_) final;

  const std::vector<int64_t>& get_shape() override { return _shape; }

  std::string print() override {
    return util::recursive_string<std::string>(_shape, 0, (std::string*)get_raw_ptr(), 0,
                                               _numElements);
  }

  nlohmann::json to_json() const override {
    return util::recursive_json<std::string>(
        _shape, 0, static_cast<const std::string*>(get_raw_ptr()), 0, _numElements);
  }

  OpReturnType get_int_subscript(int index) final;

  bool in(const OpReturnType& elem) override;

  void set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) final;

  int get_numElements() final { return _numElements; }

  OpReturnType sort(const OpReturnType argument) override;

  OpReturnType argsort(const OpReturnType argument) override;

  OpReturnType topk(const std::vector<OpReturnType>& arguments) override;

  OpReturnType arrange(const OpReturnType argument) override;

  bool is_integer() override { return false; }

  bool is_numeric() override { return false; }

  bool is_string() override { return true; }

 public:
  StringTensorVariable() {}

  StringTensorVariable(const std::vector<OpReturnType>& items, int size);

  StringTensorVariable(const std::vector<int64_t>& shape_);

  StringTensorVariable(void* data, int64_t* shape, const int dimsLength);

  StringTensorVariable(std::vector<std::string>&& data, std::vector<int64_t>&& shape,
                       const int dimsLength);

  virtual ~StringTensorVariable() { _data.clear(); }
};

/**
 * @brief Slice view of a string tensor variable
 *
 * Similar to SliceVariable but specialized for string tensors, providing
 * a view into a portion of a string tensor without copying the string data.
 */
class StringSliceVariable final : public StringTensorVariable {
  OpReturnType origTensor = nullptr; /**< Reference to original string tensor */
  int startIndex = 0;                /**< Starting index of the slice */

  int get_dataType_enum() const final { return DATATYPE::STRING; }

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  void* get_raw_ptr() final { return ((std::string*)origTensor->get_raw_ptr()) + startIndex; }

 public:
  StringSliceVariable(std::shared_ptr<DataVariable> origTensor_, const std::vector<int64_t>& shape_,
                      int startIndex_, int size_);
};

/**
 * @brief Empty tensor variable representing zero-sized tensors
 *
 * This class represents empty tensors of any data type. It provides
 * consistent behavior for empty tensor operations while maintaining
 * type information for the intended data type.
 */
class EmptyTensorVariable final : public DataVariable {
  const int dataType;           /**< The intended data type for this empty tensor */
  std::vector<int64_t> shape;   /**< Shape with zero elements */

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  void* get_raw_ptr() final { return nullptr; }

  char** get_string_ptr() final { return nullptr; }

  bool is_numeric() final {
    return dataType == INT32 || dataType == INT64 || dataType == FLOAT || dataType == DOUBLE;
  }

  bool is_integer() final { return dataType == INT32 || dataType == INT64; }

  bool is_string() final { return dataType == STRING; }

  int get_dataType_enum() const final { return dataType; }

  std::string print() final { return "[]"; }

  nlohmann::json to_json() const final { return nlohmann::json::array(); }

  bool get_bool() final { return false; }

  int get_size() final { return 0; }

  const std::vector<int64_t>& get_shape() final { return shape; }

  int get_numElements() final { return 0; }

 public:
  EmptyTensorVariable(int dataType) : dataType(dataType) {
    shape.resize(1);
    shape[0] = 0;
  }
};
