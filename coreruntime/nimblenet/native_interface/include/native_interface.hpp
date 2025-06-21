/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "core_utils/fmt.hpp"
#include "native_interface_constants.hpp"
#include "native_interface_structs.hpp"

namespace nativeinterface {
extern std::string HOMEDIR;
// Length -1 for cases when sending null terminated string data as body. For any other form of data
// specify its length in this method
const std::shared_ptr<NetworkResponse> send_request(const std::string& body,
                                                    const std::string& header,
                                                    const std::string& url,
                                                    const std::string& method, int length = -1);
FileDownloadInfo download_to_file_async(const std::string& url, const std::string& headers,
                                        const std::string& fileName);
std::pair<bool, std::string> read_log_file(const std::string& logFileName);
std::pair<bool, std::string> read_potentially_compressed_file(const std::string& fileName,
                                                              bool filePathProvided = false);
bool get_file_from_device_common(const std::string& fileName, std::string& result,
                                 bool filePathProvided = false);
bool get_unencrypted_file_from_device_common(const std::string& fileName, std::string& result,
                                             bool filePathProvided = false);
bool compress_and_save_file_on_device(const std::string& content, const std::string& fileName);
std::string save_file_on_device_common(std::string&& content, const std::string& fileName,
                                       bool overWrite = true);

bool create_folder(const std::string& folderFullPath);
bool file_exists_common(const std::string& file);
bool folder_exists_common(const std::string& folderName, bool fullFilePathProvided = false);
int get_file_size_common(const std::string& fileName);
std::string get_full_file_path_common(const std::string& fileName) noexcept;
bool compress_file(const char* inFileName, const char* outFileName);
bool decompress_file(const std::string& inFileName, const std::string& outFileName);
bool write_compressed_data_on_file(const std::string&& content, const std::string& fileName);
void write_data_to_file(const std::string&& content, const std::string& fileName,
                        bool write_data_on_file = true);
bool delete_file(const std::string& fileName, bool fullFilePathProvided = false);
#ifdef GENAI
bool unzip_archive(const std::string& fileName, const std::string& destinationFolderName);
// OS LLM related functions
void initialize_os_llm();
void prompt_os_llm(const std::string& prompt);
void cancel_os_llm_query();
FileDownloadStatus check_os_llm_status();
std::optional<std::string> get_os_supported_llm();
#endif  // GENAI
void set_thread_priority_min();

void set_thread_priority_max();
bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char* workManagerConfigJsonChar);
}  // namespace nativeinterface
