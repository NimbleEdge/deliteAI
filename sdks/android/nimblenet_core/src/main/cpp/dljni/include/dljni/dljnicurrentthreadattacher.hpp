/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

class DlJniCurrentThreadAttacher {
  JavaVM& _jvm;
  JNIEnv* _env = nullptr;
  bool _wasAttachedAlready = false;
  bool _gotAttached = false;

 public:
  explicit DlJniCurrentThreadAttacher(JavaVM& jvm);

  ~DlJniCurrentThreadAttacher();

  [[nodiscard]] JNIEnv* getEnv() const noexcept { return _env; }

  [[maybe_unused, nodiscard]] bool wasAttachedAlready() const noexcept {
    return _wasAttachedAlready;
  }

  [[maybe_unused, nodiscard]] bool gotAttached() const noexcept { return _gotAttached; }

  [[maybe_unused, nodiscard]] bool notAttached() const noexcept {
    return !_wasAttachedAlready && !_gotAttached;
  }
};
