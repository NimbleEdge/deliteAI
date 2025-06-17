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

/**
 * @brief A simple string wrapper that manages dynamic memory allocation
 *
 * This class provides RAII-style memory management for formatted strings.
 * It automatically allocates memory on construction and deallocates on destruction.
 */
struct FmtString {
  char* str = nullptr;

  /**
   * @brief Constructs a FmtString with the specified size
   * @param size The size of the string buffer to allocate
   */
  FmtString(std::size_t size) { str = new char[size]; }

  /**
   * @brief Destructor that frees the allocated memory
   */
  ~FmtString() { delete[] str; }
};

/**
 * @brief Macro for handling variadic formatting
 * 
 * @param format The format string
 */
#define NE_VARIADIC_FMT(format)      \
  va_list args;                      \
  va_start(args, format);            \
  auto buf = ne::vfmt(format, args); \
  va_end(args)

/**
 * @brief Formats a string using a va_list
 * 
 * @param format The format string
 * @param args The va_list containing the arguments
 * @return FmtString containing the formatted result
 * @throw std::runtime_error if formatting fails
 */
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

/**
 * @brief Formats a string using variadic arguments
 * 
 * @param format The format string
 * @param ... Variable arguments to be formatted
 * @return FmtString containing the formatted result
 */
static inline FmtString fmt(const char* format, ...) {
  va_list args;

  va_start(args, format);
  auto ret = vfmt(format, args);
  va_end(args);
  return ret;
}

/**
 * @brief Formats a string and returns a raw char pointer
 * 
 * @param format The format string
 * @param ... Variable arguments to be formatted
 * @return char* containing the formatted result (must be freed by caller)
 * @throw std::runtime_error if formatting fails
 */
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

/**
 * @brief Macro for throwing formatted runtime errors
 * 
 * @param format The format string
 * @param ... Variable arguments to be formatted
 */
#define THROW(format, ...)                                   \
  do {                                                       \
    auto throwBuffer = ne::fmt(format, ##__VA_ARGS__);       \
    auto runTIMEERROR = std::runtime_error(throwBuffer.str); \
    throw runTIMEERROR;                                      \
  } while (0)
