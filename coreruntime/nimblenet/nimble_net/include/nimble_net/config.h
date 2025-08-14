/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if defined(__ANDROID__)
  #define DELITEAI_TARGET_OS_ANDROID 1
#elif defined(__APPLE__)
  #include <TargetConditionals.h>

  #if TARGET_OS_IPHONE
    #if TARGET_OS_IOS
      #define DELITEAI_TARGET_OS_IOS 1
      #if TARGET_OS_SIMULATOR
        #define DELITEAI_TARGET_OS_IOS_SIMULATOR 1
      #endif  // TARGET_OS_SIMULATOR
    #else
      #error "Unsupported Apple platform"
    #endif  // TARGET_OS_IOS
  #elif TARGET_OS_OSX
    #define DELITEAI_TARGET_OS_MACOS 1
  #else
    #error "Unsupported Apple platform"
  #endif
#elif defined(__linux__)
  #define DELITEAI_TARGET_OS_LINUX 1
#else
  #error "Unsupported platform"
#endif

// Make sure all the DELITEAI_TARGET_OS_* values are defined.

#ifndef DELITEAI_TARGET_OS_ANDROID
  #define DELITEAI_TARGET_OS_ANDROID 0
#endif  // DELITEAI_TARGET_OS_ANDROID

#ifndef DELITEAI_TARGET_OS_IOS
  #define DELITEAI_TARGET_OS_IOS 0
#endif  // DELITEAI_TARGET_OS_IOS

#ifndef DELITEAI_TARGET_OS_IOS_SIMULATOR
  #define DELITEAI_TARGET_OS_IOS_SIMULATOR 0
#endif  // DELITEAI_TARGET_OS_IOS_SIMULATOR

#ifndef DELITEAI_TARGET_OS_MACOS
  #define DELITEAI_TARGET_OS_MACOS 0
#endif  // DELITEAI_TARGET_OS_MACOS

#ifndef DELITEAI_TARGET_OS_LINUX
  #define DELITEAI_TARGET_OS_LINUX 0
#endif  // DELITEAI_TARGET_OS_LINUX

// =================================================================================================

#ifdef __cplusplus
  #define EXTERN_C          extern "C"
  #define EXTERN_C_BEGIN    extern "C" {
  #define EXTERN_C_END      }  // extern "C"
#else
  #define EXTERN_C
  #define EXTERN_C_BEGIN
  #define EXTERN_C_END
#endif  // __cplusplus
