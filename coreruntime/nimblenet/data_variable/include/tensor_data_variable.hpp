#pragma once

#include <string>
#include <vector>

#include "data_variable.hpp"
#include "data_variable_enums.hpp"
#include "ne_fwd.hpp"
#include "util.hpp"

class BaseTensorVariable : public DataVariable {
 protected:
  std::vector<int64_t> shape;
  int numElements = 0;

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

class BaseTypedTensorVariable : public BaseTensorVariable {
 protected:
  const DATATYPE _dataType;
  const int _elemSize;

 protected:
  int get_dataType_enum() const final { return _dataType; }

  // Helper function to get size in bytes corresponding to data type
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

class SliceVariable : public BaseTypedTensorVariable {
  std::shared_ptr<BaseTypedTensorVariable> origTensor = nullptr;
  int startIndex = 0;

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  void* get_raw_ptr() final { return origTensor->get_raw_ptr_at_idx(startIndex); }

 public:
  SliceVariable(std::shared_ptr<BaseTypedTensorVariable> origTensor_, DATATYPE dataType,
                const std::vector<int64_t>& shape_, int startIndex_, int size_);
};

class TensorVariable : public BaseTypedTensorVariable {
  void* variable = nullptr;

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

class StringTensorVariable : public DataVariable {
 protected:
  std::vector<std::string> _data;
  std::vector<int64_t> _shape;
  int _numElements = 0;
  std::vector<char*> stringPtrs;

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

class StringSliceVariable final : public StringTensorVariable {
  OpReturnType origTensor = nullptr;
  int startIndex = 0;

  int get_dataType_enum() const final { return DATATYPE::STRING; }

  int get_containerType() const final { return CONTAINERTYPE::VECTOR; }

  void* get_raw_ptr() final { return ((std::string*)origTensor->get_raw_ptr()) + startIndex; }

 public:
  StringSliceVariable(std::shared_ptr<DataVariable> origTensor_, const std::vector<int64_t>& shape_,
                      int startIndex_, int size_);
};

class EmptyTensorVariable final : public DataVariable {
  const int dataType;
  std::vector<int64_t> shape;

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
