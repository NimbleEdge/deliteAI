/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_store.hpp"

template <>
char* Store<StoreType::LOGS>::format(const char* type, const char* timestamp, const char* log) {
  return ne::fmt_to_raw("%s::: %s ::: %s\n", type, timestamp, log);
}

template <>
char* Store<StoreType::METRICS>::format(const char* type, const char* timestamp, const char* log) {
  return ne::fmt_to_raw("METRICS::: %s ::: %s ::: %s\n", timestamp, type, log);
}
