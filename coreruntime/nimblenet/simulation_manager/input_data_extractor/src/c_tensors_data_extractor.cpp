/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_input_structs.hpp"

#include <fstream>
#include <memory>
#include <stdexcept>

#include "nlohmann/json.hpp"

#include "client.h"
#include "custom_func_data_variable.hpp"
#include "data_variable.hpp"
#include "map_data_variable.hpp"

using json = nlohmann::json;

// Setting name, dataType, shape and shapeLength of CTensor here
bool parse_CTensor_common(const json& Input, CTensor& user_Input) {
  try {
    std::string name = Input["name"];
    int name_length = name.length();
    user_Input.name = new char[name_length + 1];
    std::strcpy(user_Input.name, name.data());
    user_Input.dataType = Input["type"];
    int64_t* shape = new int64_t[Input["shape"].size()];
    for (int i = 0; i < Input["shape"].size(); i++) {
      shape[i] = Input["shape"][i].get<int64_t>();
    }
    user_Input.shape = shape;
    user_Input.shapeLength = Input["shape"].size();
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing common input : %s", e.what());
    return false;
  }
  return true;
}

// Sets data to CTensor
CTensor parse_CTensor_to_model(const json& Input) {
  CTensor inp;
  if (!parse_CTensor_common(Input, inp)) {
    throw std::runtime_error("Error parsing user input.");
  }
  // Single Variable
  if (Input["shape"].size() == 0) {
    if (inp.dataType == DATATYPE::FLOAT) {
      float* val = new float(Input["Data"]);
      inp.data = (void*)val;
    } else if (inp.dataType == DATATYPE::BOOLEAN) {
      const auto& boolStr = nlohmann::to_string(Input["Data"]);
      bool* val;
      if (boolStr == "false") {
        val = new bool(false);
      } else if (boolStr == "true") {
        val = new bool(true);
      } else {
        throw std::runtime_error("Expected true/false for boolean");
      }
      inp.data = (void*)val;
    } else if (inp.dataType == DATATYPE::INT32) {
      int32_t* val = new int32_t(Input["Data"]);
      inp.data = (void*)val;
    } else if (inp.dataType == DATATYPE::INT64) {
      int64_t* val = new int64_t(Input["Data"]);
      inp.data = (void*)val;
    } else if (inp.dataType == DATATYPE::DOUBLE) {
      double* val = new double(Input["Data"]);
      inp.data = (void*)val;
    } else if (inp.dataType == DATATYPE::STRING) {
      char** input = new char*[1];
      char* c = new char[Input["Data"].get<std::string>().size() + 1];
      c[Input["Data"].get<std::string>().size()] = 0;
      std::string s = Input["Data"].get<std::string>();
      std::strcpy(c, s.c_str());
      input[0] = c;
      inp.data = (void*)input;
    } else {
      throw std::runtime_error("Invalid data type while parsing CTensor data to model");
    }
    return inp;
  }
  int64_t total_length = 1;
  for (int i = 0; i < Input["shape"].size(); i++) {
    total_length *= Input["shape"][i].get<int64_t>();
  }

  // Empty tensor
  if (total_length == 0) {
    inp.data = nullptr;
    return inp;
  }

  if (inp.dataType == DATATYPE::FLOAT) {
    float* values = new float[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::BOOLEAN) {
    bool* values = new bool[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::INT32) {
    int32_t* values = new int32_t[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::DOUBLE) {
    double* values = new double[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::INT64) {
    int64_t* values = new int64_t[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::STRING) {
    char** values = new char*[total_length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < total_length; i++) {
        values[i] = new char[jsonArray[i].get<std::string>().size() + 1];
        std::string a = jsonArray[i].get<std::string>();
        std::strcpy(values[i], a.c_str());
        values[i][jsonArray[i].get<std::string>().size()] = 0;
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  }
  return inp;
}

CTensor parse_CTensor_to_json_input(const json& input) {
  CTensor inp;
  if (!parse_CTensor_common(input, inp)) {
    throw std::runtime_error("Error parsing user input.");
  }
  try {
    int inputShapeSize = input["shape"].size();
    // Single json object
    if (inputShapeSize == 0) {
      json j = input["Data"];
      OpReturnType* map = new OpReturnType(DataVariable::get_map_from_json_object(std::move(j)));
      inp.dataType = DATATYPE::JSON;
      inp.data = (void*)map;
      inp.shapeLength = 0;
      inp.shape = nullptr;
      return inp;
    }

    json j = input["Data"];
    OpReturnType* list = new OpReturnType(DataVariable::get_list_from_json_array(std::move(j)));
    inp.data = (void*)list;
    inp.dataType = DATATYPE::JSON_ARRAY;
    inp.shapeLength = 1;
    inp.shape = new int64_t[1];
    inp.shape[0] = j.size();
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing to json input : %s", e.what());
    throw std::runtime_error("Error parsing user input.");
  }
  return inp;
}

CTensor get_CTensor(const json& input) {
  int condition;
  try {
    condition = input["type"];
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing input json : %s", e.what());
    return CTensor();
  }
  if (condition == DATATYPE::JSON || condition == DATATYPE::JSON_ARRAY) {
    return parse_CTensor_to_json_input(input);
  } else {
    return parse_CTensor_to_model(input);
  }
}

CTensors get_CTensors_from_json(const char* j) {
  std::string jsonString = j;
  json jsonData;

  try {
    jsonData = json::parse(jsonString);
  } catch (const json::parse_error& e) {
    fprintf(stderr, "Json String = %scould not be parsed %s", jsonString.c_str(), e.what());
  }

  CTensors createdCTensors;
  int nTensors = jsonData.size();
  createdCTensors.numTensors = nTensors;
  CTensor* cTensors = new CTensor[nTensors];

  for (int i = 0; i < nTensors; i++) {
    CTensor cTensor = get_CTensor(jsonData[i]);
    cTensors[i] = cTensor;
  }
  createdCTensors.tensors = cTensors;
  return createdCTensors;
}

CTensors get_CTensors(const char* fileName) {
  std::fstream inputFile((std::string)fileName);
  if (!inputFile) {
    throw std::runtime_error("File for CTensors input found.");
  }
  std::string jsonString;
  try {
    jsonString =
        std::string(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
  } catch (const std::exception& e) {
    fprintf(stderr, "File=%s could not be parsed", fileName);
  }

  inputFile.close();
  return get_CTensors_from_json(jsonString.c_str());
}

void deallocate_CTensors(CTensors cTensors) {
  for (int i = 0; i < cTensors.numTensors; i++) {
    delete[] cTensors.tensors[i].name;
    if (cTensors.tensors[i].shapeLength == 0) {
      if (cTensors.tensors[i].dataType == DATATYPE::FLOAT) {
        delete (float*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::INT32) {
        delete (int32_t*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::INT64) {
        delete (long long*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::DOUBLE) {
        delete (double*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::JSON) {
        delete (OpReturnType*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::BOOLEAN) {
        delete (bool*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::STRING) {
        delete[] (char**)cTensors.tensors[i].data;
      }
    } else {
      if (cTensors.tensors[i].dataType == DATATYPE::FLOAT) {
        delete[] (float*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::INT32) {
        delete[] (int32_t*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::INT64) {
        delete[] (long long*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::DOUBLE) {
        delete[] (double*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::JSON_ARRAY) {
        delete (OpReturnType*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::BOOLEAN) {
        delete[] (bool*)cTensors.tensors[i].data;
      } else if (cTensors.tensors[i].dataType == DATATYPE::STRING) {
        delete[] (char**)cTensors.tensors[i].data;
      }
    }

    delete[] cTensors.tensors[i].shape;
  }

  delete[] cTensors.tensors;
}

#ifdef SIMULATION_MODE
void* TaskInputData::get_list_from_json_object_for_simulator(nlohmann::json&& j) {
  return (void*)(new OpReturnType(DataVariable::get_list_from_json_array(std::move(j))));
}

void* TaskInputData::get_map_from_json_object_for_simulator(nlohmann::json&& j) {
  return (void*)(new OpReturnType(DataVariable::get_map_from_json_object(std::move(j))));
}

nlohmann::json TaskInputData::get_json_from_OpReturnType(void* data) {
  return (static_cast<OpReturnType*>(data))->get()->to_json();
}

namespace {
std::shared_ptr<MapDataVariable> create_foreign_function_arg_map(
    const std::vector<OpReturnType>& args) {
  if (args.size() != 1) {
    THROW("calling foreign function: num args: expected = 1, actual = %zu", args.size());
  }

  const auto& arg = args[0];
  if (arg->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("calling foreign function: arg container type: expected = \"Map\", actual = \"%s\"",
          arg->get_containerType_string());
  }
  return std::dynamic_pointer_cast<MapDataVariable>(arg);
}
}  // anonymous namespace

void* TaskInputData::create_function_data_variable(void* context,
                                                   FrontendFunctionPtr frontEndFunctionPtr) {
  auto sContext =
      std::shared_ptr<void>(context, [](void* ctx) { free_frontend_function_context(ctx); });

  return (void*)new OpReturnType(new CustomFuncDataVariable(
      [context = sContext, frontEndFunctionPtr](const std::vector<OpReturnType>& arguments,
                                                CallStack& stack) -> OpReturnType {
        std::shared_ptr<MapDataVariable> fnInput = create_foreign_function_arg_map(arguments);
        CTensors fnInTensors;
        fnInput->convert_to_cTensors(&fnInTensors);

        CTensors fnOutTensors;
        auto status = frontEndFunctionPtr(context.get(), fnInTensors, &fnOutTensors);
        delete[] fnInTensors.tensors;

        if (status != nullptr) {
          auto fmtString = ne::fmt("Callback function failed with status code %d error %s",
                                   status->code, status->message);
          deallocate_nimblenet_status(status);
          throw std::runtime_error(fmtString.str);
        }

        auto fnOutput = std::make_shared<MapDataVariable>(fnOutTensors);
        deallocate_frontend_tensors(fnOutTensors);
        return fnOutput;
      }));
}

void TaskInputData::deallocate_OpReturnType(void* data) { delete (OpReturnType*)data; }

#endif
