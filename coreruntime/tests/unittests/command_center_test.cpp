/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <memory>

#include "core_sdk.hpp"
#include "native_interface.hpp"
#include "nimbletest.hpp"
#include "server_api.hpp"
#include "util.hpp"

class CommandCenterTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    /* Write your Setup for Test here*/
  };

  virtual void TearDown() override { /* Write your teardown for Test here*/ };
};

TEST(CommandCenterTest, MultipleInitTest) {
  auto config = std::make_shared<Config>(configJsonChar);
  config->add_model("contestRankingDemo");
  config->online = false;  // to prevent background thread to start running
  auto coreSDK = std::make_unique<CoreSDK>();
  auto initConfig = config;
  coreSDK->initialize(config);

  auto newConfig = std::make_shared<Config>(configJsonChar);
  newConfig->add_model("randomModelId");

  coreSDK->initialize(newConfig);

  ASSERT_EQ(coreSDK->get_config()->get_modelIds()[0], initConfig->get_modelIds()[0]);
}
