/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "logs_upload_scheduler_shadow.hpp"

#include "dependency_container_shadow.hpp"

void LogsUploadSchedulerShadow::init(JNIEnv* env) {
  if (env == nullptr) return;

  jclass localCls = env->FindClass("ai/nimbleedge/impl/loggers/workManager/LogsUploadScheduler");
  if (localCls == nullptr) {
    LOGE("Class ai.nimbleedge.schedulers.LogsUploadScheduler not found.\n");
    return;
  }

  logsUploadSchedulerClass = static_cast<jclass>(env->NewGlobalRef(localCls));
  env->DeleteLocalRef(localCls);

  if (logsUploadSchedulerClass == nullptr) {
    LOGE("Failed to create global ref for LogsUploadScheduler class.\n");
    return;
  }

  scheduleMethodId =
      env->GetMethodID(logsUploadSchedulerClass, "schedule", "(JJLjava/lang/String;)V");

  if (scheduleMethodId == nullptr) {
    LOGE("Method schedule(...) not found on LogsUploadScheduler.\n");
  }

  auto localInstance = DependencyContainerShadow::getLogsUploadSchedulerInstance(env);

  logsUploadSchedulerKotlinInstance = env->NewGlobalRef(localInstance);
  env->DeleteLocalRef(localInstance);
}

void LogsUploadSchedulerShadow::schedule(JNIEnv* env, jobject application,
                                         jlong initialDelayInMinutes,
                                         jlong retryIntervalInMinutesIfFailed,
                                         const std::string& payload) {
  if (!env || !logsUploadSchedulerClass || !scheduleMethodId) {
    throw std::runtime_error("Invalid state to call LogsUploadScheduler.schedule");
  }

  jstring jPayload = env->NewStringUTF(payload.c_str());
  env->CallVoidMethod(logsUploadSchedulerKotlinInstance, scheduleMethodId, initialDelayInMinutes * 60,
                      retryIntervalInMinutesIfFailed * 60, jPayload);
  env->DeleteLocalRef(jPayload);

  if (env->ExceptionCheck()) {
    env->ExceptionDescribe();
    env->ExceptionClear();
    LOGE("Exception thrown during LogsUploadScheduler.schedule call.\n");
  }
}
