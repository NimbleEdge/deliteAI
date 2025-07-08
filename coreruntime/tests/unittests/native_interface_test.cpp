/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include "nimbletest.hpp"
#include "tests_util.hpp"

class NativeInterfaceTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    const char* testName = testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string testFolder = "./testrun/" + std::string(testName) + "/";
    ASSERT_TRUE(ServerHelpers::create_folder(testFolder));
    nativeinterface::HOMEDIR = testFolder;
  };

  virtual void TearDown() override { /* Write your teardown for Test here*/ };
};

#ifdef GENAI
TEST_F(NativeInterfaceTest, UnzipArchiveTest) {
  std::string sourceFilePath;
  std::string destinationFolderPath = "archive_test";
  ASSERT_TRUE(ServerHelpers::get_full_file_path_from_assets(
      "native_interface_test/archive_test.zip", sourceFilePath));
  ASSERT_TRUE(TestsUtil::copy_file(sourceFilePath, nativeinterface::HOMEDIR + "archive_test.zip"));
  ASSERT_TRUE(nativeinterface::unzip_archive("archive_test.zip", destinationFolderPath));

  std::set<std::string> expected_files = {"1.txt", "2.json"};
  ASSERT_EQ(TestsUtil::get_file_names_from_folder(destinationFolderPath), expected_files);
  std::string txtFileContent;
  std::string jsonFileContent;
  nativeinterface::get_file_from_device_common(destinationFolderPath + "/1.txt", txtFileContent,
                                               false);
  nativeinterface::get_file_from_device_common(destinationFolderPath + "/2.json", jsonFileContent,
                                               false);
  ASSERT_STREQ(txtFileContent.c_str(), "SampleText1\nSampleText2");
  ASSERT_STREQ(jsonFileContent.c_str(), "{\n    \"key\": 1,\n    \"value\": \"val\"\n}");
}
#endif  // GENAI
