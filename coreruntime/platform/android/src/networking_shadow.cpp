/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "networking_shadow.hpp"

#include "../../jni/utils/jni_logger.h"
#include "client.h"

void NetworkingShadow::init(JNIEnv* env) {
  if (!env) return;

  jclass localClass = env->FindClass("dev/deliteai/impl/io/Networking");
  if (!localClass) {
    LOGE("Class dev.deliteai.io.Networking not found.\n");
    return;
  }

  networkingClass = static_cast<jclass>(env->NewGlobalRef(localClass));
  env->DeleteLocalRef(localClass);

  if (!networkingClass) {
    LOGE("Failed to create global reference for Networking class.\n");
    return;
  }

  sendRequestMethodId =
      env->GetMethodID(networkingClass, "sendRequest",
                       "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;I)Ldev/deliteai/impl/io/datamodels/NetworkResponse;");
  if (!sendRequestMethodId) {
    LOGE("Method sendRequest not found.\n");
  }

  downloadFileMethodId =
      env->GetMethodID(networkingClass, "downloadFileThroughDownloadManager",
                       "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ldev/deliteai/impl/io/datamodels/FileDownloadStateTransition;");
  if (!downloadFileMethodId) {
    LOGE("Method downloadFileThroughDownloadManager not found.\n");
  }

  auto networkingKotlinInstanceLocal = DependencyContainerShadow::getNetworkingInstance(env);
  networkingKotlinInstance = env->NewGlobalRef(networkingKotlinInstanceLocal);
  env->DeleteLocalRef(networkingKotlinInstanceLocal);
}

jobject NetworkingShadow::sendRequest(JNIEnv* env, const std::string& url,
                                      const std::string& requestHeaders,
                                      const std::string& requestBody, jbyteArray requestBodyByte,
                                      const std::string& method, jint totalCallTimeoutInSecs) {
  if (!networkingKotlinInstance) {
    LOGE("networkingObj is null in sendRequest.\n");
    return nullptr;
  }

  jstring jUrl = env->NewStringUTF(url.c_str());
  jstring jHeaders = env->NewStringUTF(requestHeaders.c_str());
  jstring jBody = env->NewStringUTF(requestBody.c_str());
  jstring jMethod = env->NewStringUTF(method.c_str());

  jobject result =
      env->CallObjectMethod(networkingKotlinInstance, sendRequestMethodId, jUrl, jHeaders, jBody,
                            requestBodyByte, jMethod, totalCallTimeoutInSecs);

  env->DeleteLocalRef(jUrl);
  env->DeleteLocalRef(jHeaders);
  env->DeleteLocalRef(jBody);
  env->DeleteLocalRef(jMethod);

  return result;
}

jobject NetworkingShadow::downloadFileThroughDownloadManager(JNIEnv* env, const std::string& url,
                                                             const std::string& requestHeaders,
                                                             const std::string& fileName,
                                                             const std::string& nimbleSdkDir) {
  if (!networkingKotlinInstance) {
    LOGE("networkingObj is null in downloadFileThroughDownloadManager.\n");
    return nullptr;
  }

  jstring jUrl = env->NewStringUTF(url.c_str());
  jstring jHeaders = env->NewStringUTF(requestHeaders.c_str());
  jstring jFileName = env->NewStringUTF(fileName.c_str());
  jstring jDir = env->NewStringUTF(nimbleSdkDir.c_str());

  jobject result = env->CallObjectMethod(networkingKotlinInstance, downloadFileMethodId, jUrl,
                                         jHeaders, jFileName, jDir);

  env->DeleteLocalRef(jUrl);
  env->DeleteLocalRef(jHeaders);
  env->DeleteLocalRef(jFileName);
  env->DeleteLocalRef(jDir);

  return result;
}
