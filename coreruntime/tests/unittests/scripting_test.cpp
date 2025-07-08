/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#ifdef SCRIPTING
#include "command_center.hpp"
#include "input_structs.hpp"
#include "native_interface.hpp"
#include "nimblejson.hpp"
#include "nimbletest.hpp"
#include "server_api.hpp"
#include "task_input_structs.hpp"
#include "tests_util.hpp"
#include "time_manager.hpp"
#include "user_events_manager.hpp"
#include "util.hpp"

using namespace std;

class ScriptingTest : public ::testing::Test {
 protected:
  CommandCenter* commandCenter;
  MetricsAgent metricsAgent;
  Database* database;

  virtual void SetUp() override {
    auto config = std::make_shared<Config>(scriptConfigJsonChar);
    const char* testName = testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string testFolder = "./testrun/" + std::string(testName) + "/";
    ASSERT_TRUE(ServerHelpers::create_folder(testFolder));
    nativeinterface::HOMEDIR = testFolder + "common/";
    ASSERT_TRUE(ServerHelpers::create_folder(nativeinterface::HOMEDIR));
    metricsAgent.initialize(logger);
    std::shared_ptr<ServerAPI> serverAPI = std::make_shared<ServerAPI>(&metricsAgent, config);
    database = new Database(&metricsAgent);
    auto externalLogger = std::make_shared<Logger>();
    auto scheduler = std::make_shared<JobScheduler>(coresdkconstants::JobSchedulerCapacity);
    auto deployment = jsonparser::get<Deployment>(scriptDeploymentJson);
    commandCenter = new CommandCenter(serverAPI, config, &metricsAgent, database, scheduler,
                                      externalLogger, true, deployment);
    ResourceManager& resourceManager = commandCenter->get_resource_manager();
    UserEventsManager& userEventsManager = commandCenter->get_userEventsManager();
    commandCenter->pegged_device_time(PeggedDeviceTime(
        DeviceTime::zero.add_duration(Duration::from_microseconds(1)), Duration::zero));
  };

  virtual void TearDown() override {
    delete commandCenter;
    delete database;
  };
};

// Load task should throw an error when the schema defined for table does not match
// with the enriched event returned by the decorator
TEST_F(ScriptingTest, InvalidEventTest) {
  std::string modelId = "ContestRanking";
  std::string tableName = "ContestJoinedClient";

  // Add entry for EventsType table and values inside it
  nlohmann::json contestJoinedClientTableSchema = {
      {"contestType", "string"},  {"productid", "int32"},    {"roundid", "int32"},
      {"winnerPercent", "float"}, {"prizeAmount", "double"}, {"entryFee", "int32"}};
  commandCenter->get_userEventsManager().add_eventType(tableName, contestJoinedClientTableSchema);
  std::string allEventsJsonString;
  bool status =
      ServerHelpers::get_file_from_assets("contest_ranking/all_events.json", allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);
  Time::sleep_until(1);

  for (auto& eventJson : allEventsJson["warmup"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(eventJson["event"].dump(), tableName);
  }
  TestsUtil::copy_asset("contest_ranking/invalid_contest_ranking.ast", "1.0.0", "DEFAULT_SCRIPT",
                        AssetType::SCRIPT);
  commandCenter->job_scheduler()->do_all_non_priority_jobs();
  // TODO: Add a good check for checking invalid events
}

// Load task should throw an error if sum aggregation done on a key of type string
TEST_F(ScriptingTest, InvalidAggregatorLoadTaskTest) {
  std::string modelId = "ContestRanking";
  std::string tableName = "ContestJoinedClient";

  // Add entry for EventsType table and values inside it
  nlohmann::json contestJoinedClientTableSchema = {
      {"contestType", "string"},  {"productid", "int32"},    {"roundid", "int32"},
      {"winnerPercent", "float"}, {"prizeAmount", "double"}, {"entryFee", "int32"}};

  commandCenter->get_userEventsManager().add_eventType(tableName, contestJoinedClientTableSchema);
  std::string allEventsJsonString;
  bool status =
      ServerHelpers::get_file_from_assets("contest_ranking/all_events.json", allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);
  Time::sleep_until(1);

  for (auto& eventJson : allEventsJson["warmup"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(eventJson["event"].dump(), tableName);
  }

  TestsUtil::copy_asset("contest_ranking/invalid_agg_contest_ranking.ast", "1.0.0",
                        "DEFAULT_SCRIPT", AssetType::SCRIPT);

  commandCenter->job_scheduler()->do_all_non_priority_jobs();
  ASSERT_FALSE(commandCenter->is_ready());
  ASSERT_TRUE(commandCenter->is_task_initializing());
}

// Trying to add event of type InvalidEventType should not get added in DB, as its entry is not
// present in EventType table
TEST_F(ScriptingTest, AddInvalidEventTypeEvent) {
  std::string modelId = "ContestRanking";
  std::string tableName = "ContestJoinedClient";
  std::string invalidEventType = "InvalidEventType";

  // Add entry for EventsType table and values inside it
  nlohmann::json contestJoinedClientTableSchema = {
      {"contestType", "string"},  {"productid", "int32"},    {"roundid", "int32"},
      {"winnerPercent", "float"}, {"prizeAmount", "double"}, {"entryFee", "int32"}};
  commandCenter->get_userEventsManager().add_eventType(tableName, contestJoinedClientTableSchema);
  std::string allEventsJsonString;
  bool status =
      ServerHelpers::get_file_from_assets("contest_ranking/all_events.json", allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);
  Time::sleep_until(1);

  for (auto& eventJson : allEventsJson["warmup"]) {
    float timeToSleep = eventJson["sleepTime"];
    auto eventData = commandCenter->add_user_event(eventJson["event"].dump(), invalidEventType);
    ASSERT_EQ(eventData.status, nullptr);
  }

  // Assert that DB does not have events inside it
  int count = commandCenter->get_userEventsManager().get_count_from_eventsTable(invalidEventType);
  ASSERT_EQ(count, 0);
}

// Run task should throw an error if task has a processor created with a key and trying to get its
// output, which is not present in input
TEST_F(ScriptingTest, KeyNotPresentInInputTest) {
  std::string modelId = "ContestRanking";
  std::string tableName = "ContestJoinedClient";

  // Add entry for EventsType table and schema inside it
  nlohmann::json contestJoinedClientTableSchema = {
      {"contestType", "string"},  {"productid", "int32"},    {"roundid", "int32"},
      {"winnerPercent", "float"}, {"prizeAmount", "double"}, {"entryFee", "int32"}};
  commandCenter->get_userEventsManager().add_eventType(tableName, contestJoinedClientTableSchema);
  std::string allEventsJsonString;
  bool status =
      ServerHelpers::get_file_from_assets("contest_ranking/all_events.json", allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);
  Time::sleep_until(1);

  for (auto& eventJson : allEventsJson["warmup"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(eventJson["event"].dump(), tableName);
  }
  TestsUtil::copy_asset("contest_ranking/contest_ranking.ast", "1.0.0", "DEFAULT_SCRIPT",
                        AssetType::SCRIPT);

  commandCenter->job_scheduler()->do_all_non_priority_jobs();

  // adding events in session after preprocessor created when loading model
  for (auto& eventJson : allEventsJson["inSession"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(eventJson["event"].dump(), tableName);
  }

  std::string inputsJson;
  bool inputLoaded = ServerHelpers::get_file_from_assets("contest_ranking/input.json", inputsJson);
  ASSERT_EQ(inputLoaded, true);
  CTensors input = get_CTensors_from_json(inputsJson.c_str());
  CTensors output;
  std::string functionName = "get_processor_output_invalid_key";
  auto taskStatus = commandCenter->run_task(GLOBALTASKNAME, functionName.c_str(), input, &output);
  ASSERT_NE(taskStatus, nullptr);
  deallocate_CTensors(input);
  deallocate_nimblenet_status(taskStatus);
}

// Verify that the output of processor for an enriched key is correct
TEST_F(ScriptingTest, EnrichedKeyProcessorOutput) {
  std::string modelId = "ContestRanking";
  std::string tableName = "ContestJoinedClient";

  // Add entry for EventsType table and schema inside it
  nlohmann::json contestJoinedClientTableSchema = {
      {"contestType", "string"},  {"productid", "int32"},    {"roundid", "int32"},
      {"winnerPercent", "float"}, {"prizeAmount", "double"}, {"entryFee", "int32"}};
  commandCenter->get_userEventsManager().add_eventType(tableName, contestJoinedClientTableSchema);
  std::string allEventsJsonString;
  bool status =
      ServerHelpers::get_file_from_assets("contest_ranking/all_events.json", allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);
  Time::sleep_until(1);

  for (auto& eventJson : allEventsJson["warmup"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(
        DataVariable::get_map_from_json_object(std::move(eventJson["event"])), tableName);
  }

  TestsUtil::copy_asset("contest_ranking/contest_ranking.ast", "1.0.0", "DEFAULT_SCRIPT",
                        AssetType::SCRIPT);

  commandCenter->job_scheduler()->do_all_non_priority_jobs();

  // adding events in session after preprocessor created when loading model
  for (auto& eventJson : allEventsJson["inSession"]) {
    float timeToSleep = eventJson["sleepTime"];
    commandCenter->add_user_event(
        DataVariable::get_map_from_json_object(std::move(eventJson["event"])), tableName);
  }
  TestsUtil::check_script_run(*commandCenter, "get_enriched_key_processor_output",
                              "contest_ranking/enriched_key_input.json",
                              "contest_ranking/enriched_key_script_output.json");
}

TEST_F(ScriptingTest, ScriptAllFunctionalityTest) {
  std::string tableName = "ContestBundles";

  // Add entry for EventType table and schema inside it
  nlohmann::json contestBundleTableSchema = {
      {"bundleId", "int32"},         {"promotion_code", "int32"}, {"product_ids", "int32[]"},
      {"bundleStrings", "string[]"}, {"bundleFloats", "float[]"}, {"setStrings", "string[]"},
      {"setInts", "int32[]"}};
  commandCenter->get_userEventsManager().add_eventType(tableName, contestBundleTableSchema);

  // Add events
  std::string allEventsJsonString;
  bool status = ServerHelpers::get_file_from_assets("complete_script_test/all_events.json",
                                                    allEventsJsonString);
  ASSERT_EQ(status, true);
  // adding events before loading in memory so that they go to database directly
  auto allEventsJson = nlohmann::json::parse(allEventsJsonString);

  // Load script
  std::string taskAST;
  bool scriptFound =
      ServerHelpers::get_file_from_assets("complete_script_test/script.ast", taskAST);
  ASSERT_EQ(scriptFound, true);
  bool scriptLoaded = commandCenter->load_task(GLOBALTASKNAME, "1.0.0", std::move(taskAST));
  ASSERT_EQ(scriptLoaded, true);

  for (auto& eventJson : allEventsJson["contestBundles"]) {
    commandCenter->add_user_event(eventJson["event"].dump(), tableName);
  }

  // Check output of the script
  TestsUtil::check_script_run(*commandCenter, "main", "complete_script_test/input.json",
                              "complete_script_test/script_output.json");
}

// Verify that script can return failed nimblenet status along with output
TEST_F(ScriptingTest, ScriptReturnFalseTest) {
  std::string taskAST;
  ASSERT_TRUE(
      ServerHelpers::get_file_from_assets("basic_script_test/may_return_false.ast", taskAST));
  bool scriptLoaded = commandCenter->load_task(GLOBALTASKNAME, "1.0.0", std::move(taskAST));
  ASSERT_EQ(scriptLoaded, true);

  const auto get_input = [](const char* boolVal) {
    auto input = ne::fmt(R"([
      {
        "name": "shouldFail",
        "Data": %s,
        "shape": [],
        "length": 0,
        "type": 9
      }    
    ])",
                         boolVal);
    CTensors cTensors = get_CTensors_from_json(input.str);
    return cTensors;
  };

  const auto expectedOutputJson = R"([
    {
      "name": "numericData",
      "Data": 5,
      "shape": [],
      "length": 0,
      "type": 7
    },
    {
      "name": "message",
      "Data": "Some error occurred",
      "shape": [],
      "length": 0,
      "type": 8
    }
  ])";
  const auto expectedOutput = get_CTensors_from_json(expectedOutputJson);

  {
    CTensors output;
    auto status = commandCenter->run_task(GLOBALTASKNAME, "main", get_input("true"), &output);
    ASSERT_NE(status, nullptr);
    ASSERT_EQ(output.numTensors, 2);
    TestsUtil::compare_tensors(output, expectedOutput);
    commandCenter->deallocate_output_memory(&output);
    deallocate_nimblenet_status(status);
  }

  {
    CTensors output;
    auto status = commandCenter->run_task(GLOBALTASKNAME, "main", get_input("false"), &output);
    ASSERT_EQ(status, nullptr);
    ASSERT_EQ(output.numTensors, 2);
    TestsUtil::compare_tensors(output, expectedOutput);
    commandCenter->deallocate_output_memory(&output);
    deallocate_nimblenet_status(status);
  }
  deallocate_CTensors(expectedOutput);
}

TEST_F(ScriptingTest, MissingMainModuleTest) {
  TestsUtil::copy_asset("basic_script_test/missing_main_module.ast", "1.0.0", "DEFAULT_SCRIPT",
                        AssetType::SCRIPT);

  commandCenter->job_scheduler()->do_all_non_priority_jobs();
  ASSERT_FALSE(commandCenter->is_ready());
  ASSERT_TRUE(commandCenter->is_task_initializing());
}

TEST_F(ScriptingTest, InvalidModuleImportTest) {
  TestsUtil::copy_asset("basic_script_test/invalid_module_import.ast", "1.0.0", "DEFAULT_SCRIPT",
                        AssetType::SCRIPT);

  commandCenter->job_scheduler()->do_all_non_priority_jobs();
  ASSERT_FALSE(commandCenter->is_ready());
  ASSERT_TRUE(commandCenter->is_task_initializing());
}
#endif  // SCRIPTING
