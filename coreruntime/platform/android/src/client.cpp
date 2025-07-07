/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client.h"

#include <string>

#include "file_download_state_transition_shadow.h"
#include "hardware_info_shadow.hpp"
#include "logs_upload_scheduler_shadow.hpp"
#include "nimble_net_util.hpp"

JavaVM *globalJvm;

thread_local JNIEnv *_threadLocalEnv = nullptr;

jobject context;

jmethodID scheduleLogsUploadFunction;
jclass LogsUploadSchedulerClazz;

#ifdef GEMINI
void initializeGemini() {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return;
    }
  }

  geminiNanoHandlerShadow.initialize(env, context);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }
}

FileDownloadStatus getGeminiStatus() {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return FileDownloadStatus::DOWNLOAD_FAILURE;
    }
  }

  FileDownloadStatus status = geminiNanoHandlerShadow.getStatus(env);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }

  return status;
}
#endif  // GEMINI

CNetworkResponse send_request(const char *body, const char *headers, const char *url,
                              const char *method, int length) {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return emptyResponse();
    }
  }

  jint sendRequestCallTimeout = 30;
  jbyteArray jbodybyte = nullptr;

  if (length != -1) {
    auto bodyLength = static_cast<jsize>(strlen(body));

    jbodybyte = env->NewByteArray(bodyLength);
    env->SetByteArrayRegion(jbodybyte, 0, bodyLength, reinterpret_cast<const jbyte *>(body));
  }

  jobject result = NetworkingShadow::sendRequest(env, url, headers, body, jbodybyte, method,
                                                 sendRequestCallTimeout);

  jclass resultClass = env->GetObjectClass(result);

  CNetworkResponse response{};

  response.statusCode =
      (int)env->GetIntField(result, env->GetFieldID(resultClass, "statusCode", "I"));

  auto responseHeaders = (jstring)env->GetObjectField(
      result, env->GetFieldID(resultClass, "headers", "Ljava/lang/String;"));

  char *tempHeaders = const_cast<char *>(env->GetStringUTFChars(responseHeaders, nullptr));
  auto headersLen = strlen(tempHeaders);
  response.headers = (char *)malloc(headersLen + 1);
  memcpy(response.headers, tempHeaders, headersLen);
  response.headers[headersLen] = '\0';
  env->ReleaseStringUTFChars(responseHeaders, tempHeaders);

  auto responseBody =
      (jbyteArray)env->GetObjectField(result, env->GetFieldID(resultClass, "body", "[B"));
  char *temp = (char *)(env->GetByteArrayElements(responseBody, nullptr));
  response.bodyLength =
      (int)env->GetIntField(result, env->GetFieldID(resultClass, "bodyLength", "I"));
  response.body = (char *)malloc(response.bodyLength + 1);
  memcpy(response.body, temp, response.bodyLength);
  response.body[response.bodyLength] = '\0';
  env->ReleaseByteArrayElements(responseBody, (jbyte *)temp, 0);
  if (attached) {
    globalJvm->DetachCurrentThread();
  }
  return response;
}

FileDownloadInfo download_to_file_async(const char *url, const char *headers, const char *fileName,
                                        const char *nimbleSdkDir) {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      FileDownloadInfo fileDownloadInfo = emptyFileDownloadInfo();
      fileDownloadInfo.currentStatus = DOWNLOAD_FAILURE;
      fileDownloadInfo.timeElapsedInMicro = -1;

      return fileDownloadInfo;
    }
  }

  auto fileDownloadStateTransitionObject = NetworkingShadow::downloadFileThroughDownloadManager(
      env, url, headers, fileName, nimbleSdkDir);

  auto fileDownloadStateTransitionShadow =
      FileDownloadStateTransitionShadow(env, fileDownloadStateTransitionObject);

  FileDownloadInfo fileDownloadInfo = emptyFileDownloadInfo();
  fileDownloadInfo.requestId = fileDownloadStateTransitionShadow.getDownloadManagerDownloadId(env);
  fileDownloadInfo.prevStatus =
      static_cast<FileDownloadStatus>(fileDownloadStateTransitionShadow.getPreviousState(env));
  fileDownloadInfo.currentStatus =
      static_cast<FileDownloadStatus>(fileDownloadStateTransitionShadow.getCurrentState(env));
  fileDownloadInfo.timeElapsedInMicro = fileDownloadStateTransitionShadow.getTimeTaken(env) * 1000;
  fileDownloadInfo.currentStatusReasonCode =
      fileDownloadStateTransitionShadow.getCurrentStateReasonCode(env);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }

  return fileDownloadInfo;
}

CNetworkResponse emptyResponse() {
  CNetworkResponse response{};
  response.statusCode = EMPTY_ERROR_CODE;
  response.headers = nullptr;
  response.body = nullptr;
  return response;
}

FileDownloadInfo emptyFileDownloadInfo() {
  FileDownloadInfo fileDownloadInfo{};
  fileDownloadInfo.currentStatus = static_cast<FileDownloadStatus>(DOWNLOAD_UNKNOWN);
  fileDownloadInfo.prevStatus = static_cast<FileDownloadStatus>(DOWNLOAD_UNKNOWN);
  fileDownloadInfo.timeElapsedInMicro = -1;
  return fileDownloadInfo;
}

char *get_hardware_info() {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);
  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return nullptr;
    }
  }

  jobject result = HardwareInfoShadow::getStaticDeviceMetrics(env);

  const char *str = env->GetStringUTFChars((jstring)result, NULL);
  if (!str) return nullptr;

  char *strCopy = strdup(str);
  env->ReleaseStringUTFChars((jstring)result, str);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }

  return strCopy;
}

bool set_thread_priority_max() {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return false;
    }
  }

  jclass jcProcess = env->FindClass("android/os/Process");

  jmethodID jmSetThreadPriority = env->GetStaticMethodID(jcProcess, "setThreadPriority", "(I)V");
  env->CallStaticVoidMethod(jcProcess, jmSetThreadPriority, (jint)-1);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }
  return true;
}

// Setting values from
// https://developer.android.com/reference/android/os/Process#THREAD_PRIORITY_LESS_FAVORABLE
bool set_thread_priority_min() {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;

    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return false;
    }
  }

  jclass jcProcess = env->FindClass("android/os/Process");

  jmethodID jmSetThreadPriority = env->GetStaticMethodID(jcProcess, "setThreadPriority", "(I)V");
  env->CallStaticVoidMethod(jcProcess, jmSetThreadPriority, (jint)1);
  if (attached) {
    globalJvm->DetachCurrentThread();
  }
  return true;
}

bool schedule_logs_upload(long repeatIntervalInMinutes, long retryIntervalInMinutesIfFailed,
                          const char *workManagerConfigJsonChar) {
  JNIEnv *env;
  bool attached = false;
  int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);

  if (getEnvStatus == JNI_EDETACHED) {
    attached = true;
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name = NULL;
    args.group = NULL;
    if (globalJvm->AttachCurrentThread(&env, NULL) != 0) {
      return false;
    }
  }

  LogsUploadSchedulerShadow::schedule(env, context, repeatIntervalInMinutes,
                                      retryIntervalInMinutesIfFailed, workManagerConfigJsonChar);

  if (attached) {
    globalJvm->DetachCurrentThread();
  }
  return true;
}

bool deallocate_frontend_tensors(CTensors cTensors) { return true; }

bool free_frontend_function_context(void *context) { return true; }
