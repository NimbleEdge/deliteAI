/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fstream>

#include "input_structs.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::ordered_json;

bool parse_common(const json& Input, CUserInput& user_Input) {
  try {
    std::string name = Input["name"];
    int name_length = name.length();
    user_Input.name = new char[name_length + 1];
    std::strcpy(user_Input.name, name.data());
    user_Input.length = Input["length"];
    user_Input.dataType = Input["type"];
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing common input : %s", e.what());
    return false;
  }
  return true;
}

CUserInput parse_to_model(const json& Input) {
  CUserInput inp;
  if (!parse_common(Input, inp)) {
    throw std::runtime_error("Error parsing user input.");
  }
  if (inp.dataType == DATATYPE::FLOAT) {
    float* values = new float[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::BOOLEAN) {
    bool* values = new bool[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::INT32) {
    int32_t* values = new int32_t[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::DOUBLE) {
    double* values = new double[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::INT64) {
    int64_t* values = new int64_t[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = jsonArray[i];
      }
      inp.data = (void*)values;
    } catch (const std::exception& e) {
      fprintf(stderr, "Error parsing to model input : %s", e.what());
      throw std::runtime_error("Error parsing user input.");
    }
    return inp;
  } else if (inp.dataType == DATATYPE::STRING) {
    char** values = new char*[inp.length];
    try {
      json jsonArray = Input["Data"];
      for (int i = 0; i < inp.length; i++) {
        values[i] = new char[jsonArray[i].size() + 1];
        std::strcpy(values[i], jsonArray[i].get<std::string>().c_str());
        values[i][jsonArray[i].size() + 1] = 0;
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

CUserInput parse_to_processor(const json& Input) {
  CUserInput inp;
  if (!parse_common(Input, inp)) {
    throw std::runtime_error("Error parsing user input.");
  }
  try {
    nlohmann::json* j = new nlohmann::json();
    *j = nlohmann::json::parse(Input["Data"].dump());
    inp.data = (void*)j;
    inp.dataType = DATATYPE::JSON;
    if (j->is_array()) {
      inp.length = j->size();
    } else {
      inp.length = 0;
    }
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing to json input : %s", e.what());
    throw std::runtime_error("Error parsing user input.");
  }
  return inp;
}

CUserInput parse_json(const json& Input) {
  int condition;
  try {
    condition = Input["type"];
  } catch (const std::exception& e) {
    fprintf(stderr, "Error parsing Input json : %s", e.what());
    return CUserInput();
  }
  if (condition != DATATYPE::JSON && condition != DATATYPE::JSON_ARRAY) {
    return parse_to_model(Input);
  } else {
    return parse_to_processor(Input);
  }
}

InputData::InputData() {
  this->inputs = {};
  this->totalInputs = 0;
}

InputData::InputData(const std::string& inputData) {
  std::fstream inputDataFile(inputData);
  if (inputDataFile) {
    getInputFromFile(inputData);
  } else {
    getInputFromBuffer(inputData);
  }
  return;
}

void InputData::getInputFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file) {
    throw std::runtime_error("User input file could not be opened");
  }
  std::string jsonString;
  json jsonData;
  try {
    jsonString =
        std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  } catch (const std::exception& e) {
    fprintf(stderr, "File=%s, could not be read : %s", filename.c_str(), e.what());
    file.close();
    return;
  }
  file.close();
  try {
    jsonData = json::parse(jsonString);
  } catch (const json::parse_error& e) {
    fprintf(stderr, "Json string=%s, could not be parsed : %s", jsonString.c_str(), e.what());
    return;
  }

  size_t numUserInputs = jsonData.size();
  this->totalInputs = numUserInputs;
  for (int i = 0; i < numUserInputs; i++) {
    std::shared_ptr<UserInput> userData = std::make_shared<UserInput>();
    (*userData).inp = parse_json(jsonData[i]);
    this->inputs.push_back(std::move(userData));  // changing copying to moving
  }
}

void InputData::getInputFromBuffer(const std::string& jsonString) {
  json jsonData;
  try {
    jsonData = json::parse(jsonString);
  } catch (const json::parse_error& e) {
    fprintf(stderr, "Json string=%s, could not be parsed : %s", jsonString.c_str(), e.what());
    return;
  }

  size_t numUserInputs = jsonData.size();
  this->totalInputs = numUserInputs;
  for (int i = 0; i < numUserInputs; i++) {
    std::shared_ptr<UserInput> userData = std::make_shared<UserInput>();
    (*userData).inp = parse_json(jsonData[i]);
    this->inputs.push_back(std::move(userData));  // changing copying to moving
  }
}
