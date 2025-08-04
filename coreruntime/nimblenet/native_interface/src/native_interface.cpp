/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "native_interface.hpp"

#ifdef GENAI
#include "miniz.h"
#endif  // GENAI
#include <sys/stat.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <thread>

#include "client.h"
#include "logger.hpp"
#include "resource_manager_constants.hpp"
#include "util.hpp"
#include "zlib.h"

using namespace std;

static inline bool get_file_from_device(const std::string& fullFilePath, string& result,
                                        bool encrypted) {
  std::ifstream inFile(fullFilePath, ios::binary);
  if (inFile.fail()) return false;
  result = string((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
  if (encrypted) {
    util::decrypt_data(result.data(), result.size());
  }
  return true;
}

static inline std::string save_file_on_device(std::string&& content,
                                              const std::string& fullFilePath, bool overWrite) {
  util::encrypt_data(content.data(), content.size());
  if (overWrite) {
    ofstream outFile(fullFilePath, ios::out | ios::binary);
    outFile.write(content.c_str(), content.size());
  } else {
    ofstream outFile(fullFilePath, ios::out | ios::binary | ios::app);
    outFile.write(content.c_str(), content.size());
  }
  return fullFilePath;
}

namespace nativeinterface {
std::string HOMEDIR;

const std::shared_ptr<NetworkResponse> send_request(const string& body, const string& header,
                                                    const string& url, const string& method,
                                                    int length) {
  return std::make_shared<NetworkResponse>(
      ::send_request(body.c_str(), header.c_str(), url.c_str(), method.c_str(), length));
}

FileDownloadInfo download_to_file_async(const string& url, const string& header,
                                        const string& fileName) {
  return ::download_to_file_async(url.c_str(), header.c_str(), fileName.c_str(), HOMEDIR.c_str());
}

std::pair<bool, std::string> decompress_file_to_string(const char* inFileName) {
  gzFile inFile = gzopen(inFileName, "rb");
  if (inFile == Z_NULL) {
    LOG_TO_ERROR("could not open gzFile=%s", inFileName);
    return {false, ""};
  }

  std::string uncompressedString;
  char buffer[128];
  int numRead = 0;
  while ((numRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
    uncompressedString.append(buffer, numRead);
  }

  gzclose(inFile);
  return {true, uncompressedString};
}

bool decompress_file(const std::string& inFileName, const std::string& outFileName) {
  // Open gzipped input file
  std::string fullInFileName = get_full_file_path_common(inFileName);
  std::string fullOutFileName = get_full_file_path_common(outFileName);
  gzFile inFile = gzopen(fullInFileName.c_str(), "rb");
  if (inFile == Z_NULL) {
    int err;
    LOG_TO_ERROR("could not open gzFile=%s, error=%s", inFileName.c_str(), gzerror(inFile, &err));
    return false;
  }

  // Open output file for normal binary writing
  FILE* outFile = fopen(fullOutFileName.c_str(), "wb");
  if (outFile == nullptr) {
    gzclose(inFile);
    LOG_TO_ERROR("could not open file=%s", outFileName.c_str());
    return false;
  }

  char buffer[128];
  int numRead = 0;
  while ((numRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
    size_t numWritten = fwrite(buffer, 1, numRead, outFile);
    if (numWritten != static_cast<size_t>(numRead)) {
      gzclose(inFile);
      fclose(outFile);
      LOG_TO_ERROR("Error while decompressing gzFile=%s to file=%s, error=%d", inFile, outFile,
                   errno);
      return false;
    }
  }

  gzclose(inFile);
  fclose(outFile);
  return true;
}

bool compress_file(const char* inFileName, const char* outFileName) {
  // TODO: Do proper RAII closing of files on error
  gzFile inFile = gzopen(inFileName, "rb");
  if (inFile == Z_NULL) {
    int err;
    // LOG_TO_ERROR("could not open gzFile=%s, error=%s", inFileName, gzerror(inFile, &err));
    return false;
  }

  gzFile outFile = gzopen(outFileName, "wb");
  if (outFile == Z_NULL) {
    gzclose(inFile);
    int err;
    // LOG_TO_ERROR("could not open gzFile=%s, error=%s", outFileName, gzerror(outFile, &err));
    return false;
  }

  char buffer[128];
  int numRead = 0;
  while ((numRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
    int numWritten = gzwrite(outFile, buffer, numRead);
    if (numWritten <= 0) {
      int err;
      // LOG_TO_ERROR("Error while decompressing file=%s to gzFile=%s, error=%s", inFile, outFile,
      //              gzerror(outFile, &err));
      gzclose(inFile);
      gzclose(outFile);
      return false;
    }
  }

  gzclose(inFile);
  gzclose(outFile);
  return true;
}

std::pair<bool, std::string> read_log_file(const std::string& logFileName) {
  // TODO: Also decrypt here
  auto ret = decompress_file_to_string(logFileName.c_str());
  if (!ret.first) {
    return ret;
  }
  util::decrypt_data(ret.second.data(), ret.second.size());
  return ret;
}

std::pair<bool, std::string> read_potentially_compressed_file(const std::string& fileName,
                                                              bool filePathProvided) {
  std::string fullFilePath = filePathProvided ? fileName : HOMEDIR + fileName;
  return decompress_file_to_string(fullFilePath.c_str());
}

bool get_file_from_device_common(const std::string& fileName, string& result,
                                 bool filePathProvided) {
  std::string fullFilePath = filePathProvided ? fileName : HOMEDIR + fileName;
  bool didGetFile = get_file_from_device(fullFilePath, result, true);
  return didGetFile;
}

bool get_unencrypted_file_from_device_common(const std::string& fileName, std::string& result,
                                             bool filePathProvided) {
  std::string fullFilePath = filePathProvided ? fileName : HOMEDIR + fileName;
  bool didGetFile = get_file_from_device(fullFilePath, result, false);
  return didGetFile;
}

// This will always overwrite the current file with given content
bool compress_and_save_file_on_device(const std::string& content, const std::string& fileName) {
  const auto fullFilePath = HOMEDIR + fileName;
  gzFile file = gzopen(fullFilePath.c_str(), "wb");
  int ret = gzwrite(file, content.c_str(), content.size());
  gzclose(file);
  if (ret <= 0) {
    LOG_TO_ERROR("Unable to compress and save file to device, gzwrite return: %d, err: %s", ret,
                 gzerror(file, &ret));
    return false;
  } else if (ret != content.size()) {
    LOG_TO_ERROR("File not fully written. Wrote %d bytes, had to write %d bytes", ret,
                 content.size());
    return false;
  }

  return true;
}

bool write_compressed_data_on_file(const std::string&& content, const std::string& fileName) {
  ofstream outFile(HOMEDIR + fileName, ios::out | ios::binary);
  outFile.write(content.c_str(), content.size());
  return true;
}

void write_data_to_file(const std::string&& content, const std::string& fileName,
                        bool fullFilePathProvided) {
  ofstream outFile(fullFilePathProvided ? fileName : HOMEDIR + fileName, ios::out);
  outFile.write(content.c_str(), content.size());
}

std::string save_file_on_device_common(std::string&& content, const std::string& fileName,
                                       bool overWrite) {
  return save_file_on_device(std::move(content), HOMEDIR + fileName, overWrite);
}

bool create_folder(const std::string& folderFullPath) {
  int ret = mkdir(folderFullPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
  if (ret != 0) {
    if (errno == EEXIST) {
      return true;
    } else {
      LOG_TO_ERROR("Could not create directory %s, errno: %d, error: %s", folderFullPath.c_str(),
                   errno, strerror(errno));
      return false;
    }
  }
  return true;
}

bool file_exists(const std::string& fileName) {
  std::ifstream inFile(fileName, ios::binary);
  if (inFile.fail()) return false;
  return true;
}

bool file_exists_common(const std::string& fileName) {
  auto fullfilePath = HOMEDIR + fileName;
  return file_exists(fullfilePath);
}

bool folder_exists(const std::string& folderPath) {
  struct stat info;
  if (stat(folderPath.c_str(), &info) != 0) return false;
  return info.st_mode & S_IFDIR;
}

bool folder_exists_common(const std::string& folderName, bool fullFilePathProvided) {
  auto fullFolderPath = fullFilePathProvided ? folderName : get_full_file_path_common(folderName);
  return folder_exists(fullFolderPath);
}

static inline int get_file_size(const std::string& fileName) {
  std::ifstream inFile(fileName, ios::binary);
  if (inFile.fail()) return 0;
  int initial = inFile.tellg();
  inFile.seekg(0, ios::end);
  return int(inFile.tellg()) - initial;
}

int get_file_size_common(const std::string& fileName) { return get_file_size(HOMEDIR + fileName); }

std::string get_full_file_path_common(const std::string& fileName) noexcept {
  return HOMEDIR + fileName;
}

#ifdef GENAI
bool unzip_archive(const std::string& fileName, const std::string& destinationFolderName) {
  const std::string fullFilePath = get_full_file_path_common(fileName);
  std::string fullDestinationPath = get_full_file_path_common(destinationFolderName);

  mz_zip_archive zip_archive;
  memset(&zip_archive, 0, sizeof(zip_archive));

  if (!mz_zip_reader_init_file(&zip_archive, fullFilePath.c_str(), 0)) {
    LOG_TO_CLIENT_ERROR("Failed to open zip file %s", fullFilePath.c_str());
    return false;
  }

  int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
  for (int i = 0; i < fileCount; ++i) {
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
      LOG_TO_CLIENT_ERROR("Failed to get stat for file present at index: %d inside file: %s", i,
                          fullFilePath.c_str());
      mz_zip_reader_end(&zip_archive);
      return false;
    }

    std::string fileDestinationPath = fullDestinationPath + "/" + file_stat.m_filename;

    if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
      LOG_TO_CLIENT_ERROR("%s", "Directory inside a zip archive not supported yet.");
      mz_zip_reader_end(&zip_archive);
      return false;
    } else {
      if (!create_folder(fullDestinationPath)) {
        LOG_TO_CLIENT_ERROR("Failed to create parent directory: %s", fullDestinationPath.c_str());
        mz_zip_reader_end(&zip_archive);
        return false;
      }

      if (!mz_zip_reader_extract_to_file(&zip_archive, i, fileDestinationPath.c_str(), 0)) {
        LOG_TO_CLIENT_ERROR("Failed to extract file: %s at %s", file_stat.m_filename,
                            fileDestinationPath.c_str());
        mz_zip_reader_end(&zip_archive);
        return false;
      }
    }
  }

  mz_zip_reader_end(&zip_archive);

  return true;
}

void initialize_os_llm() {
#ifdef GEMINI
  initializeGemini();
#endif  // GEMINI
}

void prompt_os_llm(const std::string& prompt) {
#ifdef GEMINI
  geminiNanoHandlerShadow.prompt(_threadLocalEnv, prompt);
#else
  THROW("OS LLM is not enabled in this build");
#endif  // GEMINI
}

void cancel_os_llm_query() {
#ifdef GEMINI
  geminiNanoHandlerShadow.cancel(_threadLocalEnv);
#else
  THROW("OS LLM is not enabled in this build");
#endif  // GEMINI
}

FileDownloadStatus check_os_llm_status() {
#ifdef GEMINI
  return getGeminiStatus();
#endif  // GEMINI
  return FileDownloadStatus::DOWNLOAD_FAILURE;
}

std::optional<std::string> get_os_supported_llm() {
#ifdef GEMINI
  initializeGemini();
  // Wait for 100ms to allow model load to fail if it is not available.
  // Status will be PENDING if model is supported and being downloaded
  // for 1st time or SUCCESS if model is already available.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (getGeminiStatus() != FileDownloadStatus::DOWNLOAD_FAILURE) {
    return rmconstants::GeminiModelName;
  }
#endif  // GEMINI
  return std::nullopt;
}
#endif  // GENAI

bool delete_file(const std::string& filePath, bool fullFilePathProvided) {
  const std::string fullFilePath =
      fullFilePathProvided ? filePath : get_full_file_path_common(filePath);
  if (remove(fullFilePath.c_str()) != 0) {
    LOG_TO_ERROR("Failed to delete file: %s with errorno: %d and error: %s", fullFilePath.c_str(),
                 errno, std::strerror(errno));
    return false;
  }
  return true;
}

void set_thread_priority_min() {
  if (!::set_thread_priority_min()) {
    LOG_TO_ERROR("%s", "Could not set background thread priority to min");
  }
}

void set_thread_priority_max() {
  if (!::set_thread_priority_max()) {
    LOG_TO_ERROR("%s", "Could not set background thread priority to max");
  }
}

bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char* workManagerConfigJsonChar) {
  return ::schedule_logs_upload(repeatIntervalInMinutes, retryIntervalInMinutesIfFailed,
                                workManagerConfigJsonChar);
}

void create_symlink(const fs::path& target, const std::string& link) {
  std::string targetStr = target.string();
  try {
    // Ignoring the return value, since we don't care if link existed or not
    static_cast<void>(fs::remove(link));
    fs::create_symlink(fs::absolute(target), link);
  } catch (const fs::filesystem_error& e) {
    THROW("Could not create symlink from %s to %s with error: %s", targetStr.c_str(), link.c_str(),
          e.what());
  } catch (...) {
    THROW("Could not create symlink from %s to %s", targetStr.c_str(), link.c_str());
  }
}

#ifdef IOS
const char* get_phonemes(const char* text) { return ::get_phonemes(text); }
#endif  // IOS
}  // namespace nativeinterface
