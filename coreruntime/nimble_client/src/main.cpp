#include <unistd.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "input_structs.hpp"
#include "nimblejson.hpp"
#include "nimblenet.h"
#include "nlohmann/json.hpp"
#include "string.h"
#include "task_input_structs.hpp"
#include "time.h"
using namespace std;

int main(int argc, char** argv) {
  cout << initialize_nimblenet(
              R"delim(
		{
			"deviceId": "preprocessingDevice",
			"clientId": "testclient",
			"host": "https://apiv3.nimbleedge-staging.com",
			"clientSecret": "samplekey123",
      "compatibilityTag": "generic-loader",
      "maxInputsToSave": 1,
      "internalDeviceId": "TestDevice1",
      "debug": true
		}
	)delim",
              "./")
       << endl;

  while (is_ready()) {
    cout << "Nimblenet not ready, sleeping\n";
    sleep(1);
  }

  int sleeptime = 0;
  if (argc >= 2) sleeptime = stoi(argv[1]);
  internet_switched_on();
  const char* modelId2 = "simpleNet";

  std::ifstream inFile("../tests/assets/nimble_client/main.ast", ios::binary);
  if (inFile.fail()) {
    cout << "Code File Not found" << endl;
    exit(0);
  }
  CUserEventsData cUserEventsData;

  add_event(
      "{\"contestType\": \"special\", \"productid\": 1, \"roundid\": 27, \"winnerPercent\": 25, "
      "\"prizeAmount\": 100.7, \"entryFee\": 35}",
      "TEST_EVENT", &cUserEventsData);
  auto result = string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
  load_task(result.c_str());

  usleep(sleeptime * 1000000);
}
