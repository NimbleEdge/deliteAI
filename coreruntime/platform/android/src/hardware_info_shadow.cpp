/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "hardware_info_shadow.hpp"

#include "../../jni/utils/jni_logger.h"
#include "client.h"

void HardwareInfoShadow::init(JNIEnv* env) {
  if (!env) return;
  jclass localClass = env->FindClass("ai/nimbleedge/impl/common/HardwareInfo");
  if (!localClass) {
    LOGE("Class ai.nimbleedge.io.HardwareInfo not found.\n");
    return;
  }
  hardwareInfoClass = static_cast<jclass>(env->NewGlobalRef(localClass));
  env->DeleteLocalRef(localClass);
  if (!hardwareInfoClass) {
    LOGE("Failed to create global reference for HardwareInfo class.\n");
    return;
  }
  getStaticDeviceMetricsMethodId =
      env->GetMethodID(hardwareInfoClass, "getStaticDeviceMetrics", "()Ljava/lang/String;");
  if (!getStaticDeviceMetricsMethodId) {
    LOGE("Method getStaticDeviceMetrics not found.\n");
  }

  auto localInstance = DependencyContainerShadow::getHardwareInfoInstance(env);

  hardwareInfoKotlinInstance = env->NewGlobalRef(localInstance);
  env->DeleteLocalRef(localInstance);
}

jstring HardwareInfoShadow::getStaticDeviceMetrics(JNIEnv* env) {
  if (!hardwareInfoKotlinInstance || !getStaticDeviceMetricsMethodId) {
    throw std::runtime_error("HardwareInfoShadow not initialized");
  }
  return static_cast<jstring>(
      env->CallObjectMethod(hardwareInfoKotlinInstance, getStaticDeviceMetricsMethodId));
}
