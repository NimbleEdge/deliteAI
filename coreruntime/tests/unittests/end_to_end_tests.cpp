/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <future>

#include "command_center.hpp"
#include "input_structs.hpp"
#include "native_interface.hpp"
#include "nimblenet.hpp"
#include "nimbletest.hpp"
#include "server_api.hpp"
#include "single_variable.hpp"
#include "tests_util.hpp"
#include "util.hpp"

using namespace std;

class EndToEndTest : public ::testing::Test {
 protected:
  std::shared_ptr<ServerAPI> _serverAPI;
  std::shared_ptr<Config> _config;
  MetricsAgent* _metricsAgent;
  std::string _testName;

  // Each integration test has multiple nimblenet initializations and resets to mimic multiple app
  // sessions. Each session has its own set of API calls, _apiAssertionIndex shows the current
  // session index which needs to be checked
  int _apiAssertionIndex = 0;

  void SetUp() override {
    _testName = testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string testFolder = "./testrun/" + std::string(_testName) + "/";
    ASSERT_TRUE(ServerHelpers::create_folder(testFolder));
    nativeinterface::HOMEDIR = testFolder;
    _config = std::make_shared<Config>(validE2EConfigJson);
    _metricsAgent = new MetricsAgent();
    _metricsAgent->initialize(logger);
    _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, _config);
    _serverAPI->init();
  };

  void TearDown() override {
    TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
    nimblenetInternal::reset();
    ASSERT_TRUE(TestsUtil::reset_expectations());
    delete _metricsAgent;
  };
};

// Initialization is done and is_Ready is true, on initializing again, is_Ready is true but no API
// calls are made
TEST_F(EndToEndTest, run_sdk_with_correct_config_no_mock_test) {
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again, this time task and models should be fetched from disk and no API calls should
  // be made as cloudConfig is not modified
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
}

TEST_F(EndToEndTest, run_sdk_with_python_modules) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "PYTHON_MODULES";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());

  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("add", 7);
  TestsUtil::assert_deployment_with_modules();
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again, this time task and models should be fetched from disk and no API calls should
  // be made as cloudConfig is not modified
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  TestsUtil::assert_deployment("add", 7);
  TestsUtil::assert_deployment_with_modules();
  ASSERT_EQ(status, nullptr);
}

// CloudConfig returns 401, is_ready() returns false
TEST_F(EndToEndTest, run_sdk_with_cloud_config_unable_to_fetch_test) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 401));
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_NE(status, nullptr);
  ASSERT_EQ(status->code, 1);
  ASSERT_EQ(std::string(status->message), "NimbleNet is not initialized");
}

// Able to download the model, but not able to save it, fetch again and save it this time.
TEST_F(EndToEndTest, run_sdk_with_unable_to_save_on_disk_test) {
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // To simulate that there was an error while saving the file, delete the model file, on
  // initialize
  // model should be fetched and is_ready should be true
  auto deploymentConfig = ServerHelpers::load_deployment_config_from_device(_config);
  std::string modelFilePath =
      nativeinterface::HOMEDIR + deploymentConfig.modules[0]->get_file_name_on_device();
  ASSERT_EQ(TestsUtil::delete_file(modelFilePath), true);
  // Initialize again, this time task should be fetched from disk and model from cloud
  status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
}

// Initialization is done and is_ready() is true. In next session model download
// fails and the old deployment continues to work.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_but_model_download_fails) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "error": null,
        "status": 200
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Set expectation for updated model download which returns 400
  ASSERT_TRUE(TestsUtil::set_expectations(
      "http://localhost:8080/mds/api/v4/clients/testclient/assets/model/names/"
      "multiply_two_model/versions/1.0.0/formats/ort",
      0, 400));
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("add", 7);
}

// Initialization is done and is_ready() is true. In next session task download
// fails and the old deployment continues to work.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_but_task_download_fails) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "error": null,
        "status": 200
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Set expectation for updated task download which returns 400
  ASSERT_TRUE(
      TestsUtil::set_expectations("http://localhost:8080/mds/api/v4/clients/testclient/assets/"
                                  "script/names/DEFAULT_SCRIPT/versions/2.0.0/formats/json",
                                  0, 400));
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("add", 7);
}

// Initialization is done and is_ready() is true. In next session new deployment will be fetched and
// saved on disk. In next session new deployment will be loaded.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_one_model_changed_but_load_in_next_session) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "error": null,
        "status": 200
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("add", 7);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again and this time new model and script should get used
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("multiply", 10);
}

// Initialization is done and is_ready() is true. In next session new deployment will be fetched,
// saved on disk and also loaded as forceUpdate is true.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_one_model_changed_but_load_in_same_session) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "error": null,
        "status": 200
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Set forceUpdate as true in CloudConfig with newer deployment
  expectedCloudConfigResponse = R"({
          "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "2.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/2.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "multiply_two_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }],
            "forceUpdate": true
        },
        "status": 200
    })";
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("multiply", 10);
}

// Load deployment with MODEL_CHANGE tag, run_method works in the same session
// Load deployment with MODEL_UPDATE tag, run_method works in the same session
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_with_new_compatibility_tag) {
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("multiply", 10);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "MODEL_UPDATE";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("add", 8);
}

// Initialization is done and is_ready() is true. In next session new deployment(with model updated)
// will be fetched, saved on disk and in next session it will be used.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_model_version_update_in_next_session) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "MODEL_UPDATE";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(newConfigJson.dump());
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "status": 200,
        "error": null
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("add", 7);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again and this time new model version and script should get used
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("add", 8);
}

// Initialization is done and is_ready() is true. In next session new deployment(with one extra
// model) will be fetched, saved on disk and in next session it will be used.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_model_addition_update_in_next_session) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "MODEL_ADDITION";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(newConfigJson.dump());
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 1,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "1.0.0",
                "type": "script",
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/1.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "add_model",
                "version": "1.0.0",
                "type": "model",
                "location": {"path":
                "/clients/testclient/assets/model/names/add_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "status": 200,
        "error": null
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("add", 7);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again and this time new model version and script should get used
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("add_and_multiply", 14);
}

// Initialization is done and is_ready() is true. In next session new deployment(with no model
// present) will be fetched, saved on disk and in next session it will be used.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_no_model_present_in_next_session) {
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("multiply", 10);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "NO_MODEL";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment_with_no_model();
}

// Initialization is done and is_ready() is true. In next session new deployment(with same models
// and task) will be fetched, saved on disk and in next session it will be fetched.
TEST_F(EndToEndTest, run_sdk_with_upgrade_path_only_deployment_id_change_in_next_session) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 2,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "2.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/2.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "multiply_two_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "status": 200,
        "error": null
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("multiply", 10);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("multiply", 10);
}

// Init is done and saved on disk for a state with sampleEtag
// In next session, old state saved on disk is loaded but  cloudConfig returns an invalid modelId
// the cloudConfig saved on disk should have older eTag, so that we try to fetch the cloudConfig
// again in next session
// In next session we load the working state saved on disk and try to fetch cloudConfig again
TEST_F(EndToEndTest,
       run_sdk_with_invalid_cloud_config_in_new_command_center_and_old_etag_saved_on_disk) {
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(validE2EConfigJson);
  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 2,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "2.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/2.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "multiply_two_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/multiply_two_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "status": 200,
        "error": null
    })";
  std::string expectedCloudconfigHeaders = R"({"Etag": "sampleEtag"})";
  // Set expectation for cloudConfig with valid response
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200,
                                          nlohmann::json::parse(expectedCloudconfigHeaders),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 2,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "2.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/2.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": [{
                "name": "multiply_two_invalid_model", 
                "version": "1.0.0", 
                "type": "model", 
                "location": {"path": "/clients/testclient/assets/model/names/multiply_two_invalid_model/versions/1.0.0/formats/ort", "isPrivate": false}, 
                "metadata": {}
            }]
        },
        "status": 200,
        "error": null
    })";
  expectedCloudconfigHeaders = R"({"Etag": "80b63ce21d76b6957469013866eccee1"})";
  // Set expectations for cloudConfig with invalid deployment, the modelId is wrong, older state
  // should keep working, and the etag of this request should not be saved on disk
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200,
                                          nlohmann::json::parse(expectedCloudconfigHeaders),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  TestsUtil::assert_deployment("multiply", 10);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Initialize again and this time there should be a call for cloudConfig which returns 200 instead
  // of 304, as the cloudConfig should have been saved on disk with older eTag
  status = TestsUtil::initialize_and_is_ready_with_sleep(validE2EConfigJson);
}

// Load a deployment with a script that doesn't load
// The script tries to call a non-existing function in global scope, which throws
// We test that loading this deployment doesn't crash, and it doesn't get ready either
TEST_F(EndToEndTest, run_sdk_with_script_load_failure) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "SCRIPT_LOAD_FAILURE";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(newConfigJson.dump());

  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_NE(status, nullptr);
  ASSERT_EQ(status->code, 404);
  ASSERT_STREQ(status->message, "Not ready for exposing.");
}

// Load a deployment with a valid script. Then in next session, we get a new config with a script
// that doesn't load This tests that the old script continues to load and work, even in the next to
// next session.
TEST_F(EndToEndTest, run_sdk_with_new_script_load_failure) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "SCRIPT_LOAD_FAILURE";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto cloudConfigUrl = _serverAPI->get_cloudconfig_url(newConfigJson.dump());

  std::string expectedCloudConfigResponse = R"({
        "deployment": {
            "id": 9,
            "script": {
                "name": "DEFAULT_SCRIPT",
                "version": "4.0.0", 
                "type": "script", 
                "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/4.0.0/formats/json", "isPrivate": false}, 
                "metadata": {}
            },
            "modules": []
        },
        "status": 200,
        "error": null
    })";
  // Set expectations for cloudConfig with older deployment
  ASSERT_TRUE(TestsUtil::set_expectations(cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment_with_no_model();
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // This will load the script that is present on disk, which will load properly
  // It will look at the config, which will have a script that doesn't load.
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  TestsUtil::assert_deployment_with_no_model();
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // This creates a new session after new deployment was fetched in last session. This shows that
  // still the old script loads, which implies that the new script didn't load
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  TestsUtil::assert_deployment_with_no_model();  // old script is loaded
  ASSERT_EQ(status, nullptr);
}

TEST_F(EndToEndTest, run_sdk_with_invalid_device_config) {
  // debug flag is passed as a string instead of a boolean
  std::string invalidConfig = R"delim(
		{   
			"deviceId": "testDevice",
			"internalDeviceId": "intDevID",
			"clientId": "testclient",
			"host": "http://localhost:8080",
			"clientSecret": "dummy",
            "debug": "true",
			"compatibilityTag": "MODEL_CHANGE",
			"databaseConfig" : [],
			"online": true,
			"cohortIds": ["cohort1"]
		}
	)delim";
  auto status = nimblenet::initialize_nimblenet(invalidConfig, nativeinterface::HOMEDIR);
  ASSERT_NE(status, nullptr);
  ASSERT_EQ(status->code, 1);
  ASSERT_STREQ(status->message,
               "[json.exception.type_error.302] type must be boolean, but is string");
}

TEST_F(EndToEndTest, DISABLED_run_sdk_with_multiple_init_calls) {
  auto future1 = std::async(std::launch::async, []() {
    return nimblenet::initialize_nimblenet(validE2EConfigJson, nativeinterface::HOMEDIR);
  });

  auto future2 = std::async(std::launch::async, []() {
    return nimblenet::initialize_nimblenet(validE2EConfigJson, nativeinterface::HOMEDIR);
  });

  auto status1 = future1.get();
  auto status2 = future2.get();

  // Expect one to fail (nullptr) and the other to succeed (not nullptr)
  ASSERT_TRUE((status1 == nullptr && status2 != nullptr) ||
              (status1 != nullptr && status2 == nullptr));

  auto failedStatus = (status1 == nullptr) ? status2 : status1;
  ASSERT_EQ(failedStatus->code, 5002);
  ASSERT_STREQ(failedStatus->message,
               "Initialization is already in progress, might be called from different thread");
}

TEST_F(EndToEndTest, run_sdk_with_invalid_model) {
  auto status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_EQ(status, nullptr);
  TestsUtil::assert_deployment("multiply", 10);
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // To simulate that there was an error while reading the model file, corrupt the model file by
  // adding a few random bytes, on initialize  model load should fail and is_ready() should be false
  auto deploymentConfig = ServerHelpers::load_deployment_config_from_device(_config);
  std::string modelFilePath = nativeinterface::get_full_file_path_common(
      deploymentConfig.modules[0]->get_file_name_on_device());
  ASSERT_EQ(TestsUtil::corrupt_file(modelFilePath), true);
  status = TestsUtil::initialize_and_is_ready(validE2EConfigJson);
  ASSERT_NE(status, nullptr);
}

TEST_F(EndToEndTest, run_sdk_offline_mode) {
  std::string json_str = R"(
    [
        {
            "name": "workflow_script",
            "version": "1.0.0",
            "type": "script",
            "location": {
                "path": "./assets/end_to_end_test/add_script.ast"
            }
        },
        {
            "name": "add_model",
            "version": "1.0.0",
            "type": "model",
            "location": {
                "path": "./assets/end_to_end_test/add_two_model.onnx"
            }
        }
    ]
)";

  nlohmann::json j = nlohmann::json::parse(json_str);
  ASSERT_EQ(nimblenet::load_modules(j, nativeinterface::HOMEDIR), nullptr);
  std::string config = R"delim(
      {
          "online": false,
          "debug": true
      }
      )delim";
  ASSERT_EQ(TestsUtil::initialize_and_is_ready(config), nullptr);
  TestsUtil::assert_deployment("add", 7);
}

#ifdef GENAI
TEST_F(EndToEndTest, DISABLED_run_sdk_with_llm) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "LLM";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  Time::sleep_until(60);  // Waiting extra minute to download and unzip the model
  ASSERT_EQ(nimblenet::is_ready(), nullptr);

  // Invoke LLM and assert response
  auto map = std::make_shared<MapDataVariable>();
  map->set_value_in_map("query", OpReturnType(new SingleVariable<std::string>("How are you?")));
  auto outputMap = std::make_shared<MapDataVariable>();
  nimblenet::run_method("prompt_llm", map, outputMap);
  int i = 0;
  std::string outputFromLLM;
  while (i < 5) {
    nimblenet::run_method("get_next_str", map, outputMap);
    if (outputMap->in(OpReturnType(new SingleVariable<std::string>("finished")))) {
      break;
    }
    outputFromLLM += outputMap->get_string_subscript("str")->get_string();
    Time::sleep_until(1);
    i++;
  }
  ASSERT_GT(outputFromLLM.size(), 0);
  nimblenet::run_method("stop_running", map, outputMap);

  // This time LLM should not be downloaded
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  ASSERT_EQ(nimblenet::is_ready(), nullptr);
}

TEST_F(EndToEndTest, run_sdk_with_list_compatible_llms) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "LIST_COM_LLMS";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  ASSERT_EQ(nimblenet::is_ready(), nullptr);

  auto map = std::make_shared<MapDataVariable>();
  auto outputMap = std::make_shared<MapDataVariable>();
  nimblenet::run_method("get_compatible_llms", map, outputMap);
  std::string exp_output = R"({"llms":[{"name":"llama-3","provider":"custom"}]})";
  ASSERT_EQ(exp_output, outputMap->print());
}

TEST_F(EndToEndTest, run_sdk_with_llm_use_same_zip) {
  auto newConfigJson = nlohmann::json::parse(validE2EConfigJson);
  newConfigJson["compatibilityTag"] = "LLM";
  std::shared_ptr<Config> newConfig = std::make_shared<Config>(newConfigJson);
  _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, newConfig);
  auto status = TestsUtil::initialize_and_is_ready(newConfigJson.dump());
  Time::sleep_until(120);
  ASSERT_EQ(nimblenet::is_ready(), nullptr);

  // Invoke LLM and assert response
  auto map = std::make_shared<MapDataVariable>();
  map->set_value_in_map("query", OpReturnType(new SingleVariable<std::string>("How are you?")));
  auto outputMap = std::make_shared<MapDataVariable>();
  nimblenet::run_method("prompt_llm", map, outputMap);
  int i = 0;
  std::string outputFromLLM;
  while (i < 5) {
    nimblenet::run_method("get_next_str", map, outputMap);
    if (outputMap->in(OpReturnType(new SingleVariable<std::string>("finished")))) {
      break;
    }
    outputFromLLM += outputMap->get_string_subscript("str")->get_string();
    Time::sleep_until(1);
    i++;
  }
  ASSERT_GT(outputFromLLM.size(), 0);
  nimblenet::run_method("stop_running", map, outputMap);

  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Delete the folder and create archive to simulator user behaviour where app was closed in the
  // middle of unzipping
  auto deploymentConfig = ServerHelpers::load_deployment_config_from_device(newConfig);
  std::string llmFolderName = deploymentConfig.modules[0]->get_file_name_on_device();
  ASSERT_TRUE(TestsUtil::create_archive(llmFolderName));
  ASSERT_TRUE(util::delete_folder_recursively(nativeinterface::HOMEDIR + llmFolderName));

  // when initializing again there whould not be call to cloud
  status = TestsUtil::initialize_and_is_ready_with_sleep(newConfigJson.dump());
  Time::sleep_until(20);
  ASSERT_EQ(nimblenet::is_ready(), nullptr);
}
#endif
