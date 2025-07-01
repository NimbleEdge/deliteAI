/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dependency_container_shadow.hpp"

#include "../../jni/utils/jni_logger.h"
#include "client.h"

void DependencyContainerShadow::setMethodIds(JNIEnv *env) {
  getInstanceMethodId =
      env->GetStaticMethodID(dependencyContainerClass, "getInstance",
                             "(Landroid/app/Application;Lai/nimbleedge/datamodels/"
                             "NimbleNetConfig;)Lai/nimbleedge/impl/DependencyContainer;");
  getNetworkingMethodId = env->GetMethodID(dependencyContainerClass, "getNetworking",
                                           "()Lai/nimbleedge/impl/io/Networking;");
  getHardwareInfoMethodId = env->GetMethodID(dependencyContainerClass, "getHardwareInfo",
                                             "()Lai/nimbleedge/impl/common/HardwareInfo;");
  getLogsUploadSchedulerMethodId = env->GetMethodID(dependencyContainerClass, "getLogsUploadScheduler",
                                                 "()Lai/nimbleedge/impl/loggers/workManager/LogsUploadScheduler;");
}

void DependencyContainerShadow::setDependencyContainerInstance(JNIEnv *env) {
  if (!dependencyContainerClass || !getInstanceMethodId) {
    throw std::runtime_error("encountered nullptr in setDependencyContainerInstance()");
  }
  jobject local = env->CallStaticObjectMethod(dependencyContainerClass, getInstanceMethodId,
                                              (jobject) nullptr, (jobject) nullptr);
  dependencyContainerInstance = env->NewGlobalRef(local);
  env->DeleteLocalRef(local);
}

void DependencyContainerShadow::init(JNIEnv *env) {
  if (!env) return;
  jclass localClass = env->FindClass("ai/nimbleedge/impl/DependencyContainer");
  if (!localClass) {
    LOGE("Class ai.nimbleedge.scriptWrappers.GeminiNanoHandler not found.\n");
    return;
  }
  dependencyContainerClass = static_cast<jclass>(env->NewGlobalRef(localClass));
  env->DeleteLocalRef(localClass);
  if (!dependencyContainerClass) {
    LOGE("Failed to create global reference for DependencyContainerClass class.\n");
    return;
  }
  setMethodIds(env);
  setDependencyContainerInstance(env);
}

jobject DependencyContainerShadow::getNetworkingInstance(JNIEnv *env) {
  if (!getNetworkingMethodId || !dependencyContainerClass) {
    throw std::runtime_error("encountered nullptr in getNetworkingInstance()");
  }
  if (!dependencyContainerInstance) {
    throw std::runtime_error("dependencyContainerClass is not initialized");
  }

  return env->CallObjectMethod(dependencyContainerInstance, getNetworkingMethodId);
}

jobject DependencyContainerShadow::getHardwareInfoInstance(JNIEnv *env) {
  if (!getHardwareInfoMethodId || !dependencyContainerClass) {
    throw std::runtime_error("encountered nullptr in getHardwareInfoInstance()");
  }
  if (!dependencyContainerInstance) {
    throw std::runtime_error("dependencyContainerClass is not initialized");
  }

  return env->CallObjectMethod(dependencyContainerInstance, getHardwareInfoMethodId);
}

jobject DependencyContainerShadow::getLogsUploadSchedulerInstance(JNIEnv *env) {
  if (!getLogsUploadSchedulerMethodId || !dependencyContainerClass) {
    throw std::runtime_error("encountered nullptr in getLogsUploadSchedulerShadow()");
  }
  if (!dependencyContainerInstance) {
    throw std::runtime_error("dependencyContainerClass is not initialized");
  }

  return env->CallObjectMethod(dependencyContainerInstance, getLogsUploadSchedulerMethodId);
}
