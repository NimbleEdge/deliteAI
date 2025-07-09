/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdlib>
#include <fstream>

#include "client.h"
#include "input_structs.hpp"
#include "nimble_net_util.hpp"
#include "nimblejson.hpp"
#include "nimblenet.h"
#include "nlohmann/json.hpp"
#include "task_input_structs.hpp"

namespace py = pybind11;

template <typename T>
CUserInput assign_data(py::array_t<T>& moved_arr) {
  CUserInput inp;
  inp.length = moved_arr.size();
  py::dtype dt = moved_arr.dtype();
  std::string dtypeName = dt.attr("name").cast<std::string>();
  if (dtypeName == "float32") {
    inp.dataType = DATATYPE::FLOAT;
    float* values = new float[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(float));
    inp.data = (void*)values;
  } else if (dtypeName == "bool") {
    inp.dataType = DATATYPE::BOOLEAN;
    bool* values = new bool[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(bool));
    inp.data = (void*)values;
  } else if (dtypeName == "int32") {
    inp.dataType = DATATYPE::INT32;
    int32_t* values = new int32_t[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(int32_t));
    inp.data = (void*)values;
  } else if (dtypeName == "int64") {
    inp.dataType = DATATYPE::INT64;
    int64_t* values = new int64_t[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(int64_t));
    inp.data = (void*)values;
  } else if (dtypeName == "float64") {
    inp.dataType = DATATYPE::DOUBLE;
    double* values = new double[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(double));
    inp.data = (void*)values;
  } else if (dtypeName == "double") {
    inp.dataType = DATATYPE::DOUBLE;
    double* values = new double[inp.length];
    std::memcpy(values, moved_arr.data(), inp.length * sizeof(double));
    inp.data = (void*)values;
  } else {
    throw std::runtime_error("Invalid datatype:" + dtypeName + " for input provided.");
  }
  return inp;
}

CUserInput assign_list(py::list& list) {
  CUserInput inp;
  inp.dataType = DATATYPE::JSON;
  inp.length = list.size();
  size_t dataSize = list.size();
  auto json_allocator = nimblejson::create_json_allocator();
  auto jsonArray = nimblejson::create_json_array(json_allocator);
  for (size_t i = 0; i < dataSize; ++i) {
    auto json = nimblejson::create_json_object(json_allocator);
    py::dict dict = py::cast<py::dict>(list[i]);
    int keys_Length = dict.size();
    for (const auto& item_within : dict) {
      std::string key = py::cast<std::string>(item_within.first);
      std::string value = py::cast<std::string>(item_within.second);
      nimblejson::add_string_value(key.c_str(), value.c_str(), json);
    }
    nimblejson::move_json_object_or_array_to_array(jsonArray, json);
  }
  inp.data = jsonArray;
  return inp;
}

char* set_input_name(const std::string& value) {
  char* name = new char[value.size() + 1];
  std::copy(value.begin(), value.end(), name);
  name[value.size()] = '\0';
  return name;
}

// responsible for converting the void** output into a vector(in python a list) of numpy arrays
std::map<std::string, py::array> get_out(const InferenceReturn& ret) {
  void** outputs = ret.outputs;
  int** outputShapes = ret.outputShapes;
  int* outputShapeLengths = ret.outputShapeLengths;
  char** outputNames = ret.outputNames;
  int* outputTypes = ret.outputTypes;
  int numOutputs = ret.numOutputs;

  std::map<std::string, py::array> py_outputs;
  for (int i = 0; i < numOutputs; i++) {
    int* shape = outputShapes[i];
    int shape_length = outputShapeLengths[i];
    int type = outputTypes[i];
    std::string name = outputNames[i];
    void* data = outputs[i];

    std::vector<ssize_t> shape_vector;
    for (int j = 0; j < shape_length; j++) {
      shape_vector.push_back(shape[j]);
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
      default:
        throw std::runtime_error("Unsupported data type");
    }

    py::array py_array(dtype, shape_vector, data);
    py_outputs[name] = py_array;
  }
  return py_outputs;
}

std::map<std::string, py::array> func_out(UserReturn& final) { return get_out(final.output); }

std::map<std::string, py::array> func_inp(UserReturn& final) { return get_out(final.input); }

void set_inputs(InputData& data, const std::vector<std::shared_ptr<UserInput>>& value) {
  data.totalInputs = value.size();
  data.inputs = value;
}

template <typename T>
std::shared_ptr<UserInput> constructor(const std::string& name, py::array_t<T>& arr) {
  std::shared_ptr<UserInput> data_ptr = std::make_shared<UserInput>();
  // maintain the order of assigning data and name
  (*data_ptr).inp = assign_data(arr);
  (*data_ptr).inp.name = set_input_name(name);
  (*data_ptr).dataType = (*data_ptr).inp.dataType;
  (*data_ptr).length = (*data_ptr).inp.length;
  return data_ptr;
}

std::shared_ptr<UserInput> construct_processor_data(const std::string& name, py::list& list) {
  std::shared_ptr<UserInput> data_ptr = std::make_shared<UserInput>();
  (*data_ptr).inp = assign_list(list);
  (*data_ptr).inp.name = set_input_name(name);
  (*data_ptr).dataType = (*data_ptr).inp.dataType;
  (*data_ptr).length = (*data_ptr).inp.length;
  return data_ptr;
}

std::shared_ptr<UserInput> constructor(const std::string& name, py::list& list) {
  std::shared_ptr<UserInput> data_ptr = std::make_shared<UserInput>();
  if (list.size() > 0) {
    bool isString = py::isinstance<py::str>(list[0]);
    bool isJsonObject = py::isinstance<py::dict>(list[0]);
    if (!isString && !isJsonObject) {
      throw std::runtime_error(
          "Input list should not contain elements other than string and json objects.");
    }
    for (size_t i = 1; i < list.size(); i++) {
      if ((isString && !py::isinstance<py::str>(list[i])) ||
          (isJsonObject && !py::isinstance<py::dict>(list[i]))) {
        throw std::runtime_error("Input list contains elements of multiple data types.");
      }
    }

    if (py::isinstance<py::str>(list[0])) {
      (*data_ptr).dataType = 8;
      (*data_ptr).length = list.size();
      std::vector<std::string> stringVector = py::cast<std::vector<std::string>>(list);
      (*data_ptr).inp.data = static_cast<void*>(&stringVector);
      (*data_ptr).inp.name = set_input_name(name);
      (*data_ptr).inp.length = list.size();
      (*data_ptr).inp.dataType = 8;
    } else if (py::isinstance<py::dict>(list[0])) {
      return construct_processor_data(name, list);
    }
  }
  return data_ptr;
}

bool deallocFrontendTensors(CTensors cTensors) {
  for (int i = 0; i < cTensors.numTensors; i++) {
    auto& tensor = cTensors.tensors[i];
    delete[] tensor.name;
    delete[] tensor.shape;
    switch (tensor.dataType) {
      case DATATYPE::JSON:
      case DATATYPE::JSON_ARRAY:
      case DATATYPE::FUNCTION:
        TaskInputData::deallocate_OpReturnType(tensor.data);
        break;
      case DATATYPE::STRING: {
        char** data = static_cast<char**>(tensor.data);
        free(*data);
        free(data);
        break;
      }
      default:
        free(tensor.data);
    }
  }
  delete[] cTensors.tensors;
  return true;
}

bool freeFrontendContext(void* context) {
  delete (py::object*)context;
  return true;
}

nlohmann::json convert_py_list_to_json(py::list& list) {
  nlohmann::json jsonArray = nlohmann::json(nlohmann::json::array());
  for (const auto& item : list) {
    py::module_ json_module = py::module_::import("json");
    py::str json_str = json_module.attr("dumps")(item);
    std::string cpp_json_str = py::cast<std::string>(json_str);
    jsonArray.push_back(nlohmann::json::parse(cpp_json_str));
  }
  return jsonArray;
}

void load_simulator_modules(py::list& moduleConfig) {
  auto jsonConfig = convert_py_list_to_json(moduleConfig);
  std::string jsonString = jsonConfig.dump();
  auto status = load_modules(jsonString.c_str(), "./NimbleSDK/");
  if (status != nullptr) {
    throw std::runtime_error("Error while loading modules");
  }
}

int initialize_simulator_nimblenet(const char* configInput, py::list moduleConfig) {
  // Resetting before initialise so as to use same instance of nimblenet across multiple python
  // sessions.
  reset();

  // setting global cleanup functions
  globalDeallocate = deallocFrontendTensors;
  globalFrontendContextFree = freeFrontendContext;

  // If no config provided prepare a default config
  if (configInput == nullptr) {
    configInput = "{}";
  }
  nlohmann::json configJson = nlohmann::json::parse(configInput);
  if (!configJson.value("online", false)) {
    if (py::len(moduleConfig) == 0) {
      throw std::runtime_error(
          "moduleInfo has to be present if online flag is either false or not present in config");
    }
    load_simulator_modules(moduleConfig);
  }
  auto t = initialize_nimblenet(configInput, "./NimbleSDK/");
  if (t == nullptr) return 1;
  throw std::runtime_error(std::string(t->message) + "\nInit failed.");
}

bool add_user_events(const char* user_events, const char* table_name) {
  std::ifstream userEventsFile(user_events);
  if (userEventsFile) {
    return add_events_from_file(user_events, table_name);
  }
  return add_events_from_buffer(user_events, table_name);
}

bool add_user_event_dict(const py::dict& input_dict, const char* table_name) {
  nlohmann::json jsonObj;
  for (auto item : input_dict) {
    std::string key = item.first.cast<std::string>();

    // Determine the type of the value
    if (PyLong_Check(item.second.ptr())) {
      int value = item.second.cast<int>();
      jsonObj[key] = value;
    } else if (PyUnicode_Check(item.second.ptr())) {
      std::string value = item.second.cast<std::string>();
      jsonObj[key] = value;
    } else if (PyFloat_Check(item.second.ptr())) {
      double value = item.second.cast<double>();
      jsonObj[key] = value;
    } else if (PyBool_Check(item.second.ptr())) {
      bool value = item.second.cast<bool>();
      jsonObj[key] = value;
    } else {
      throw std::runtime_error("Unsupported data type inside a dict.");
    }
  }
  std::string jsonObjDump = jsonObj.dump();
  return add_events_from_buffer(jsonObjDump.c_str(), table_name);
}

bool add_user_event_list(const py::list& inputs, const char* table_name) {
  bool flag = true;
  // Iterate over the items in the Python list
  for (auto item : inputs) {
    py::dict dict_item = py::cast<py::dict>(item);
    if (!add_user_event_dict(dict_item, table_name)) {
      flag = false;
    }
  }
  return flag;
}

bool is_simulator_ready() {
  NimbleNetStatus* status = is_ready();
  if (status == nullptr) {
    return true;
  }
  deallocate_nimblenet_status(status);
  return false;
}

void cleanup() { deallocate_nimblenet(); }

void load_task(py::module_&);

void run_task(py::module_&);

void get_build_flags_simulator(py::module_& m);

PYBIND11_MODULE(simulator, m) {
  m.doc() = R"(
      Simulator module which defines the following data type types and functions exposed for simulation.
      Following are the functions exposed:
        - initialize
        - load_model
        - load_workflow_script
        - add_user_events
        - get_inference
        - run_method
        - UserInput
        - InputData
        - UserReturn
        )";

  m.def("initialize", &initialize_simulator_nimblenet, R"(
    Initializes the simulator.

    Attributes :
    config (string) : String buffer with the configurations. It is optional field, if not provided nimblenet will initialize with default configs i.e. in offline mode with isTimeSimulated as true.
    moduleConfig(list): Module information to be loaded from disk in case of offline initialize
    
    Return value :
    Int : If success then return 1.

    Exceptions :
    RunTimeError : If config file could not opened or found.
    RunTimeError : If config could not be parsed.
  )",
        py::arg("config") = nullptr, py::arg("moduleConfig") = py::list());

  m.def("load_model", &load_model_from_file, R"(
    Function to load model.

    Attributes :
    modelFilePath : file path where model is stored
    inferenceConfigFilePath : file path where model config is stored (optional, only required if workflow script is not used)
    modelId : model id

    Return value :
    Boolean : True if the model was successfully loaded

    Exceptions :
    RunTimeError : model or config could not be parsed or loaded
  )",
        py::arg("modelFilePath"), py::arg("inferenceConfigFilePath") = nullptr, py::arg("modelId"),
        py::arg("epConfigJsonChar") = nullptr);

  m.def("add_user_events", &add_user_events, R"(
    Function to load user events.

    Attributes :
    userEvents : file path where user events are stored or a string buffer containing the user events.
    tableName : table name where these user events will be stored.

    Return value :
    Boolean : True if the events were successfully loaded.

    Exceptions :
    RunTimeError : File could not be read or events could not be successfully parsed.
  )",
        py::arg("userEvents"), py::arg("tableName"));

  m.def("add_user_events", &add_user_event_dict, R"(
    Function to load a user events from a dictionary.

    Attributes :
    userEvents : string buffer where user events are stored
    tableName : table name where these user events will be stored.

    Return value :
    Boolean : True if the events were successfully loaded.

    Exceptions :
    RunTimeError : File could not be read or events could not be successfully parsed.
  )",
        py::arg("userEvents"), py::arg("tableName"));

  m.def("add_user_events", &add_user_event_list, R"(
    Function to load user events from a list of dictionaries.

    Attributes :
    userEvents : string buffer where user events are stored
    tableName : table name where these user events will be stored.

    Return value :
    Boolean : True if the events were successfully loaded.

    Exceptions :
    RunTimeError : File could not be read or events could not be successfully parsed.
  )",
        py::arg("userEvents"), py::arg("tableName"));

  m.def("is_ready", &is_simulator_ready, R"(Tells whether nimbleSDK is ready or not.)");

  m.def("cleanup", &cleanup, "Perform cleanup of the resources.");

  /** Usage:
   *    from deliteai import simulator
   *    simulator.version.git_revision
   *    simulator.version.sdk_version
   */
  py::module_ v = m.def_submodule("version");
  v.attr("git_revision") = NIMBLE_GIT_REV;
  v.attr("sdk_version") = SDKVERSION;

  load_task(m);
  run_task(m);
  get_build_flags_simulator(m);
}
