/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <set>

#include "asset_manager.hpp"
#include "executor_structs.h"
#include "json.hpp"
#include "tests_util_structs.hpp"

class CommandCenter;

class TestsUtil final {
 private:
  static inline const std::string _mockServerHost = "http://localhost:8080";
  static inline const std::string _setExpectationURL = _mockServerHost + "/mocker/expectation";
  static inline const std::string _resetExpectationURL = _mockServerHost + "/mocker/reset";
  static inline const std::string _historicalAPIsURL = _mockServerHost + "/mocker/history";

  TestsUtil() = delete;
  static HistoricalAPIs get_historical_api_calls();
  static void create_file_if_not_present(const std::string& filePath);
  static nlohmann::json read_api_calls_history_file(const std::string& filePath);
  static void write_json_data_to_file(const std::string& filePath, nlohmann::json&& data);

  static std::string get_historical_apis_file_path_from_test_name(const std::string& testName) {
    return "./assets/end_to_end_test/" + testName + ".json";
  }

  static void compare_json_arrays_util(void* jsonPointerOutput, void* expectedJsonPointer,
                                       void* json_allocator);
  static void compare_json_objects_util(void* jsonPointerOutput, void* expectedJsonPointer,
                                        void* json_allocator);
  static void compare_json_objects(void* outputJson, void* expectedJsonOutput,
                                   void* json_allocator);
  static void compare_json_arrays(void* outputJson, void* expectedJsonOutput, void* json_allocator);

 public:
  // Initialization functions
  static NimbleNetStatus* initialize_and_is_ready(const std::string& validE2EConfigJson);
  static NimbleNetStatus* initialize_and_is_ready_with_sleep(const std::string& validE2EConfigJson);
  static NimbleNetStatus* add_event(const std::string& eventType, CUserEventsData* cUserEventsData,
                                    int waitTimeForJobScheduler = 0);

  // Mock server expectations
  static bool set_expectations(
      const std::string& path, int response_delay, int expected_status_code = 200,
      const nlohmann::json& expected_headers = nlohmann::json::object(),
      const nlohmann::json& expected_response_body = nlohmann::json::object());
  static bool reset_expectations();

  // Assertions functions
  static void assert_deployment(const std::string& functionName, int expectedValue);
  static void assert_deployment_with_no_model();
  static void assert_deployment_with_modules();
  static void compare_tensors(CTensors t1, CTensors t2);
  static void check_script_run(CommandCenter& commandCenter, const std::string& functionName,
                               const std::string& inputFileName, const std::string& outputFileName);
  // When creating a new integration test, to get the historical APIs data written in a file,  pass
  // dumpHistoricalData as true
  static void assert_historical_api_calls(const std::string& testName, int index,
                                          bool dumpHistoricalData = false);

  // Utility functions to create Integration test assets
  static void dump_historical_api_calls(const std::string& testName, int index);

  // Filesystem utility functions
  static bool delete_file(const std::string& filePath);
  static bool copy_file(const std::string& source, const std::string& destination);
  static void copy_asset(const std::string& path, const std::string& version,
                         const std::string& name, const AssetType assetType);
  static std::set<std::string> get_file_names_from_folder(const std::string& folderName);
  static bool corrupt_file(const std::string& filePath, int numBytes = 8, int offset = 20);
  static bool create_archive(const std::string& folderName);
};
