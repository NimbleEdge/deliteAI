/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "native_request_receiver_shadow.hpp"

#include <cstdarg>
#include <string>

#include "../../jni/utils/jni_logger.h"
#include "client.h"
#include "dependency_container_shadow.hpp"

void NativeRequestReceiverShadow::init(JNIEnv* env) {
  if (!env) return;

  jclass localClass = env->FindClass("dev/deliteai/impl/nativeBridge/NativeRequestReceiver");
  if (!localClass) {
    LOGE("Class dev.deliteai.impl.nativeBridge.NativeRequestReceiver not found.\n");
    return;
  }

  nativeRequestReceiverClass = static_cast<jclass>(env->NewGlobalRef(localClass));
  env->DeleteLocalRef(localClass);

  if (!nativeRequestReceiverClass) {
    LOGE("Failed to create global reference for NativeRequestReceiver class.\n");
    return;
  }

  dispatchMethodId =
      env->GetMethodID(nativeRequestReceiverClass, "dispatch",
                       "(Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;");
  if (!dispatchMethodId) {
    LOGE("Method dispatch not found.\n");
  }

  auto nativeRequestReceiverKotlinInstanceLocal = DependencyContainerShadow::getNativeRequestReceiverInstance(env);
  nativeRequestReceiverKotlinInstance = env->NewGlobalRef(nativeRequestReceiverKotlinInstanceLocal);
  env->DeleteLocalRef(nativeRequestReceiverKotlinInstanceLocal);
}

jobject NativeRequestReceiverShadow::dispatch(JNIEnv* env, const std::string& functionName, int argCount, ...) {
  if (!nativeRequestReceiverKotlinInstance) {
    LOGE("nativeRequestReceiverKotlinInstance is null in dispatch.\n");
    return nullptr;
  }

  jstring jFunctionName = env->NewStringUTF(functionName.c_str());

  // Create an array with the specified number of arguments
  jclass objectClass = env->FindClass("java/lang/Object");
  jobjectArray argsArray = env->NewObjectArray(argCount, objectClass, nullptr);
  env->DeleteLocalRef(objectClass);

  // Process variable arguments
  va_list args;
  va_start(args, argCount);
  for (int i = 0; i < argCount; i++) {
    jobject arg = va_arg(args, jobject);
    env->SetObjectArrayElement(argsArray, i, arg);
  }
  va_end(args);

  jobject result =
      env->CallObjectMethod(nativeRequestReceiverKotlinInstance, dispatchMethodId, jFunctionName, argsArray);

  env->DeleteLocalRef(jFunctionName);
  env->DeleteLocalRef(argsArray);

  return result;
}
