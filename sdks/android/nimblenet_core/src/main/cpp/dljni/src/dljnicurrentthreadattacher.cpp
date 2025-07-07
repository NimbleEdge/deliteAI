/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dljni/dljnicurrentthreadattacher.hpp"

DlJniCurrentThreadAttacher::DlJniCurrentThreadAttacher(JavaVM& jvm) : _jvm(jvm) {
  auto getEnvStatus = jvm.GetEnv((void**)&_env, JNI_VERSION_1_6);
  _wasAttachedAlready = (getEnvStatus != JNI_EDETACHED);

  if (!_wasAttachedAlready) {
    auto attachThreadStatus = jvm.AttachCurrentThread(&_env, nullptr);
    _gotAttached = (attachThreadStatus == JNI_OK);
  }
}

DlJniCurrentThreadAttacher::~DlJniCurrentThreadAttacher() {
  if (_gotAttached) {
    _jvm.DetachCurrentThread();
  }
}
