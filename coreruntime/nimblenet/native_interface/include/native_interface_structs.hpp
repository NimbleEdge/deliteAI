/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>

#include "client.h"

/**
 * @brief C++ wrapper around C-style `CNetworkResponse` struct used for HTTP response handling.
 *
 * This class manages dynamic memory for response headers and body received from a C HTTP client.
 * It provides basic RAII-style cleanup and safe default initialization.
 */
struct NetworkResponse {
 public:
  CNetworkResponse r; /**< Raw C-style response structure containing status, body, headers, etc. */

  /**
   * @brief Destructor to clean up dynamically allocated memory for headers and body.
   */
  ~NetworkResponse() {
    free(r.headers);
    free(r.body);
  }

  /**
   * @brief Constructor from a CNetworkResponse. Copies the struct and ensures
   * valid non-null pointers for body and headers.
   *
   * @param cResponse The raw C response to wrap.
   */
  NetworkResponse(const CNetworkResponse& cResponse) {
    r = cResponse;
    if (r.body == NULL) {
      // Ensure body is not null — allocate empty string
      r.body = (char*)malloc(1);
      r.body[0] = 0;
      r.headers = (char*)malloc(1);
      r.headers[0] = 0;
    }
  }

  /**
   * @brief Default constructor that initializes pointers to null.
   */
  NetworkResponse() {
    r.body = nullptr;
    r.headers = nullptr;
  }

  /**
   * @brief Get a string summary of the response (status code and body length).
   *
   * @return A human-readable summary string.
   */
  std::string c_str() {
    return std::string("statusCode=") + std::to_string(r.statusCode) +
           " bodyLen=" + std::to_string(r.bodyLength);
  }
};