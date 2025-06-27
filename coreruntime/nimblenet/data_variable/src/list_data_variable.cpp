#include "list_data_variable.hpp"

#include "tensor_data_variable.hpp"

OpReturnType ListOperators::create_tensor(int dataType, OpReturnType list) {
  // Create empty tensor, if the list is empty
  if (list->get_size() == 0) {
    return OpReturnType(new EmptyTensorVariable(dataType));
  }

  std::vector<int64_t> shape;
  int size = 1;
  auto initial = list;
  while (!initial->is_single()) {
    shape.push_back(initial->get_size());
    size *= initial->get_size();
    initial = initial->get_int_subscript(0);
  }

  switch (dataType) {
    case DATATYPE::FLOAT:
      return ListOperators::operate<float>(list, std::move(shape), size);
    case DATATYPE::INT32:
      return ListOperators::operate<int32_t>(list, std::move(shape), size);
    case DATATYPE::DOUBLE:
      return ListOperators::operate<double>(list, std::move(shape), size);
    case DATATYPE::INT64:
      return ListOperators::operate<int64_t>(list, std::move(shape), size);
    case DATATYPE::STRING:
      return ListOperators::operate_string(list, std::move(shape), size);
    default:
      THROW("dataType=%s cannot be converted to tensor", util::get_string_from_enum(dataType));
  }
}

OpReturnType ListOperators::operate_string(OpReturnType list, std::vector<int64_t>&& shape,
                                           int size) {
  std::vector<std::string> stringVec(size);
  for (int i = 0; i < size; i++) {
    stringVec[i] = get_element<std::string>(list, shape, i, size);
  }
  return OpReturnType(
      new StringTensorVariable(std::move(stringVec), std::move(shape), shape.size()));
}
