#include "tensor_data_variable.hpp"

#include <algorithm>
#include <vector>

#include "single_variable.hpp"
#include "util.hpp"

bool BaseTensorVariable::reshape(const std::vector<int64_t>& shape_) {
  int size_ = 1;
  for (auto x : shape_) {
    size_ *= x;
  }
  if (numElements != size_) {
    LOG_TO_CLIENT_ERROR("cannot reshape numElements don't match %d %d", numElements, size_);
    return false;
  }
  shape = shape_;
  return true;
}

int BaseTypedTensorVariable::get_elem_size(DATATYPE dataType) {
  switch (dataType) {
    case INT32:
      return sizeof(int32_t);
    case INT64:
      return sizeof(int64_t);
    case FLOAT:
      return sizeof(float);
    case DOUBLE:
      return sizeof(double);
    case BOOLEAN:
      return sizeof(bool);
    default:
      THROW("Datatype %s not supported", util::get_string_from_enum(dataType));
  }
}

bool BaseTypedTensorVariable::in(const OpReturnType& elem) {
  auto func = [this, elem](auto typeObj) {
    using T = decltype(typeObj);

    T checkVal = elem->get<T>();
    T* rawPtr = (T*)get_raw_ptr();
    for (int i = 0; i < numElements; i++) {
      if (rawPtr[i] == checkVal) {
        return true;
      }
    }
    return false;
  };

  return util::call_function_for_dataType(func, _dataType);
}

std::string BaseTypedTensorVariable::print() {
  switch (get_dataType_enum()) {
    case DATATYPE::FLOAT:
      return util::recursive_string<float>(shape, 0, (float*)get_raw_ptr(), 0, numElements);
    case DATATYPE::DOUBLE:
      return util::recursive_string<double>(shape, 0, (double*)get_raw_ptr(), 0, numElements);
    case DATATYPE::INT64:
      return util::recursive_string<int64_t>(shape, 0, (int64_t*)get_raw_ptr(), 0, numElements);
    case DATATYPE::INT32:
      return util::recursive_string<int32_t>(shape, 0, (int32_t*)get_raw_ptr(), 0, numElements);
    case DATATYPE::BOOLEAN:
      return util::recursive_string<bool>(shape, 0, (bool*)get_raw_ptr(), 0, numElements);
    case DATATYPE::JSON:
      return ((nlohmann::json*)get_raw_ptr())->dump();
  }
  return DataVariable::fallback_print();
}

nlohmann::json BaseTypedTensorVariable::to_json() const {
  auto func = [this](auto typeObj) {
    using T = decltype(typeObj);
    // HACK: const_cast to remove const and call its get_raw_ptr. This is until we have a base
    // class const get_raw_ptr
    return util::recursive_json<T>(
        shape, 0, static_cast<T*>(const_cast<BaseTypedTensorVariable*>(this)->get_raw_ptr()), 0,
        numElements);
  };

  return util::call_function_for_dataType(func, _dataType);
}

OpReturnType BaseTypedTensorVariable::get_int_subscript(int index) {
  if (shape.size() == 0) {
    THROW("cannot access index %d of empty shape", index);
  }
  int size = shape[0];
  if (index >= size || index < 0) {
    THROW("trying to access %d index for tensor of size=%d", index, size);
  }

  if (shape.size() == 1) {
    if (_dataType == DATATYPE::JSON) {
      nlohmann::json* val = (nlohmann::json*)get_raw_ptr();
      return OpReturnType(new JSONSingleVariable<nlohmann::json>((*val).at(index)));
    } else {
      auto func = [this, index](auto typeObj) {
        using T = decltype(typeObj);
        T val = ((T*)get_raw_ptr())[index];
        return OpReturnType(new SingleVariable<T>(val));
      };
      return util::call_function_for_dataType(func, _dataType);
    }
  } else {
    int sizeOfSlice = (numElements / shape[0]);
    int startIndex = (numElements / shape[0]) * index;
    std::vector<int64_t> newShape = shape;
    newShape.erase(newShape.begin());
    return OpReturnType(
        new SliceVariable(std::static_pointer_cast<BaseTypedTensorVariable>(shared_from_this()),
                          _dataType, newShape, startIndex, sizeOfSlice));
  }
}

OpReturnType BaseTypedTensorVariable::get_string_subscript(const std::string& key) {
  THROW("%s", "get_string_subscript not available.");
}

void BaseTypedTensorVariable::set_subscript(const OpReturnType& subscriptVal,
                                            const OpReturnType& d) {
  if (_dataType == DATATYPE::JSON) {
    set_json_subscript(subscriptVal, d);
    return;
  }

  int index = subscriptVal->get_int32();
  if (shape.size() == 0) {
    THROW("cannot set index %d of empty shape", index);
  }
  int size = shape[0];
  if (index >= size || index < 0) {
    THROW("trying to set %d index for tensor of size=%d", index, size);
  }

  int numElementsForSetting = numElements / shape[0];
  if (numElementsForSetting == 1 && shape.size() == 1) {
    auto func = [this, d, index](auto typeObj) {
      using T = decltype(typeObj);
      ((T*)get_raw_ptr())[index] = d->get<T>();
    };
    util::call_function_for_dataType(func, _dataType);
    return;
  }
  // allowing to single value as above. but not allowing to set tensor with different types
  if (d->get_dataType_enum() != get_dataType_enum()) {
    THROW("datatype not matching for setting %s, %s",
          util::get_string_from_enum(get_dataType_enum()),
          util::get_string_from_enum(d->get_dataType_enum()));
  }
  auto shape1 = d->get_shape();
  if (shape.size() - 1 != shape1.size()) {
    THROW("%s", "shape not matching for assignment");
  }
  for (int i = 0; i < shape1.size(); i++) {
    if (shape1[i] != shape[i + 1]) {
      THROW("shape not matching expected %d at index %d, but got %d", shape[i + 1], i, shape1[i]);
    }
  }
  memcpy((char*)get_raw_ptr() + _elemSize * (index * numElements / shape[0]), d->get_raw_ptr(),
         numElementsForSetting * _elemSize);
  return;
}

void BaseTypedTensorVariable::set_json_subscript(const OpReturnType& subscriptVal,
                                                 const OpReturnType& d) {
  int index = subscriptVal->get_int32();
  if (shape.size() == 0) {
    THROW("cannot set index %d of empty shape", index);
  }
  if (shape.size() > 1) {
    THROW("%s", "Cannot set json object inside multi dimensional tensor.");
  }
  nlohmann::json* input = (nlohmann::json*)get_raw_ptr();
  if (index >= numElements || index < 0) {
    THROW("trying to set %d index for json of size=%d", index, input->size());
  }

  if (d->get_dataType_enum() != get_dataType_enum()) {
    THROW("datatype not matching for setting %s, %s",
          util::get_string_from_enum(get_dataType_enum()),
          util::get_string_from_enum(d->get_dataType_enum()));
  }
  (*input)[index] = d->get<nlohmann::json>();
  return;
}

OpReturnType BaseTypedTensorVariable::sort(const OpReturnType argument) {
  if (shape.size() != 1) {
    THROW("sort expects tensor to be of 1 dimension. Given %d dimensions.", shape.size());
  }

  if (_dataType == JSON) {
    THROW("%s", "sort not available for JSON tensor.");
  }
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(argument->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::SORT);
  if (argument->get_string() != "asc" && argument->get_string() != "desc") {
    THROW("Argument of sort should be either asc/desc. Given %s argument.",
          argument->get_string().c_str());
  }

  auto func = [this, argument](auto typeObj) {
    using T = decltype(typeObj);
    T* tensor_data = static_cast<T*>(get_raw_ptr());
    std::string sortType = argument->get_string();
    if (sortType == "asc") {
      std::sort(tensor_data, tensor_data + get_size(), [](T a, T b) { return a < b; });
    } else {
      std::sort(tensor_data, tensor_data + get_size(), [](T a, T b) { return a > b; });
    }
  };
  util::call_function_for_dataType(func, _dataType);
  return shared_from_this();
}

OpReturnType BaseTypedTensorVariable::argsort(const OpReturnType argument) {
  if (shape.size() != 1) {
    THROW("argsort expects tensor to be of 1 dimension. Given %d dimensions.", shape.size());
  }
  if (_dataType == JSON) {
    THROW("%s", "argsort not available for JSON tensor.");
  }
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(argument->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::ARGSORT);
  if (argument->get_string() != "asc" && argument->get_string() != "desc") {
    THROW("Argument of argsort should be either asc/desc. Given %s argument.",
          argument->get_string().c_str());
  }
  int32_t* indices = (int32_t*)malloc(sizeof(int32_t) * shape[0]);
  std::iota(indices, indices + shape[0], 0);
  std::string sortType = argument->get_string();

  auto func = [this, &sortType, indices](auto typeObj) {
    using T = decltype(typeObj);
    T* tensor_data = static_cast<T*>(get_raw_ptr());
    if (sortType == "asc") {
      std::stable_sort(indices, indices + shape[0], [tensor_data](size_t i1, size_t i2) {
        return tensor_data[i1] < tensor_data[i2];
      });
    } else {
      std::stable_sort(indices, indices + shape[0], [tensor_data](size_t i1, size_t i2) {
        return tensor_data[i1] > tensor_data[i2];
      });
    }
  };
  util::call_function_for_dataType(func, _dataType);
  return OpReturnType(new TensorVariable(indices, INT32, shape, CreateTensorType::MOVE));
}

OpReturnType BaseTypedTensorVariable::topk(const std::vector<OpReturnType>& arguments) {
  if (shape.size() != 1) {
    THROW("topk expects tensor to be of 1 dimension. Given %d dimensions.", shape.size());
  }
  if (_dataType == JSON) {
    THROW("%s", "topk not available for JSON tensor.");
  }

  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::TOPK);
  if (arguments[1]->get_string() != "asc" && arguments[1]->get_string() != "desc") {
    THROW("Second argument of topk should be either asc/desc. Given %s argument.",
          arguments[1]->get_string().c_str());
  }
  int numOfElements = arguments[0]->get_int32();
  if (numOfElements > shape[0]) {
    THROW(
        "First argument of topk cannot be greater than the size of tensor. Given %d argument "
        "and size of tensor is: %d.",
        numOfElements, shape[0]);
  }
  std::string sortType = arguments[1]->get_string();
  // Using partial_sort for topk and stable_sort for argsort because partial_sort
  // uses heapsort internally which is slower when comparing across the whole range of vector, but
  // faster when sorting a smaller subset of vector
  std::vector<int32_t> idx(shape[0]);
  std::iota(idx.begin(), idx.end(), 0);

  auto func = [this, &sortType, numOfElements, &idx](auto typeObj) {
    using T = decltype(typeObj);
    T* tensor_data = static_cast<T*>(get_raw_ptr());
    if (sortType == "asc") {
      std::partial_sort(
          idx.begin(), idx.begin() + numOfElements, idx.end(),
          [tensor_data](size_t i1, size_t i2) { return tensor_data[i1] < tensor_data[i2]; });
    } else {
      std::partial_sort(
          idx.begin(), idx.begin() + numOfElements, idx.end(),
          [tensor_data](size_t i1, size_t i2) { return tensor_data[i1] > tensor_data[i2]; });
    }
  };
  util::call_function_for_dataType(func, _dataType);

  int32_t* indices = (int32_t*)malloc(sizeof(int32_t) * numOfElements);
  memcpy(indices, idx.data(), numOfElements * sizeof(int32_t));
  return OpReturnType(new TensorVariable(indices, INT32, numOfElements, CreateTensorType::MOVE));
}

OpReturnType BaseTypedTensorVariable::arrange(const OpReturnType argument) {
  if (argument->get_containerType() != CONTAINERTYPE::VECTOR &&
      argument->get_containerType() != CONTAINERTYPE::LIST) {
    THROW("Argument of arrange should be a tensor/list, provided %s",
          argument->get_containerType_string());
  }
  if (argument->get_containerType() == CONTAINERTYPE::VECTOR && argument->get_shape().size() != 1) {
    THROW("Argument of arrange if tensor, should be of dimension 1, provided %d dimensions",
          argument->get_shape().size());
  }
  if (shape.size() != 1) {
    THROW("arrange expects tensor to be of 1 dimension. Given %d dimensions.", shape.size());
  }
  int size = argument->get_size();
  if (size > shape[0]) {
    THROW(
        "Elements present in argument of arrange should less than or equal to elements present "
        "in "
        "tensor, provided %d elements for a tensor of size %d",
        size, shape[0]);
  }

  char* tensor_data = static_cast<char*>(get_raw_ptr());
  char* data = (char*)malloc(_elemSize * size);
  for (int i = 0; i < size; i++) {
    OpReturnType index = argument->get_int_subscript(i);
    if (!index->is_integer()) {
      THROW("Element present in argument of arrange at index=%d should be of type int, provided %s",
            i, util::get_string_from_enum(index->get_dataType_enum()));
    }
    if (index->get_int32() < 0 || index->get_int32() >= shape[0]) {
      THROW("Tried to access %d index of the tensor.", index->get_int32());
    }
    memcpy(data + i * _elemSize, tensor_data + index->get_int32() * _elemSize, _elemSize);
  }
  return OpReturnType(new TensorVariable(data, _dataType, size, CreateTensorType::MOVE));
}

void StringTensorVariable::set_subscript(const OpReturnType& subscriptVal, const OpReturnType& d) {
  int index = subscriptVal->get_int32();
  if (_shape.size() == 0) {
    THROW("cannot set index %d of empty shape", index);
  }
  int size = _shape[0];
  if (index >= size || index < 0) {
    THROW("trying to set %d index for tensor of size=%d", index, size);
  }

  if (d->get_dataType_enum() != get_dataType_enum()) {
    THROW("datatype not matching for setting %s, %s",
          util::get_string_from_enum(get_dataType_enum()),
          util::get_string_from_enum(d->get_dataType_enum()));
  }
  int numElementsForSetting = _numElements / _shape[0];
  if (numElementsForSetting == 1 && _shape.size() == 1) {
    ((std::string*)get_raw_ptr())[index] = d->get_string();
    return;
  }

  if (d->get_size() != numElementsForSetting) {
    auto shape1 = d->get_shape();
    for (int i = 0; i < shape1.size(); i++) {
      if (shape1[i] != _shape[i + 1]) {
        THROW("shape not matching expected %d at index %d, but got %d", _shape[i + 1], i,
              shape1[i]);
      }
    }
  }

  for (int i = 0; i < numElementsForSetting; i++) {
    ((std::string*)get_raw_ptr())[i + index] = d->get_int_subscript(i)->get_string();
  }
  return;
}

OpReturnType StringTensorVariable::get_int_subscript(int index) {
  if (_shape.size() == 0) {
    THROW("cannot access index %d of empty shape", index);
  }
  int size = _shape[0];
  if (index >= size || index < 0) {
    THROW("trying to access %d index for tensor of size=%d", index, size);
  }
  if (_shape.size() == 1) {
    std::string val = ((std::string*)get_raw_ptr())[index];
    return OpReturnType(new SingleVariable<std::string>(val));
  } else {
    int sizeOfSlice = (_numElements / _shape[0]);
    int startIndex = (_numElements / _shape[0]) * index;
    std::vector<int64_t> newShape = _shape;
    newShape.erase(newShape.begin());
    return OpReturnType(
        new StringSliceVariable(shared_from_this(), newShape, startIndex, sizeOfSlice));
  }
}

OpReturnType StringTensorVariable::sort(const OpReturnType argument) {
  if (_shape.size() != 1) {
    THROW("sort expects tensor to be of 1 dimension. Given %d dimensions.", _shape.size());
  }
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(argument->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::SORT);
  if (argument->get_string() != "asc" && argument->get_string() != "desc") {
    THROW("Argument of sort should be either asc/desc. Given %s argument.",
          argument->get_string().c_str());
  }

  std::string sortType = argument->get_string();
  if (sortType == "asc") {
    std::sort(_data.begin(), _data.end(), [](std::string a, std::string b) { return a < b; });
  } else {
    std::sort(_data.begin(), _data.end(), [](std::string a, std::string b) { return a > b; });
  }
  std::vector<int64_t> shape = get_shape();
  return shared_from_this();
}

OpReturnType StringTensorVariable::argsort(const OpReturnType argument) {
  if (_shape.size() != 1) {
    THROW("argsort expects tensor to be of 1 dimension. Given %d dimensions.", _shape.size());
  }
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(argument->get_dataType_enum(), DATATYPE::STRING, 0,
                                    MemberFuncType::ARGSORT);
  if (argument->get_string() != "asc" && argument->get_string() != "desc") {
    THROW("Argument of argsort should be either asc/desc. Given %s argument.",
          argument->get_string().c_str());
  }
  int32_t* indices = (int32_t*)malloc(_shape[0] * sizeof(int32_t));
  std::iota(indices, indices + _shape[0], 0);
  std::string sortType = argument->get_string();
  std::vector<std::string> data = _data;
  if (sortType == "asc") {
    std::stable_sort(indices, indices + _shape[0],
                     [data](size_t i1, size_t i2) { return data[i1] < data[i2]; });
  } else {
    std::stable_sort(indices, indices + _shape[0],
                     [data](size_t i1, size_t i2) { return data[i1] > data[i2]; });
  }

  return OpReturnType(new TensorVariable(indices, INT32, _shape, CreateTensorType::MOVE));
}

OpReturnType StringTensorVariable::topk(const std::vector<OpReturnType>& arguments) {
  if (_shape.size() != 1) {
    THROW("topk expects tensor to be of 1 dimension. Given %d dimensions.", _shape.size());
  }
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[1]->get_dataType_enum(), DATATYPE::STRING, 1,
                                    MemberFuncType::TOPK);
  if (arguments[1]->get_string() != "asc" && arguments[1]->get_string() != "desc") {
    THROW("Argument of topk should be either asc/desc. Given %s argument.",
          arguments[1]->get_string().c_str());
  }

  int numOfElements = arguments[0]->get_int32();
  if (numOfElements > _shape[0]) {
    THROW(
        "First argument of topk cannot be greater than the shape of tensor. Given %d argument "
        "and size of tensor is: %d.",
        numOfElements, _shape[0]);
  }
  std::string sortType = arguments[1]->get_string();
  std::vector<int32_t> idx(_shape[0]);
  std::iota(idx.begin(), idx.end(), 0);
  std::vector<std::string> data = _data;
  if (sortType == "asc") {
    std::partial_sort(idx.begin(), idx.begin() + numOfElements, idx.end(),
                      [data](size_t i1, size_t i2) { return data[i1] < data[i2]; });
  } else {
    std::partial_sort(idx.begin(), idx.begin() + numOfElements, idx.end(),
                      [data](size_t i1, size_t i2) { return data[i1] > data[i2]; });
  }
  int32_t* indices = (int32_t*)malloc(sizeof(int32_t) * numOfElements);
  memcpy(indices, idx.data(), numOfElements * sizeof(int32_t));
  return OpReturnType(new TensorVariable(indices, INT32, numOfElements, CreateTensorType::MOVE));
}

OpReturnType StringTensorVariable::arrange(const OpReturnType argument) {
  if (argument->get_containerType() != CONTAINERTYPE::VECTOR &&
      argument->get_containerType() != CONTAINERTYPE::LIST) {
    THROW("Argument of arrange should be a tensor/list, provided %s",
          argument->get_containerType_string());
  }
  if (argument->get_containerType() == CONTAINERTYPE::VECTOR && argument->get_shape().size() != 1) {
    THROW("Argument of arrange if tensor, should be of dimension 1, provided %d dimensions",
          argument->get_shape().size());
  }
  if (_shape.size() != 1) {
    THROW("arrange expects tensor to be of 1 dimension. Given %d dimensions.", _shape.size());
  }
  int size = argument->get_size();
  if (size > _shape[0]) {
    THROW(
        "Elements present in argument of arrange should less than or equal to elements present "
        "in "
        "tensor, provided %d elements for a tensor of size %d",
        size, _shape[0]);
  }
  std::vector<std::string> data;
  for (int i = 0; i < size; i++) {
    OpReturnType index = argument->get_int_subscript(i);
    if (!index->is_integer()) {
      THROW("Element present in argument of arrange at index=%d should be of type int, provided %s",
            i, util::get_string_from_enum(index->get_dataType_enum()));
    }
    if (index->get_int32() < 0 || index->get_int32() >= _shape[0]) {
      THROW("Tried to access %d index of the tensor.", index->get_int32());
    }
    data.push_back(_data[index->get_int32()]);
  }
  std::vector<int64_t> shape(1, size);
  return OpReturnType(new StringTensorVariable(std::move(data), std::move(shape), 1));
}

SliceVariable::SliceVariable(std::shared_ptr<BaseTypedTensorVariable> origTensor_,
                             DATATYPE dataType, const std::vector<int64_t>& shape_, int startIndex_,
                             int size_)
    : BaseTypedTensorVariable(dataType) {
  origTensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(origTensor_);
  if (!origTensor) {
    THROW("Unable to cast tensor to BaseTypedTensorVariable");
  }
  BaseTensorVariable::shape = shape_;
  startIndex = startIndex_;
  BaseTensorVariable::numElements = size_;
}

TensorVariable::TensorVariable(void* data, DATATYPE dataType, const std::vector<int64_t>& shape,
                               CreateTensorType type)
    : BaseTypedTensorVariable(dataType) {
  int length = 1;
  for (int i = 0; i < shape.size(); i++) {
    length *= shape[i];
  }
  BaseTensorVariable::numElements = length;
  BaseTensorVariable::shape = shape;

  switch (type) {
    case CreateTensorType::MOVE:
      variable = (void*)data;
      break;
    case CreateTensorType::COPY: {
      int totalBytes = BaseTensorVariable::numElements * _elemSize;
      variable = malloc(totalBytes);
      memcpy(variable, data, totalBytes);
      break;
    }
  };
}

TensorVariable::TensorVariable(const std::vector<int64_t>& shape_, DATATYPE dataType)
    : BaseTypedTensorVariable(dataType) {
  BaseTensorVariable::shape = shape_;
  BaseTensorVariable::numElements = 1;
  for (auto x : BaseTensorVariable::shape) {
    if (x <= 0) {
      THROW("dimension %ld is invalid", x);
    }
    BaseTensorVariable::numElements *= x;
  }
  int totalBytes = BaseTensorVariable::numElements * _elemSize;
  variable = malloc(totalBytes);
  memset(variable, 0, totalBytes);
}

char** StringTensorVariable::get_string_ptr() {
  stringPtrs.clear();
  for (int i = 0; i < _numElements; i++) {
    stringPtrs.push_back((char*)_data[i].c_str());
  }
  return stringPtrs.data();
}

bool StringTensorVariable::reshape(const std::vector<int64_t>& shape_) {
  int size_ = 1;
  for (const auto x : shape_) {
    size_ *= x;
  }
  if (_numElements != size_) {
    LOG_TO_CLIENT_ERROR("cannot reshape numElements don't match %d %d", _numElements, size_);
    return false;
  }
  _shape = shape_;
  return true;
}

bool StringTensorVariable::in(const OpReturnType& elem) {
  if (elem->get_containerType() == CONTAINERTYPE::SINGLE &&
      elem->get_dataType_enum() == DATATYPE::STRING) {
    std::string checkVal = elem->get_string();
    for (const auto& it : _data) {
      if (it == checkVal) return true;
    }
    return false;
  }
  return false;
}

StringTensorVariable::StringTensorVariable(const std::vector<OpReturnType>& items, int size) {
  for (int i = 0; i < size; i++) {
    _data.emplace_back(items[i]->get_string());
  }
  _numElements = size;
  _shape.push_back(size);
}

StringTensorVariable::StringTensorVariable(const std::vector<int64_t>& shape_) {
  int length = 1;
  for (auto it : shape_) {
    length *= it;
  }
  _data.resize(length);
  _shape = shape_;
  _numElements = length;
}

StringTensorVariable::StringTensorVariable(void* data, int64_t* shape, const int dimsLength) {
  int numElements = 1;
  for (int i = 0; i < dimsLength; i++) {
    numElements *= shape[i];
  }
  _numElements = numElements;
  _shape = std::vector<int64_t>(shape, shape + dimsLength);

  char** stringVec = static_cast<char**>(data);
  for (int i = 0; i < numElements; i++) {
    _data.push_back(std::string(stringVec[i]));
  }
}

StringTensorVariable::StringTensorVariable(std::vector<std::string>&& data,
                                           std::vector<int64_t>&& shape, const int dimsLength) {
  int numElements = 1;
  for (int i = 0; i < dimsLength; i++) {
    numElements *= shape[i];
  }
  _numElements = numElements;
  _shape = shape;
  _data = std::move(data);
}

StringSliceVariable::StringSliceVariable(std::shared_ptr<DataVariable> origTensor_,
                                         const std::vector<int64_t>& shape_, int startIndex_,
                                         int size_) {
  origTensor = origTensor_;
  StringTensorVariable::_shape = shape_;
  startIndex = startIndex_;
  StringTensorVariable::_numElements = size_;
}
