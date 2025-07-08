/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "nimblenet.hpp"
#include "nimbletest.hpp"
#include "server_api.hpp"
#include "tests_util.hpp"

class AddEventEndToEndTest : public ::testing::Test {
 protected:
  std::shared_ptr<ServerAPI> _serverAPI;
  std::shared_ptr<Config> _config;
  MetricsAgent* _metricsAgent;
  std::string _testName;
  const std::string _deviceConfig = R"(
		{   
			"deviceId": "testDevice",
			"internalDeviceId": "intDevID",
			"clientId": "testclient",
			"host": "http://localhost:8080",
			"clientSecret": "dummy",
            "debug": true,
			"compatibilityTag": "ADD_EVENT",
			"databaseConfig" : [],
			"online": true,
			"cohortIds": [],
      "sessionId": "1"
		}
	)";

  const std::string _expectedInitialCloudConfigResponse = R"({
    "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {"sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "key": "apikey"}, "writer" : {"eventTypesToWrite" : {}, "collectEvents": false}}
  })";
  int _apiAssertionIndex = 0;
  std::string _cloudConfigUrl;

  void SetUp() override {
    _testName = testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string testFolder = "./testrun/" + std::string(_testName) + "/";
    ASSERT_TRUE(ServerHelpers::create_folder(testFolder));
    nativeinterface::HOMEDIR = testFolder;
    _config = std::make_shared<Config>(_deviceConfig);
    _metricsAgent = new MetricsAgent();
    _metricsAgent->initialize(logger);
    _serverAPI = std::make_shared<ServerAPI>(_metricsAgent, _config);
    _serverAPI->init();
    // Set expectation of cloudConfig to return no eventTypes before each test run
    _cloudConfigUrl = _serverAPI->get_cloudconfig_url(_deviceConfig);
    ASSERT_TRUE(
        TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                    nlohmann::json::parse(_expectedInitialCloudConfigResponse)));
  };

  void TearDown() override {
    TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
    nimblenetInternal::reset();
    ASSERT_TRUE(TestsUtil::reset_expectations());
    delete _metricsAgent;
  };
};

// Test where event of each type is fired for the first time after script load
// Assert that register event calls are getting made
// Assert the response of add_event function
// When events sent again in next session, no register_event call made
// No call to ingestion service as all the events are marked as false in cloudConfig and also
// collectEvents is not set, so it is false
TEST_F(AddEventEndToEndTest,
       add_event_e2e_test_checking_register_and_ingestion_calls_with_collect_events_false) {
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  CUserEventsData cUserEventData;

  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType2", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType3", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType4", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  status = TestsUtil::initialize_and_is_ready_with_sleep(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType2", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType3", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType4", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);
}

// Test where event of each type is fired for the first time after script load
// Assert that register event calls are getting made
// Assert the response of add_event function
// When event of same type sent again in next session, no register_event call made
// No call to ingestion service and also not sent to frontend as all the events are marked as false
// in cloudConfig even though collectEvents set to true
TEST_F(AddEventEndToEndTest,
       add_event_e2e_test_checking_register_and_ingestion_calls_with_collect_events_true) {
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  CUserEventsData cUserEventData;

  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType2", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType3", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType4", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());
  // Set cloudConfig to return true for collectEvents but all individual events are still marked as
  // false
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error" : null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":false,"eventType3":false,"updatedEventType":false}, "collectEvents": true}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  status = TestsUtil::initialize_and_is_ready_with_sleep(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType2", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType3", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType4", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);
}

// One event type is fired for the first time after script load
// Assert that register event calls are getting made
// Assert the response of add_event function
// Make collectEvents as true and eventType as true, and false for respective events in
// eventTypesToWrite. When event sent again in next session, no register_event call made, but event
// sent to frontend and also sent to ingestion service
TEST_F(
    AddEventEndToEndTest,
    add_event_e2e_test_checking_register_and_ingestion_calls_with_collect_events_true_and_event_type_to_write_true) {
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  CUserEventsData cUserEventData;

  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("eventType2", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // Set cloudConfig to return true for collectEvents and eventType as true
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":true, "eventType2": false, "updatedEventType": false}, "collectEvents": true}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  status = TestsUtil::initialize_and_is_ready_with_sleep(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  status = TestsUtil::add_event("eventType1", &cUserEventData, 0);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");

  status = TestsUtil::add_event("eventType2", &cUserEventData, 3);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, nullptr);
  ASSERT_STREQ(cUserEventData.eventJsonString, nullptr);
}

// Event of some types are fired for the first time after script load
// Assert that register event calls are getting made
// Assert the response of add_event function
// When events sent again in next session, no register_event call made
// No Call to ingestion service made even if some of the events are marked as true in cloudConfig
// and collectEvents set to false, but the event will be sent to frontend
TEST_F(
    AddEventEndToEndTest,
    add_event_e2e_test_checking_register_and_ingestion_calls_with_collect_events_false_and_event_type_to_write_true) {
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  CUserEventsData cUserEventData;

  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  status = TestsUtil::add_event("updatedEventType", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_EQ(cUserEventData.eventType, nullptr);
  ASSERT_EQ(cUserEventData.eventJsonString, nullptr);

  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());
  // Set cloudConfig to return true for collectEvents and eventType as true
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":true, "updatedEventType": true}, "collectEvents": false}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));

  status = TestsUtil::initialize_and_is_ready_with_sleep(_deviceConfig);
  ASSERT_EQ(status, nullptr);
  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");

  status = TestsUtil::add_event("updatedEventType", &cUserEventData, 1);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "updatedEventType");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
}

// Add event called before script initialization
// The one register call also happens because we are doing serverAPI->init() in Setup of this test
TEST_F(AddEventEndToEndTest, add_event_e2e_before_initialization) {
  CUserEventsData cUserEventData;
  auto status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status->code, 1);
  ASSERT_STREQ(status->message, "NimbleNet is not initialized");
}

// Add event called before command is ready
TEST_F(AddEventEndToEndTest, add_event_e2e_before_command_center_ready) {
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "0.0.0",
            "type": "script",
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/0.0.0/formats/json","isPrivate": false},
             "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":false,"eventType3":false,"updatedEventType":false}, "collectEvents": true}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status->code, 404);
  ASSERT_STREQ(status->message, "Not ready for exposing.");

  CUserEventsData cUserEventData;
  status = TestsUtil::add_event("eventType1", &cUserEventData, 1);
  ASSERT_EQ(status->code, 400);
  ASSERT_STREQ(status->message, "Cannot add/update event since NimbleEdge is not ready");
}

// Event with invalid payload added
TEST_F(AddEventEndToEndTest, add_event_e2e_with_invalid_payload) {
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);

  CUserEventsData cUserEventData;
  status = nimblenet::add_event("{\"id\": 1, \"floatData\": 1.12, \"stringData\": abcd\"}",
                                "eventType1", &cUserEventData);
  ASSERT_NE(status, nullptr);
  ASSERT_EQ(status->code, 400);
  ASSERT_STREQ(status->message,
               "Error in parsing event for table:eventType1 with eventMap: {\"id\": 1, "
               "\"floatData\": 1.12, \"stringData\": abcd\"} with error: "
               "[json.exception.parse_error.101] parse error at line 1, column 44: syntax error "
               "while parsing value - invalid literal; last read: '\"stringData\": a'");
}

// Initialize, add some events such that they are present on disk, but they are not sent
// Call send_events with minimal config, events should be sent
TEST_F(AddEventEndToEndTest, send_events_with_minimal_config) {
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": false, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":true,"eventType3":true,"updatedEventType":true}, "collectEvents": true}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);

  CUserEventsData cUserEventData;
  status = TestsUtil::add_event("eventType1", &cUserEventData);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  nlohmann::json minimalConfig = nlohmann::json(
      {{"deviceConfig", _deviceConfig},
       {"externalLoggerConfig",
        nlohmann::json::parse(
            R"({"sender":{"interval":1,"maxConcurrentLogFailures":3,"maxFilesToSend":5,"sendFirstLog":true,"sendLogsProbability":1.0,"url":"http://localhost:8080/externalLogger", "key": "apikey"},"writer":{"collectEvents":false,"eventTypesToWrite":{},"logTypesToWrite":{},"maxLogFileSizeKB":1,"scriptVerbose":false}})")}});

  auto send_events_status = nimblenet::send_events(minimalConfig.dump(), nativeinterface::HOMEDIR);
  Time::sleep_until(3);  // Wait for scheduler to send the events
}

// Initialize with correct externalLogger keys -- Events sent and cloudConfig saved on disk
// Next time CloudConfig sends 304 unmodified status code -- Events should be sent again with
// correct externalLogger key being picked up
TEST_F(AddEventEndToEndTest, events_sent_by_reading_cloud_config_from_device) {
  std::string expectedCloudConfigResponse = R"({
      "deployment": {
        "id": 8,
        "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
    },
    "status": 200,
    "error": null,
    "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true, "key": "apikey"},
            "writer" : {"eventTypesToWrite" : {"eventType1":true}, "collectEvents": true}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);

  CUserEventsData cUserEventData;
  // Having to wait for three seconds as we send logs only if previously attempted time is greater
  // than time_interval sent in cloudConfig
  status = TestsUtil::add_event("eventType1", &cUserEventData, 3);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
  TestsUtil::assert_historical_api_calls(_testName, _apiAssertionIndex++);
  nimblenetInternal::reset();
  ASSERT_TRUE(TestsUtil::reset_expectations());

  // CloudConfig returns 304 unmodified
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 304, nlohmann::json::object(),
                                          nlohmann::json::object()));
  // Event added and should be sent to ingestion service by using the correct secret_key picked up
  // from disk
  status = TestsUtil::initialize_and_is_ready_with_sleep(_deviceConfig);
  status = TestsUtil::add_event("eventType1", &cUserEventData, 2);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
}

TEST_F(AddEventEndToEndTest, add_event_with_script_log_test) {
  std::string expectedCloudConfigResponse = R"({
    "deployment": {
      "id": 8,
      "script": {
            "name": "DEFAULT_SCRIPT",
            "version": "5.0.0", 
            "type": "script", 
            "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
            "metadata": {}
        },
        "modules": []
      },
      "externalLogger" :
        {
            "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger",
            "sendFirstLog": true, "key": "apikey"}, "writer" : {"eventTypesToWrite" :
            {"eventType1":true}, "collectEvents": true, "scriptVerbose": true}
      },
      "error" : null

  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);

  CUserEventsData cUserEventData;
  status = TestsUtil::add_event("eventType1", &cUserEventData, 3);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
}

// Sending events via external key using defaultKey and internal logs with apikey being set
// correctly
TEST_F(AddEventEndToEndTest, send_events_with_default_logger_key) {
  std::string expectedCloudConfigResponse = R"({
    "deployment": {
        "id": 8,
        "script": {
              "name": "DEFAULT_SCRIPT",
              "version": "5.0.0", 
              "type": "script", 
              "location": {"path": "/clients/testclient/assets/script/names/DEFAULT_SCRIPT/versions/5.0.0/formats/json", "isPrivate": false}, 
              "metadata": {}
          },
          "modules": []
        },
        "error" : null,
        "externalLogger" : {
              "sender" : {"interval" : 1, "url" : "http://localhost:8080/externalLogger", "sendFirstLog": true},
              "writer" : {"eventTypesToWrite" : {"eventType1":true}, "collectEvents": true}
          },
        "nimbleLogger":{
              "sender" : {"interval" : 1, "url" : "http://localhost:8080/logger", "sendFirstLog": true, "key": "apikey"},
              "writer" : {}
        }
  })";
  ASSERT_TRUE(TestsUtil::set_expectations(_cloudConfigUrl, 0, 200, nlohmann::json::object(),
                                          nlohmann::json::parse(expectedCloudConfigResponse)));
  auto status = TestsUtil::initialize_and_is_ready(_deviceConfig);
  ASSERT_EQ(status, nullptr);

  CUserEventsData cUserEventData;
  // Having to wait for three seconds as we send logs only if previously attempted time is greater
  // than time_interval sent in cloudConfig
  status = TestsUtil::add_event("eventType1", &cUserEventData, 3);
  ASSERT_EQ(status, nullptr);
  ASSERT_STREQ(cUserEventData.eventType, "eventType1");
  ASSERT_STREQ(cUserEventData.eventJsonString,
               "{\"floatData\":1.12,\"id\":1,\"stringData\":\"abcd\"}");
}
