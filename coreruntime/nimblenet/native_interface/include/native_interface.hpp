/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "core_utils/fmt.hpp"
#include "native_interface_constants.hpp"
#include "native_interface_structs.hpp"

namespace fs = std::filesystem;

/**
 * @brief nativeinterface namespace acts as an interface between C++ SDK and the frontend layers.
 * Whenever some function from Kotlin or Objective-C needs to be invoked from coreruntime it is done
 * via this namespace.
 */
namespace nativeinterface {

extern std::string HOMEDIR; /**< Root directory of the SDK used for storing all the assets, logs,
                               user events etc. */

/**
 * @brief Sends a synchronous HTTP request to the given URL with optional body and headers, by
 * calling appropriate platform specific method defined in client.h.
 *
 * @param body     Request body (e.g., JSON string or binary data)
 * @param header   HTTP headers as a single string
 * @param url      Endpoint URL
 * @param method   HTTP method (e.g., GET, POST)
 * @param length   Length of the body (default -1 means null-terminated string)
 * @return         Shared pointer to a NetworkResponse object
 */
const std::shared_ptr<NetworkResponse> send_request(const std::string& body,
                                                    const std::string& header,
                                                    const std::string& url,
                                                    const std::string& method, int length = -1);

/**
 * @brief Asynchronously downloads a file from the internet and saves it locally. For e.g. in
 * Android this method will use DownloadManager internally and return pending, completed or failed
 * status.
 *
 * @param url       URL of the file
 * @param headers   HTTP headers
 * @param filePath  File path, relative to HOMEDIR
 * @return          FileDownloadInfo structure with task metadata
 */
FileDownloadInfo download_to_file_async(const std::string& url, const std::string& headers,
                                        const std::string& filePath);

/**
 * @brief Reads the contents of a local log file.
 * @deprecated
 *
 * @param logFileName File name to read from
 * @return            Pair of success flag and file contents
 */
std::pair<bool, std::string> read_log_file(const std::string& logFileName);

/**
 * @brief Reads a file, decompressing it if needed.
 *
 * @param fileName           Path to the file
 * @param filePathProvided   If true then take the fileName as is else add HOMEDIR to get the
 * complete path.
 * @return                   Pair of success flag and file content
 */
std::pair<bool, std::string> read_potentially_compressed_file(const std::string& fileName,
                                                              bool filePathProvided = false);

/**
 * @brief Reads a file's content into a string from the device. Decrypting it if needed.
 *
 * @param fileName           File name or path
 * @param result             Output string to store contents
 * @param filePathProvided   Whether the full path is given
 * @return                   Success status
 */
bool get_file_from_device_common(const std::string& fileName, std::string& result,
                                 bool filePathProvided = false);

/**
 * @brief Similar to get_file_from_device_common, this method assumes that the file being read is
 * not encrypted.
 */
bool get_unencrypted_file_from_device_common(const std::string& fileName, std::string& result,
                                             bool filePathProvided = false);

/**
 * @brief Writes content to a file and then gz compresses it.
 *
 * @param content    Content to save
 * @param fileName   Target file name
 * @return           Success status
 */
bool compress_and_save_file_on_device(const std::string& content, const std::string& fileName);

/**
 * @brief Saves content to a file, optionally overwriting if it exists.
 *
 * @param content     File contents (rvalue)
 * @param fileName    File name to write to
 * @param overWrite   Whether to overwrite existing file
 * @return            Full path to saved file
 */
std::string save_file_on_device_common(std::string&& content, const std::string& fileName,
                                       bool overWrite = true);

/**
 * @brief Creates a new folder at the specified path.
 */
bool create_folder(const std::string& folderFullPath);

/**
 * @brief Checks if a file exists.
 */
bool file_exists_common(const std::string& file);

/**
 * @brief Checks if a folder exists.
 *
 * @param folderName             Folder name or path
 * @param fullFilePathProvided   Whether the full path is provided
 */
bool folder_exists_common(const std::string& folderName, bool fullFilePathProvided = false);

/**
 * @brief Gets the file size in bytes.
 *
 * @param fileName
 */
int get_file_size_common(const std::string& fileName);

/**
 * @brief Resolves and returns the full absolute path for a file.
 */
std::string get_full_file_path_common(const std::string& fileName) noexcept;

/**
 * @brief Compresses an input file and writes it to a new file.
 *
 * @param inFileName Input file to be compressed
 * @param outFileName Output file which stores the compressed data
 */
bool compress_file(const char* inFileName, const char* outFileName);

/**
 * @brief Decompresses a compressed file into another file.
 */
bool decompress_file(const std::string& inFileName, const std::string& outFileName);

/**
 * @brief Writes pre-compressed content to a file.
 */
bool write_compressed_data_on_file(const std::string&& content, const std::string& fileName);

/**
 * @brief Writes raw content to a file.
 */
void write_data_to_file(const std::string&& content, const std::string& fileName,
                        bool filePathProvided = true);

void create_symlink(const fs::path& target, const std::string& link);

/**
 * @brief Deletes a file.
 *
 * @param fullFilePathProvided Whether fileName is a full path
 */
bool delete_file(const std::string& fileName, bool fullFilePathProvided = false);

#ifdef GENAI

/**
 * @brief Unzips a ZIP archive into the destination folder.
 */
bool unzip_archive(const std::string& fileName, const std::string& destinationFolderName);

// -------------------- OS LLM integration APIs --------------------

/**
 * @brief Initializes the local OS-level LLM instance, if available.
 */
void initialize_os_llm();

/**
 * @brief Sends a prompt to the OS-level LLM for processing.
 */
void prompt_os_llm(const std::string& prompt);

/**
 * @brief Cancels any pending os-level LLM prompt execution.
 */
void cancel_os_llm_query();

/**
 * @brief Checks the status of the OS LLM, whether it is ready for inference or not.
 */
FileDownloadStatus check_os_llm_status();

/**
 * @brief Returns the name of LLM supported by the OS, if any.
 */
std::optional<std::string> get_os_supported_llm();

#endif  // GENAI

/**
 * @brief Sets current thread's priority to the minimum value allowed by the OS.
 */
void set_thread_priority_min();

/**
 * @brief Sets current thread's priority to the maximum value allowed by the OS.
 */
void set_thread_priority_max();

/**
 * @brief Schedules periodic log uploads using the Android WorkManager.
 *
 * @param repeatIntervalInMinutes        Interval between uploads
 * @param retryIntervalInMinutesIfFailed Retry interval on failure
 * @param workManagerConfigJsonChar      JSON configuration for the job
 */
bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char* workManagerConfigJsonChar);

/**
 * @brief Returns hardware information as a JSON object.
 *
 * @return JSON object containing hardware information
 */
nlohmann::json get_hardware_info();

#ifdef IOS
const char* get_phonemes(const char* text);
#endif  // IOS

}  // namespace nativeinterface
