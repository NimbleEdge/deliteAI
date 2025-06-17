/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdarg>
#include <cstdio>
#include <stdexcept>

namespace ne {

struct FmtString {
  char* str = nullptr;

  FmtString(std::size_t size) { str = new char[size]; }

  ~FmtString() { delete[] str; }
};

#define NE_VARIADIC_FMT(format)      \
  va_list args;                      \
  va_start(args, format);            \
  auto buf = ne::vfmt(format, args); \
  va_end(args)

static inline FmtString vfmt(const char* format, va_list args) {
  // va_start(args) has already been called
  va_list args2;
  va_copy(args2, args);

  int size_s = std::vsnprintf(nullptr, 0, format, args) + 1;  // Extra space for '\0'
  if (size_s <= 0) {
    va_end(args2);
    throw std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  FmtString fmtStr{size};

  std::vsnprintf(fmtStr.str, size, format, args2);
  va_end(args2);
  return fmtStr;
}

static inline FmtString fmt(const char* format, ...) {
  va_list args;

  va_start(args, format);
  auto ret = vfmt(format, args);
  va_end(args);
  return ret;
}

static inline char* fmt_to_raw(const char* format, ...) {
  va_list args, args2;
  va_start(args, format);
  va_copy(args2, args);

  int size_s = std::vsnprintf(nullptr, 0, format, args) + 1;  // Extra space for '\0'
  va_end(args);

  if (size_s <= 0) {
    va_end(args2);
    throw std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  char* str = (char*)malloc(size * sizeof(char));

  std::vsnprintf(str, size, format, args2);
  va_end(args2);

  return str;
}

}  // namespace ne

#define THROW(format, ...)                                   \
  do {                                                       \
    auto throwBuffer = ne::fmt(format, ##__VA_ARGS__);       \
    auto runTIMEERROR = std::runtime_error(throwBuffer.str); \
    throw runTIMEERROR;                                      \
  } while (0)
