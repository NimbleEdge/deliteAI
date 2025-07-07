/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client.h"

#include <curl/curl.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "logger.hpp"
#include "time_manager.hpp"

struct stringy {
  char *ptr;
  size_t len;
};

void init_string(struct stringy *s) {
  s->len = 0;
  s->ptr = (char *)malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct stringy *s) {
  size_t new_len = s->len + size * nmemb;
  s->ptr = (char *)realloc(s->ptr, new_len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
  return size * nmemb;
}

size_t readHeader(char *header, size_t size, size_t nitems, void *userData) {
  nlohmann::json *jsonData = (nlohmann::json *)userData;

  std::string headerString(header, size * nitems);

  auto location = headerString.find(':');
  if (location == std::string::npos) return size * nitems;
  std::string stringKey = headerString.substr(0, location);
  std::string stringvalue = headerString.substr(location + 2, headerString.size() - 1);
  // location + 2 to remove ": "
  stringvalue.erase(std::remove_if(stringvalue.begin(), stringvalue.end(),
                                   [](auto ch) { return (ch == '\n' || ch == '\r'); }),
                    stringvalue.end());

  (*jsonData)[stringKey] = stringvalue;

  return size * nitems;
}

std::string decompress_string(const std::string &compressed_string) {
  std::vector<char> decompressed_data;
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  size_t CHUNK_SIZE = 16384;

  // Initialize decompression
  if (inflateInit(&strm) != Z_OK) {
    throw std::runtime_error("Failed to initialize decompression");
  }

  strm.avail_in = compressed_string.size();
  strm.next_in = (Bytef *)compressed_string.data();

  do {
    decompressed_data.resize(decompressed_data.size() + CHUNK_SIZE);
    strm.avail_out = CHUNK_SIZE;
    strm.next_out = (Bytef *)decompressed_data.data() + decompressed_data.size() - CHUNK_SIZE;

    int ret = inflate(&strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
      break;
    }
    if (ret != Z_OK) {
      inflateEnd(&strm);
      throw std::runtime_error("Decompression error");
    }
  } while (strm.avail_out == 0);

  inflateEnd(&strm);

  // Resize to actual decompressed size
  decompressed_data.resize(decompressed_data.size() - strm.avail_out);

  return std::string(decompressed_data.begin(), decompressed_data.end());
}

CNetworkResponse send_request(const char *body, const char *headers_c, const char *url,
                              const char *method, int length) {
  CURL *curl;
  CURLcode res;
  struct curl_slist *headers = NULL;
  // try catch since empty headers leads to json errors and we send empty headers on Register calls
  try {
    nlohmann::json jsonheaders = nlohmann::json::parse(headers_c);
    for (auto &headerList : jsonheaders) {
      for (auto &header : headerList.items()) {
        std::string hval = header.key() + ": " + std::string(headerList[header.key()]);
        headers = curl_slist_append(headers, hval.c_str());
      }
    }
  } catch (std::out_of_range &e) {
    std::cout << "out of range: " << e.what() << std::endl;
  } catch (std::invalid_argument &e) {
    std::cout << "invalid" << std::endl;
  } catch (std::exception &e) {
    std::cout << "Exception :" << e.what() << std::endl;
  }

  curl = curl_easy_init();
  std::string URL = url;

  CNetworkResponse response;
  response.statusCode = EMPTY_ERROR_CODE;
  response.body = nullptr;
  response.bodyLength = 0;
  response.headers = nullptr;
  if (!curl) abort();

  struct stringy s;
  init_string(&s);
  nlohmann::json headerJson;
  curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING,
                   "");  // This accepts all formats of encoding supported by curl

  // Evaluates to 0 for GET requests and body is not set for GET calls
  if (strcmp(method, "GET") == 0) {
  } else if (strcmp(method, "POST") == 0) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)length);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
  } else if (strcmp(method, "PUT") == 0) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)length);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
  } else {
    throw std::runtime_error(std::string(method) + " method not supported in client.cpp.");
  }
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, readHeader);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerJson);
// When building whl file for linux inside docker image quay.io/pypa/manylinux_2_28_x86_64 and
// running the simulator outside the docker container, curl fails to read SSL certificates giving
// error response code 77. Disabling the verification should not be only solution and need to
// figure out a better solution for this.
#ifdef SIMULATION_MODE
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
  char buffer[CURL_ERROR_SIZE + 1] = {};
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, buffer);
  res = curl_easy_perform(curl);

  // Cleanup objects used to perform request
  curl_slist_free_all(headers);

  if (res != CURLE_OK) {
    LOG_VERBOSE("Failed to do curl_easy_perform to url %s: %s", url, curl_easy_strerror(res));
    response.statusCode = EMPTY_ERROR_CODE;
    response.body = nullptr;
    response.headers = nullptr;
    response.bodyLength = 0;
    return response;
  }
  char *headerPointer;
  std::string headerJsonDump = headerJson.dump();
  asprintf(&headerPointer, "%s", headerJsonDump.c_str());
  long http_code = 0;
  res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (res != CURLE_OK) {
    LOG_VERBOSE(
        "Got response from request to URL %s, body %s, method %s: response body %s, status code %d",
        url, body, method, s.ptr, http_code);
    response.statusCode = EMPTY_ERROR_CODE;
    response.body = nullptr;
    response.headers = nullptr;
    response.bodyLength = 0;
    curl_easy_cleanup(curl);
    return response;
  }

  response.statusCode = http_code;
  response.body = s.ptr;
  response.bodyLength = s.len;
  response.headers = headerPointer;
  
#ifndef NDEBUG
  printf("%s status_code=%ld\n", buffer, http_code);
#endif
  curl_easy_cleanup(curl);
  return response;
}

int decompress_one_file(const char *infilename, const char *outfilename) {
  gzFile infile = gzopen(infilename, "rb");
  FILE *outfile = fopen(outfilename, "wb");
  if (infile == Z_NULL || !outfile) {
    printf("could not open gzFile=%s", infilename);
    return -1;
  }

  char buffer[128];
  int num_read = 0;
  while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
    fwrite(buffer, 1, num_read, outfile);
  }

  gzclose(infile);
  fclose(outfile);
  return 0;
}

FileDownloadInfo download_to_file_async(const char *url, const char *headers, const char *filePath,
                                        const char *nimbleSdkDir) {
  auto startTime = Time::get_high_resolution_clock_time();
  CNetworkResponse r = send_request("", headers, url, "GET", 0);
  int timeElapsed = Time::get_elapsed_time_in_micro(startTime);
  LOG_VERBOSE("Async download to url %s: response body %s, status %d", url, r.body, r.statusCode);
  FileDownloadInfo fileDownloadInfo;
  if (r.statusCode != 200) {
    fileDownloadInfo.currentStatus = DOWNLOAD_FAILURE;
    fileDownloadInfo.timeElapsedInMicro = -1;
  } else {
    std::string dataBuffer = std::string(r.body, r.bodyLength);
    std::string fullPath = std::string(nimbleSdkDir) + filePath;

    {
      std::ofstream outFile(fullPath, std::ios::out | std::ios::binary);
      outFile.write(dataBuffer.c_str(), dataBuffer.size());
    }

    fileDownloadInfo.currentStatus = DOWNLOAD_SUCCESS;
    fileDownloadInfo.timeElapsedInMicro = timeElapsed;
  }
  return fileDownloadInfo;
}

bool deallocate_frontend_tensors(CTensors cTensors) {
  if (globalDeallocate) {
    return globalDeallocate(cTensors);
  }
  return false;
}

bool free_frontend_function_context(void *context) {
  if (globalFrontendContextFree) {
    return globalFrontendContextFree(context);
  }
  return false;
}
