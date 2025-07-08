/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <fstream>

#include "client.h"
#include "input_structs.hpp"
#include "nimblenet.h"
#include "nlohmann/json.hpp"
#include "task_input_structs.hpp"

namespace py = pybind11;
using namespace py::literals;

char* set_name(const std::string& value) {
  char* name = new char[value.size() + 1];
  std::copy(value.begin(), value.end(), name);
  name[value.size()] = '\0';
  return name;
}

std::map<std::string, DATATYPE> numpyToNimbleType = {
    {"float32", DATATYPE::FLOAT}, {"int32", DATATYPE::INT32},   {"bool", DATATYPE::BOOLEAN},
    {"int64", DATATYPE::INT64},   {"double", DATATYPE::DOUBLE}, {"float64", DATATYPE::DOUBLE},
};

template <typename T>
CTensor assign_CTensor(const std::string& name, py::array_t<T> moved_arr) {
  py::buffer_info info = moved_arr.request();
  py::list shapeList;
  int64_t* copiedShape = new int64_t[info.shape.size()];
  for (int i = 0; i < info.shape.size(); i++) {
    copiedShape[i] = info.shape[i];
  }
  CTensor tensor;
  tensor.name = set_name(name);
  tensor.shapeLength = info.shape.size();
  tensor.shape = copiedShape;

  py::dtype dt = moved_arr.dtype();
  std::string dtypeName = dt.attr("name").cast<std::string>();
  tensor.dataType = numpyToNimbleType.at(dtypeName);
  void* newMemory = malloc(moved_arr.nbytes());
  std::memcpy(newMemory, moved_arr.data(), moved_arr.nbytes());
  tensor.data = newMemory;
  return tensor;
}

nlohmann::json convert_py_dict_to_json(py::dict py_dict) {
  try {
    py::module_ json_module = py::module_::import("json");
    py::str json_str = json_module.attr("dumps")(py_dict);
    std::string cpp_json_str = py::cast<std::string>(json_str);
    return nlohmann::json::parse(cpp_json_str);
  } catch (const py::error_already_set& e) {
    // Handle Python exceptions
    PyErr_Clear();  // Clear the Python error state
    throw std::runtime_error("Invalid dict provided.");
  } catch (const nlohmann::json::parse_error& e) {
    // Handle JSON parsing errors
    throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
  }
}

CTensor assign_CTensor_list(const std::string& name, py::list&& list) {
  CTensor tensor;
  tensor.name = set_name(name);
  tensor.shapeLength = 1;
  int64_t* copiedShape = new int64_t(list.size());
  tensor.shape = copiedShape;
  tensor.dataType = DATATYPE::JSON_ARRAY;
  // Convert both json and string arrays to JSON_ARRAY
  nlohmann::json jsonArray = nlohmann::json(nlohmann::json::array());
  for (const auto& item : list) {
    py::module_ json_module = py::module_::import("json");
    py::str json_str = json_module.attr("dumps")(item);
    std::string cpp_json_str = py::cast<std::string>(json_str);
    jsonArray.push_back(nlohmann::json::parse(cpp_json_str));
  }
  void* listDataVariable =
      TaskInputData::get_list_from_json_object_for_simulator(std::move(jsonArray));
  tensor.data = (void*)(listDataVariable);
  return tensor;
}

bool load_task_in_simulator(const char* taskFilePath) {
  py::gil_scoped_acquire acquire;  // Acquire the GIL

  auto locals = py::dict("fileName"_a = taskFilePath);
  py::exec(R"(
          import ast
          import ast2json
          import json
          f = open(fileName, 'r')
          tree = ast2json.ast2json(ast.parse(f.read()))
          parsedAST = json.dumps(tree, indent=2)
  )",
           py::globals(), locals);

  std::string taskCodeString = locals["parsedAST"].cast<std::string>();
  char* taskCode = new char[taskCodeString.size() + 1];
  std::strcpy(taskCode, taskCodeString.c_str());
  taskCode[taskCodeString.size()] = 0;

  bool r = load_task(taskCode);
  delete[] taskCode;
  return r;
}

std::map<std::string, py::object> convert_CTensors_to_pymap(const CTensors& ret) {
  CTensor* tensors = ret.tensors;
  int numOutputs = ret.numTensors;

  std::map<std::string, py::object> py_outputs;
  for (int i = 0; i < numOutputs; i++) {
    int64_t* shape = tensors[i].shape;
    int shape_length = tensors[i].shapeLength;
    int type = tensors[i].dataType;
    std::string name = tensors[i].name;
    void* data = tensors[i].data;
    // If shape_length is zero then create and return single variable else create a py::array<T>
    if (shape_length == 0) {
      switch (type) {
        case DATATYPE::INT64:
          py_outputs[name] = py::int_(*(int64_t*)data);
          break;
        case DATATYPE::FLOAT:
          py_outputs[name] = py::float_(*(float*)data);
          break;
        case DATATYPE::BOOLEAN:
          py_outputs[name] = py::bool_(*(bool*)data);
          break;
        case DATATYPE::INT32:
          py_outputs[name] = py::int_(*(int32_t*)data);
          break;
        case DATATYPE::DOUBLE:
          py_outputs[name] = py::float_(*(double*)data);
          break;
        case DATATYPE::STRING: {
          py_outputs[name] = py::str(((char**)data)[0]);
          break;
        }
        case DATATYPE::JSON: {
          nlohmann::json jsonData = TaskInputData::get_json_from_OpReturnType(data);
          py::dict result = py::module::import("json").attr("loads")(jsonData.dump());
          py_outputs[name] = result;
          break;
        }
      }
      continue;
    }

    int64_t length = 1;

    std::vector<ssize_t> shape_vector;
    for (int j = 0; j < shape_length; j++) {
      shape_vector.push_back((int)shape[j]);
      length *= shape[j];
    }

    py::dtype dtype;
    switch (type) {
      case DATATYPE::FLOAT:
        dtype = py::dtype::of<float>();
        break;
      case DATATYPE::BOOLEAN:
        dtype = py::dtype::of<bool>();
        break;
      case DATATYPE::INT32:
        dtype = py::dtype::of<int32_t>();
        break;
      case DATATYPE::INT64:
        dtype = py::dtype::of<long long>();
        break;
      case DATATYPE::DOUBLE:
        dtype = py::dtype::of<double>();
        break;
      case DATATYPE::STRING: {
        if (data == nullptr) {
          py_outputs[name] = py::array(py::dtype("U"), shape_vector, data);
          break;
        }
        char** charArray = static_cast<char**>(data);
        std::vector<std::string> strings(length);
        for (int i = 0; i < length; i++) {
          strings[i] = charArray[i];
        }
        py::array tempStringArray = py::array(py::cast(strings));
        tempStringArray.resize(shape_vector);
        py_outputs[name] = tempStringArray;
        break;
      }
      case DATATYPE::JSON_ARRAY: {
        if (data == nullptr) {
          py_outputs[name] = py::array(py::dtype("O"), shape_vector, data);
          break;
        }
        nlohmann::json jsonData = TaskInputData::get_json_from_OpReturnType(data);
        py::list list;
        for (int i = 0; i < jsonData.size(); i++) {
          list.append(py::module::import("json").attr("loads")(jsonData.at(i).dump()));
        }
        py_outputs[name] = list;
        break;
      }
      default:
        throw std::runtime_error("DataType=" + std::to_string(type) +
                                 " not supported for key=" + name + " in the output.");
    }
    if (type != DATATYPE::STRING && type != DATATYPE::JSON && type != DATATYPE::JSON_ARRAY) {
      py::array py_array(dtype, shape_vector, data);
      py_outputs[name] = py_array;
    }
  }
  return py_outputs;
}

std::map<std::string, py::object> convert_CTensors_to_pymap_and_free_tensors(
    const CTensors& cTensors) {
  auto ret = convert_CTensors_to_pymap(cTensors);
  auto status = deallocate_output_memory2(const_cast<CTensors*>(&cTensors));
  return ret;
}

template <typename T>
py::list get_shape(const py::array_t<T>& array) {
  py::buffer_info info = array.request();
  py::list shapeList;
  for (size_t dim : info.shape) {
    shapeList.append(dim);
  }
  return shapeList;
}

CTensors convert_pydict_to_CTensors(const py::dict& inputDict);

CTensor construct_singleVariable_input(const std::string& name, py::object item) {
  CTensor cTensor;
  cTensor.name = set_name(name);
  cTensor.shapeLength = 0;
  cTensor.shape = nullptr;
  if (py::isinstance<py::bool_>(item)) {
    cTensor.dataType = DATATYPE::BOOLEAN;
    bool* ptr = static_cast<bool*>(malloc(sizeof(bool)));
    *ptr = py::cast<bool>(item);
    cTensor.data = (void*)ptr;
  } else if (py::isinstance<py::int_>(item)) {
    // pybind does not differentiate between int32 and int64, both are represented by py::int_,
    // hence creating int64_t
    cTensor.dataType = DATATYPE::INT64;
    int64_t* val = static_cast<int64_t*>(malloc(sizeof(int64_t)));
    *val = py::cast<int64_t>(item);
    cTensor.data = (void*)val;
  } else if (py::isinstance<py::float_>(item)) {
    cTensor.dataType = DATATYPE::FLOAT;
    float* val = static_cast<float*>(malloc(sizeof(float)));
    *val = py::cast<float>(item);
    cTensor.data = (void*)val;
  } else if (py::isinstance<py::str>(item)) {
    cTensor.dataType = DATATYPE::STRING;
    std::string val = py::cast<std::string>(item);
    char** input = static_cast<char**>(malloc(sizeof(char*)));
    *input = strdup(val.c_str());
    cTensor.data = (void*)input;
  } else if (py::isinstance<py::dict>(item)) {
    cTensor.dataType = DATATYPE::JSON;
    nlohmann::json json = convert_py_dict_to_json(py::cast<py::dict>(item));
    cTensor.data = TaskInputData::get_map_from_json_object_for_simulator(std::move(json));
  } else if (py::isinstance<py::function>(item)) {
    cTensor.dataType = DATATYPE::FUNCTION;

    // This lambda should be able to run the frontend Function.
    // Context should store info related to running the frontend function, or the function itself
    FrontendFunctionPtr myLambda = [](void* context, const CTensors input,
                                      CTensors* output) -> NimbleNetStatus* {
      auto pythonFunction = py::cast<py::function>(*(py::handle*)context);
      auto inputForPythonFunction = convert_CTensors_to_pymap(input);
      auto returnObject = pythonFunction(inputForPythonFunction);

      *output = convert_pydict_to_CTensors(returnObject);
      return nullptr;
    };
    void* context = new py::object(item);
    cTensor.data = TaskInputData::create_function_data_variable(context, myLambda);
  }
  return cTensor;
}

CTensors convert_pydict_to_CTensors(const py::dict& inputDict) {
  CTensors cTensors;
  cTensors.numTensors = inputDict.size();
  cTensors.tensors = new CTensor[cTensors.numTensors];
  int index = 0;
  for (auto item : inputDict) {
    std::string inputName = py::str(item.first).cast<std::string>();
    if (py::isinstance<py::list>(item.second)) {
      cTensors.tensors[index++] = assign_CTensor_list(inputName, py::cast<py::list>(item.second));
    } else if (py::isinstance<py::array>(item.second)) {
      py::array inputItems = py::cast<py::array>(item.second);
      py::dtype dtype = inputItems.dtype();
      if (dtype.is(py::dtype::of<float>())) {
        cTensors.tensors[index++] = assign_CTensor<float>(
            inputName,
            py::array_t<float, py::array::c_style | py::array::forcecast>::ensure(item.second));
      } else if (dtype.is(py::dtype::of<bool>())) {
        cTensors.tensors[index++] = assign_CTensor<bool>(
            inputName,
            py::array_t<bool, py::array::c_style | py::array::forcecast>::ensure(item.second));
      } else if (dtype.is(py::dtype::of<int32_t>())) {
        cTensors.tensors[index++] = assign_CTensor<int32_t>(
            inputName,
            py::array_t<int32_t, py::array::c_style | py::array::forcecast>::ensure(item.second));
      } else if (dtype.is(py::dtype::of<long long>())) {
        cTensors.tensors[index++] = assign_CTensor<int64_t>(
            inputName,
            py::array_t<int64_t, py::array::c_style | py::array::forcecast>::ensure(item.second));
      } else if (dtype.is(py::dtype::of<double>())) {
        cTensors.tensors[index++] = assign_CTensor<double>(
            inputName,
            py::array_t<double, py::array::c_style | py::array::forcecast>::ensure(item.second));
      } else {
        throw std::runtime_error("Invalid data type of input.");
      }
    } else if (py::isinstance<py::bool_>(item.second) || py::isinstance<py::int_>(item.second) ||
               py::isinstance<py::float_>(item.second) || py::isinstance<py::str>(item.second) ||
               py::isinstance<py::dict>(item.second) || py::isinstance<py::function>(item.second)) {
      cTensors.tensors[index++] = construct_singleVariable_input(
          inputName, py::reinterpret_borrow<py::object>(item.second));
    } else {
      throw std::runtime_error(
          "Input dict supports the following types, numpy array, functions, list of string/json "
          "objects and "
          "single variables of type int/float/double/bool/string.");
    }
  }
  return cTensors;
}

class RAIITensors {
 public:
  CTensors t;

  RAIITensors(CTensors t_) { t = t_; }

  ~RAIITensors() { globalDeallocate(t); }
};

std::map<std::string, py::object> run_task_upto_timestamp_in_simulator(const char* functionName,
                                                                       py::dict taskInputDataDict,
                                                                       py::object timestampArg) {
  RAIITensors input(convert_pydict_to_CTensors(taskInputDataDict));
  CTensors output;

  if (timestampArg.is_none()) {
    auto t = run_method(functionName, input.t, &output);
    if (t != nullptr) {
      throw std::runtime_error(std::string(t->message) + "\nError running workflow script.");
    }
    auto ret = convert_CTensors_to_pymap_and_free_tensors(output);
    return ret;
  }
  int64_t timestamp = timestampArg.cast<int64_t>();
  if (!run_task_upto_timestamp(functionName, input.t, &output, timestamp)) {
    throw std::runtime_error("Error running workflow script.");
  }
  return convert_CTensors_to_pymap_and_free_tensors(output);
}

std::unordered_set<std::string> get_build_flags_set() {
  static std::unordered_set<std::string> ret;
  static bool firstTime = true;

  if (!firstTime) {
    return ret;
  }

  firstTime = false;
  const char** cBuildFlags = get_build_flags();
  int idx = 0;

  while (cBuildFlags[idx]) {
    ret.insert(std::string{cBuildFlags[idx]});
    idx++;
  }

  delete[] cBuildFlags;
  return ret;
}

void load_task(py::module_& m) {
  m.def("load_workflow_script", &load_task_in_simulator,
        R"(
    Function to get workflow script as a python file and load it in simulator.

    Attributes :
    scriptFilePath : Path to workflow script python file.

    Return value :
    bool : True if the load was successful else false.
  )",
        py::arg("scriptFilePath"));
}

void run_task(py::module_& m) {
  m.def("run_method", &run_task_upto_timestamp_in_simulator,
        R"(
    Function to invoke a function of the workflow script, given its name, function inputs and timestamp(upto which historical events should be considered).
    Returns the output generated by the function.

    Attributes :
    functionName : Function to be invoked in the script.
    inputData : Input data to the function.
    timestamp : Timestamp upto which historical events are to be considered.

    Return value :
    WorkflowUserReturn : Output of the invoked function.
  )",
        py::arg("functionName"), py::arg("inputData"), py::arg("timestamp") = nullptr);
}

void get_build_flags_simulator(py::module_& m) {
  m.def("get_build_flags", &get_build_flags_set,
        R"(
    Gets the set of build flags with which the simulator is built

    Return Value:
    set : Set of build flags
  )");
}
