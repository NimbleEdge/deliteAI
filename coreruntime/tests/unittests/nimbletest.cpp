/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "nimbletest.hpp"

#include <gtest/gtest.h>

#include "command_center.hpp"
#include "native_interface.hpp"
#include "server_api.hpp"
#include "util.hpp"

using namespace std;

const std::string configJsonChar = R"delim(
		{
			"deviceId": "testDevice",
			"internalDeviceId": "intDevID",
			"clientId": "testClient",
			"host": "http://dummy_url/",
			"clientSecret": "dummy",
            "debug": true,
            "maxInputsToSave": 2,
			"compatibilityTag": "test-tag",
			"databaseConfig" : [
						{
							"tableName": "ContestJoinedClient",
							"schema": {
								"contestType": "TEXT",
								"productid" : "INT",
								"roundid" : "INT",
								"winnerPercent" : "REAL",
								"prizeAmount": "REAL",
								"entryFee": "INT"
							},
							"expiryInMins":60
						}, 
						{
							"tableName": "UserInteraction",
							"schema":{
								"ID": "TEXT",
								"winnerPercentage" : "REAL",
								"entryFee": "REAL"
							},
							"expiryInMins":60
						}
					 ]
		}
	)delim";

const std::string scriptConfigJsonChar = R"delim(
		{
			"deviceId": "testDevice",
			"internalDeviceId": "intDevID",
			"clientId": "testclient",
			"host": "http://dummy_url",
			"clientSecret": "dummy",
            "debug": true,
            "maxInputsToSave": 2,
			"compatibilityTag": "test-tag",
			"databaseConfig" : [],
			"online": false,
			"cohortIds": ["cohort1"]
		}
	)delim";

const std::string scriptDeploymentJson = R"delim(
	{
		"id": 1,
		"forceUpdate": false,
		"script": {
			"name": "DEFAULT_SCRIPT",
			"type": "script",
			"version": "1.0.0"
		},
		"modules": [
			{
				"name": "ContestRanking",
				"version": "1.0.0",
				"type": "model"
			}
		]
	}
	)delim";

const std::string validE2EConfigJson = R"delim(
		{   
			"deviceId": "testDevice",
			"internalDeviceId": "intDevID",
			"clientId": "testclient",
			"host": "http://localhost:8080",
			"clientSecret": "dummy",
            "debug": true,
			"compatibilityTag": "MODEL_CHANGE",
			"databaseConfig" : [],
			"online": true,
			"cohortIds": ["cohort1"]
		}
	)delim";

TEST(SanityTest, CommandCenterInitializes) {
  auto config = std::make_shared<Config>(configJsonChar);
  MetricsAgent metricsAgent;
  std::shared_ptr<ServerAPI> serverAPI = std::make_shared<ServerAPI>(&metricsAgent, config);
  Database *database = new Database(&metricsAgent);
  metricsAgent.initialize(logger);
  auto scheduler = std::make_shared<JobScheduler>(coresdkconstants::JobSchedulerCapacity);
  CommandCenter commandCenter(serverAPI, config, &metricsAgent, database, scheduler, nullptr);
  ResourceManager &resourceManager = commandCenter.get_resource_manager();
  UserEventsManager &userEventsManager = commandCenter.get_userEventsManager();
  serverAPI->init();
}

int main(int argc, char **argv) {
  int returnCode = system("rm -rf ./testrun/");
  nativeinterface::HOMEDIR = "./testrun/";
  nativeinterface::create_folder(nativeinterface::HOMEDIR);

  logger->init_logger(nativeinterface::HOMEDIR + "testlogs/");
  logger->set_debug_flag(true);
  // Initialize the Google Test framework
  ::testing::InitGoogleTest(&argc, argv);

  // Run all tests on all files
  return RUN_ALL_TESTS();
}
