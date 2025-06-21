/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>

#include "client.h"

struct NetworkResponse {
 public:
  CNetworkResponse r;

  ~NetworkResponse() {
    free(r.headers);
    free(r.body);
  }

  NetworkResponse(const CNetworkResponse& cResponse) {
    r = cResponse;
    if (r.body == NULL) {
      r.body = (char*)malloc(1);
      r.body[0] = 0;
      r.headers = (char*)malloc(1);
      r.headers[0] = 0;
    }
  }

  NetworkResponse() {
    r.body = nullptr;
    r.headers = nullptr;
  }

  std::string c_str() {
    return std::string("statusCode=") + std::to_string(r.statusCode) +
           " bodyLen=" + std::to_string(r.bodyLength);
  }
};