/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests_util.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "native_interface.hpp"
#include "nimblejson.hpp"
#include "nimblenet.hpp"
#include "nimbletest.hpp"
#include "single_variable.hpp"
#include "task_input_structs.hpp"
#include "tensor_data_variable.hpp"
#ifdef GENAI
#include "miniz.h"
#endif  // GENAI

NimbleNetStatus* TestsUtil::initialize_and_is_ready(const std::string& validE2EConfigJson) {
  auto status = nimblenet::initialize_nimblenet(validE2EConfigJson, nativeinterface::HOMEDIR);
  EXPECT_EQ(status, nullptr);
  int tries = 0;
  while (tries < 5) {
    if (nimblenet::is_ready() == nullptr) {
      return nullptr;
    }
    Time::sleep_until(1);
    tries++;
  }
  return nimblenet::is_ready();
}

NimbleNetStatus* TestsUtil::initialize_and_is_ready_with_sleep(
    const std::string& validE2EConfigJson) {
  auto status = nimblenet::initialize_nimblenet(validE2EConfigJson, nativeinterface::HOMEDIR);
  EXPECT_EQ(status, nullptr);
  Time::sleep_until(6);
  return nimblenet::is_ready();
}

bool TestsUtil::delete_file(const std::string& filePath) {
  try {
    if (std::filesystem::remove(filePath)) {
      return true;
    }
  } catch (std::filesystem::filesystem_error& e) {
    THROW("Error while deleting file: %s with error: %s", filePath.c_str(), e.what());
  } catch (...) {
    THROW("Unknown error while deleting file: %s", filePath.c_str());
  }
  THROW("Trying to delete a file: %s which does not exist.", filePath.c_str());
}

bool TestsUtil::copy_file(const std::string& source, const std::string& destination) {
  try {
    std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing);
    return true;

  } catch (std::filesystem::filesystem_error& e) {
    THROW("Error while copying file from: %s to: %s with error: %s", source.c_str(),
          destination.c_str(), e.what());
  } catch (...) {
    THROW("Unknown error while copying file from: %s to: %s", source.c_str(), destination.c_str());
  }
}

std::set<std::string> TestsUtil::get_file_names_from_folder(const std::string& folderName) {
  std::string folderPath = nativeinterface::get_full_file_path_common(folderName);
  std::set<std::string> listOfFiles;

  if (!std::filesystem::exists(folderPath)) return listOfFiles;
  if (!std::filesystem::is_directory(folderPath)) return listOfFiles;

  for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
    if (std::filesystem::is_regular_file(entry.status())) {
      listOfFiles.insert(entry.path().filename().string());
    }
  }
  return listOfFiles;
}

bool TestsUtil::corrupt_file(const std::string& filePathStr, int numBytes, int offset) {
  std::filesystem::path filePath(filePathStr);

  if (!std::filesystem::exists(filePath)) {
    THROW("File does not exist: %s", filePath.c_str());
  }

  std::vector<unsigned char> fixedBytes(numBytes, 2);

  std::ofstream file(filePath, std::ios::binary | std::ios::in | std::ios::out);
  if (!file.is_open()) {
    THROW("Failed to open file: %S", filePath.c_str());
  }

  file.seekp(offset);
  if (!file) {
    THROW("Failed to seek to offset %d", offset);
  }

  file.write((char*)fixedBytes.data(), fixedBytes.size());
  return true;
}

bool TestsUtil::create_archive(const std::string& folderName) {
#ifdef GENAI
  std::string folderPath = nativeinterface::get_full_file_path_common(folderName);
  if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath)) {
    THROW("Invalid path to the folder %s", folderPath.c_str());
  }

  std::string archivePath = folderPath + ".zip";
  // Initialize zip archive
  mz_zip_archive zip = {};
  if (!mz_zip_writer_init_file(&zip, archivePath.c_str(), 0)) {
    THROW("Failed to initialize zip archive");
  }

  auto basePath = std::filesystem::path(folderPath);
  size_t basePathLen = basePath.string().length();

  // Recursively add files to zip
  for (const auto& entry : std::filesystem::recursive_directory_iterator(basePath)) {
    if (std::filesystem::is_regular_file(entry.path())) {
      std::string fullPath = entry.path().string();
      std::string relativePath = fullPath.substr(basePathLen + 1);  // remove base dir

      if (!mz_zip_writer_add_file(&zip, relativePath.c_str(), fullPath.c_str(), nullptr, 0,
                                  MZ_DEFAULT_COMPRESSION)) {
        mz_zip_writer_end(&zip);
        THROW("Failed to add file to zip: %s", fullPath.c_str());
      }
    }
  }

  // Finalize and close archive
  if (!mz_zip_writer_finalize_archive(&zip)) {
    mz_zip_writer_end(&zip);
    THROW("Failed to finalize zip archive");
  }

  mz_zip_writer_end(&zip);

  return true;
#endif  // GENAI
  THROW("Creating archive is only supported in GENAI mode.");
}

bool TestsUtil::set_expectations(const std::string& path, int response_delay,
                                 int expected_status_code, const nlohmann::json& expected_headers,
                                 const nlohmann::json& expected_response_body) {
  nlohmann::json requestBody;
  requestBody["path"] = path.substr(_mockServerHost.size());  // Remove http://localhost:8080
  requestBody["response_delay"] = response_delay;
  requestBody["status_code"] = expected_status_code;
  requestBody["body"] = expected_response_body;
  requestBody["headers"] = expected_headers;
  auto bodyString = requestBody.dump();
  std::string expMethod = "POST";
  std::string header = "[{\"Content-Type\": \"application/json\"}]";
  auto response = send_request(bodyString.c_str(), header.c_str(), _setExpectationURL.c_str(),
                               expMethod.c_str(), -1);
  if (response.statusCode != 201) {
    auto responseString = std::string(response.body, response.bodyLength);
    THROW("Failed to set expectations with status code %d, message, %s", response.statusCode,
          responseString.c_str());
  }
  return true;
}

bool TestsUtil::reset_expectations() {
  std::string bodyString = nlohmann::json::object().dump();
  std::string headersString = nlohmann::json::array().dump();
  std::string expMethod = "POST";
  auto response = send_request(bodyString.c_str(), headersString.c_str(),
                               _resetExpectationURL.c_str(), expMethod.c_str(), -1);
  if (response.statusCode != 200) {
    auto responseString = std::string(response.body, response.bodyLength);
    THROW("Failed to reset expectations with status code %d, message, %s", response.statusCode,
          responseString.c_str());
  }
  return true;
}

void TestsUtil::assert_deployment(const std::string& functionName, int expectedValue) {
  auto map = std::make_shared<MapDataVariable>();
  std::vector<int64_t> tensorShape = {1};
  auto tensor = std::make_shared<TensorVariable>(tensorShape, INT64);
  tensor->set_subscript(std::make_shared<SingleVariable<int32_t>>(0),
                        std::make_shared<SingleVariable<int64_t>>(5));
  map->set_value_in_map("num", tensor);
  auto outputMap = std::make_shared<MapDataVariable>();
  ASSERT_EQ(nimblenet::run_method(functionName, map, outputMap), nullptr);
  ASSERT_EQ(outputMap->get_string_subscript("output")->get_int_subscript(0)->get_int64(),
            expectedValue);
}

void TestsUtil::assert_deployment_with_no_model() {
  auto map = std::make_shared<MapDataVariable>();
  auto outputMap = std::make_shared<MapDataVariable>();
  ASSERT_EQ(nimblenet::run_method("no_model", map, outputMap), nullptr);
  ASSERT_EQ(outputMap->get_string_subscript("output")->get_string(), "no_model_executed");
}

void TestsUtil::assert_deployment_with_modules() {
  auto map = std::make_shared<MapDataVariable>();
  auto outputMap = std::make_shared<MapDataVariable>();
  ASSERT_EQ(nimblenet::run_method("run", map, outputMap), nullptr);
  ASSERT_EQ(outputMap->get_string_subscript("moduleA_A")->get_int64(), 10);
  ASSERT_EQ(outputMap->get_string_subscript("main_A")->get_int64(), 20);
}

NimbleNetStatus* TestsUtil::add_event(const std::string& eventType,
                                      CUserEventsData* cUserEventsData,
                                      int waitTimeForJobScheduler) {
  auto status = nimblenet::add_event("{\"id\": 1, \"floatData\": 1.12, \"stringData\": \"abcd\"}",
                                     eventType, cUserEventsData);
  Time::sleep_until(waitTimeForJobScheduler);
  return status;
}

HistoricalAPIs TestsUtil::get_historical_api_calls() {
  std::string bodyString = nlohmann::json::object().dump();
  std::string headersString = nlohmann::json::array().dump();
  std::string apiMethod = "GET";
  auto response = send_request(bodyString.c_str(), headersString.c_str(),
                               _historicalAPIsURL.c_str(), apiMethod.c_str(), -1);
  if (response.statusCode != 200) {
    auto responseString = std::string(response.body, response.bodyLength);
    THROW(
        "Failed to get list of historical API calls in current session from mock server with "
        "status code %d, message, %s",
        response.statusCode, responseString.c_str());
  }
  std::string responseString(response.body, response.bodyLength);
  HistoricalAPIs r = jsonparser::get<HistoricalAPIs>(responseString);
  return r;
}

void TestsUtil::assert_historical_api_calls(const std::string& testName, int index,
                                            bool dumpHistoricalData) {
  if (dumpHistoricalData) {
    dump_historical_api_calls(testName, index);
    return;
  }
  HistoricalAPIs actualAPICalls = get_historical_api_calls();

  std::pair<bool, std::string> expectedAPICallsFile =
      nativeinterface::read_potentially_compressed_file(
          get_historical_apis_file_path_from_test_name(testName), true);
  if (!expectedAPICallsFile.first) {
    THROW("%s", "Error while reading expected API calls file.");
  }
  nlohmann::json expectedAPICallsJson = nlohmann::json::parse(expectedAPICallsFile.second);
  HistoricalAPIs expectedAPICalls =
      jsonparser::get_from_json<HistoricalAPIs>(expectedAPICallsJson[index]);
  ASSERT_EQ(actualAPICalls, expectedAPICalls);
}

void TestsUtil::create_file_if_not_present(const std::string& filePath) {
  if (std::filesystem::exists(filePath)) {
    return;
  }
  std::ofstream file(filePath);
  if (!file) {
    THROW("Could not create file with path: %s", filePath.c_str());
  }
}

nlohmann::json TestsUtil::read_api_calls_history_file(const std::string& filePath) {
  std::ifstream file(filePath);

  if (!file) {
    THROW("Could not read e2e tests asset file with path: %s", filePath.c_str());
  }
  if (std::filesystem::is_empty(filePath)) {
    return nlohmann::json::array();
  }
  try {
    nlohmann::json jsonData;
    file >> jsonData;
    if (!jsonData.is_array()) {
      THROW("%s", "Json data present in file: %s is not an array.", filePath.c_str());
    }
    return jsonData;
  } catch (const nlohmann::json::parse_error& e) {
    THROW("%s", "Unable to parse data in file: %s as json.", filePath.c_str());
  }
}

void TestsUtil::write_json_data_to_file(const std::string& filePath, nlohmann::json&& data) {
  std::ofstream file(filePath);
  if (!file) {
    THROW("Could not write data to file: %s", filePath.c_str());
  }
  file << data.dump(4);
}

void TestsUtil::dump_historical_api_calls(const std::string& testName, int index) {
  HistoricalAPIs actualAPICalls = get_historical_api_calls();
  TestsUtil::create_file_if_not_present(get_historical_apis_file_path_from_test_name(testName));
  nlohmann::json data = TestsUtil::read_api_calls_history_file(
      get_historical_apis_file_path_from_test_name(testName));
  data[index] = actualAPICalls;
  TestsUtil::write_json_data_to_file(get_historical_apis_file_path_from_test_name(testName),
                                     std::move(data));
}

void TestsUtil::copy_asset(const std::string& path, const std::string& version,
                           const std::string& name, const AssetType assetType) {
  std::string fullFilePath;
  ASSERT_TRUE(ServerHelpers::get_full_file_path_from_assets(path, fullFilePath));
  std::shared_ptr<Asset> asset = std::make_shared<Asset>();
  asset->version = version;
  asset->name = name;
  asset->type = assetType;
  ASSERT_TRUE(TestsUtil::copy_file(fullFilePath,
                                   nativeinterface::HOMEDIR + asset->get_file_name_on_device()));
}

void TestsUtil::compare_json_arrays_util(void* jsonPointerOutput, void* expectedJsonPointer,
                                         void* json_allocator) {
  // This will give the next element of array
  while (true) {
    JsonOutput* nextElement = static_cast<JsonOutput*>(
        nimblejson::get_next_json_element(jsonPointerOutput, json_allocator));
    JsonOutput* expectedNextElement = static_cast<JsonOutput*>(
        nimblejson::get_next_json_element(expectedJsonPointer, json_allocator));
    ASSERT_EQ(nextElement->isEnd, expectedNextElement->isEnd);
    if (nextElement->isEnd) {
      return;
    }
    ASSERT_EQ(nextElement->dataType, expectedNextElement->dataType);
    switch (nextElement->dataType) {
      case DATATYPE::JSON:
        compare_json_objects_util(const_cast<void*>(nextElement->value.obj),
                                  const_cast<void*>(expectedNextElement->value.obj),
                                  json_allocator);
        break;
      case DATATYPE::JSON_ARRAY:
        compare_json_arrays_util(const_cast<void*>(nextElement->value.obj),
                                 const_cast<void*>(expectedNextElement->value.obj), json_allocator);
        break;
      case DATATYPE::DOUBLE:
        ASSERT_EQ(nextElement->value.d, expectedNextElement->value.d);
        break;
      case DATATYPE::FLOAT:
        ASSERT_EQ(nextElement->value.d, expectedNextElement->value.d);
        break;
      case DATATYPE::INT64:
        ASSERT_EQ(nextElement->value.i, expectedNextElement->value.i);
        break;
      case DATATYPE::INT32:
        ASSERT_EQ(nextElement->value.i, expectedNextElement->value.i);
        break;
      case DATATYPE::STRING:
        ASSERT_STREQ(nextElement->value.s, expectedNextElement->value.s);
        break;
      case DATATYPE::BOOLEAN:
        ASSERT_EQ(nextElement->value.b, expectedNextElement->value.b);
        break;
      case DATATYPE::NONE:
        ASSERT_EQ(nextElement->value.obj, expectedNextElement->value.obj);
        break;
      default:
        std::cout << "Something unknown found with dtype=%d in json array" << nextElement->dataType
                  << std::endl;
        break;
    }
  }
}

void TestsUtil::compare_json_objects_util(void* jsonPointerOutput, void* expectedJsonPointer,
                                          void* json_allocator) {
  // This will give the next element of object
  while (true) {
    JsonOutput* nextElement = static_cast<JsonOutput*>(
        nimblejson::get_next_json_element(jsonPointerOutput, json_allocator));
    JsonOutput* expectedNextElement = static_cast<JsonOutput*>(
        nimblejson::get_next_json_element(expectedJsonPointer, json_allocator));
    ASSERT_EQ(nextElement->isEnd, expectedNextElement->isEnd);
    if (nextElement->isEnd) {
      return;
    }
    ASSERT_EQ(nextElement->dataType, expectedNextElement->dataType);
    ASSERT_EQ(std::string(nextElement->key), std::string(expectedNextElement->key));
    switch (nextElement->dataType) {
      case DATATYPE::JSON:
        compare_json_objects_util(const_cast<void*>(nextElement->value.obj),
                                  const_cast<void*>(expectedNextElement->value.obj),
                                  json_allocator);
        break;
      case DATATYPE::JSON_ARRAY:
        compare_json_arrays_util(const_cast<void*>(nextElement->value.obj),
                                 const_cast<void*>(expectedNextElement->value.obj), json_allocator);
        break;
      case DATATYPE::DOUBLE:
        ASSERT_EQ(nextElement->value.d, expectedNextElement->value.d);
        break;
      case DATATYPE::FLOAT:
        ASSERT_EQ(nextElement->value.d, expectedNextElement->value.d);
        break;
      case DATATYPE::INT64:
        ASSERT_EQ(nextElement->value.i, expectedNextElement->value.i);
        break;
      case DATATYPE::INT32:
        ASSERT_EQ(nextElement->value.i, expectedNextElement->value.i);
        break;
      case DATATYPE::STRING:
        ASSERT_STREQ(nextElement->value.s, expectedNextElement->value.s);
        break;
      case DATATYPE::BOOLEAN:
        ASSERT_EQ(nextElement->value.b, expectedNextElement->value.b);
        break;
      case DATATYPE::NONE:
        ASSERT_EQ(nextElement->value.obj, expectedNextElement->value.obj);
        break;
      default:
        std::cout << "Something unknown found with dtype=%d in json object" << nextElement->dataType
                  << std::endl;
        break;
    }
  }
}

void TestsUtil::compare_json_objects(void* outputJson, void* expectedJsonOutput,
                                     void* json_allocator) {
  void* outputJsonPointer = nimblejson::create_json_iterator(outputJson, json_allocator);
  void* expectedJsonPointer = nimblejson::create_json_iterator(expectedJsonOutput, json_allocator);
  compare_json_objects_util(outputJsonPointer, expectedJsonPointer, json_allocator);
}

void TestsUtil::compare_json_arrays(void* outputJson, void* expectedJsonOutput,
                                    void* json_allocator) {
  void* outputJsonPointer = nimblejson::create_json_iterator(outputJson, json_allocator);
  void* expectedJsonPointer = nimblejson::create_json_iterator(expectedJsonOutput, json_allocator);
  compare_json_arrays_util(outputJsonPointer, expectedJsonPointer, json_allocator);
}

void TestsUtil::compare_tensors(CTensors t1, CTensors t2) {
  ASSERT_EQ(t1.numTensors, t2.numTensors);
  for (int i = 0; i < t1.numTensors; i++) {
    bool foundTensor = false;
    for (int j = 0; j < t2.numTensors; j++) {
      if (!strcmp(t1.tensors[i].name, t2.tensors[j].name)) {
        // found same output name
        printf("Comparing %s\n", t1.tensors[i].name);
        foundTensor = true;
        ASSERT_EQ(t1.tensors[i].dataType, t2.tensors[j].dataType);
        ASSERT_EQ(t1.tensors[i].shapeLength, t2.tensors[j].shapeLength);
        // Compare single elements
        if (t1.tensors[i].shapeLength == 0) {
          switch (t1.tensors[i].dataType) {
            case DATATYPE::FLOAT:
              EXPECT_NEAR(((float*)t1.tensors[i].data)[0], ((float*)t2.tensors[j].data)[0], 1e-4);
              break;
            case DATATYPE::DOUBLE:
              EXPECT_NEAR(((double*)t1.tensors[i].data)[0], ((double*)t2.tensors[j].data)[0], 1e-4);
              break;
            case DATATYPE::INT32:
              ASSERT_EQ(((int32_t*)t1.tensors[i].data)[0], ((int32_t*)t2.tensors[j].data)[0]);
              break;
            case DATATYPE::INT64:
              ASSERT_EQ(((int64_t*)t1.tensors[i].data)[0], ((int64_t*)t2.tensors[j].data)[0]);
              break;
            case DATATYPE::STRING:
              ASSERT_EQ((std::string)((((char**)t1.tensors[i].data)[0])),
                        (std::string)((((char**)t2.tensors[j].data)[0])));
              break;
            case DATATYPE::JSON: {
              void* json_allocator = nimblejson::create_json_allocator();
              compare_json_objects(t1.tensors[i].data, t2.tensors[j].data, json_allocator);
              nimblejson::deallocate_json_allocator(json_allocator);
              break;
            }
            default:
              throw std::runtime_error("dataType=" + std::to_string(t1.tensors[i].dataType) +
                                       "not supported for single variable in gtest");
          }
          break;
        } else {
          int length = 1;
          for (int k = 0; k < t1.tensors[i].shapeLength; k++) {
            ASSERT_EQ(t1.tensors[i].shape[k], t2.tensors[j].shape[k]);
            length *= t1.tensors[i].shape[k];
          }
          if (t1.tensors[i].dataType == DATATYPE::JSON_ARRAY) {
            void* json_allocator = nimblejson::create_json_allocator();
            compare_json_arrays(t1.tensors[i].data, t2.tensors[j].data, json_allocator);
            nimblejson::deallocate_json_allocator(json_allocator);
            continue;
          }
          for (int k = 0; k < length; k++) {
            switch (t1.tensors[i].dataType) {
              case DATATYPE::FLOAT:
                EXPECT_NEAR(((float*)t1.tensors[i].data)[k], ((float*)t2.tensors[j].data)[k], 1e-4);
                break;
              case DATATYPE::DOUBLE:
                EXPECT_NEAR(((double*)t1.tensors[i].data)[k], ((double*)t2.tensors[j].data)[k],
                            1e-4);
                break;
              case DATATYPE::INT32:
                ASSERT_EQ(((int32_t*)t1.tensors[i].data)[k], ((int32_t*)t2.tensors[j].data)[k]);
                break;
              case DATATYPE::INT64:
                ASSERT_EQ(((int64_t*)t1.tensors[i].data)[k], ((int64_t*)t2.tensors[j].data)[k]);
                break;
              case DATATYPE::STRING:
                ASSERT_EQ((std::string)(((char**)t1.tensors[i].data)[k]),
                          (std::string)(((char**)t2.tensors[j].data)[k]));
                break;
              default:
                throw std::runtime_error("dataType=" + std::to_string(t1.tensors[i].dataType) +
                                         "not supported for tensor in gtest");
            }
          }
          break;
        }
      }
    }
    ASSERT_TRUE(foundTensor);
  }
}

void TestsUtil::check_script_run(CommandCenter& commandCenter, const std::string& functionName,
                                 const std::string& inputFileName,
                                 const std::string& outputFileName) {
  std::string inputsJson;
  bool inputLoaded = ServerHelpers::get_file_from_assets(inputFileName, inputsJson);
  ASSERT_EQ(inputLoaded, true);
  CTensors input = get_CTensors_from_json(inputsJson.c_str());
  CTensors output;
  auto status = commandCenter.run_task(GLOBALTASKNAME, functionName.c_str(), input, &output);
  ASSERT_EQ(status, nullptr);
  bool expectedOutputLoaded = ServerHelpers::get_file_from_assets(outputFileName, inputsJson);
  ASSERT_EQ(expectedOutputLoaded, true);
  CTensors expectedOutput = get_CTensors_from_json(inputsJson.c_str());
  compare_tensors(output, expectedOutput);
  commandCenter.deallocate_output_memory(&output);
  deallocate_CTensors(input);
  deallocate_CTensors(expectedOutput);
}
