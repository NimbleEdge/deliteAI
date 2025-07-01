/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <jni.h>
#include "nimblenet.hpp"

#pragma GCC visibility push(default)

extern "C"
JNIEXPORT void JNICALL
Java_ai_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_reset(JNIEnv *env, jobject thiz) {
    nimblenetInternal::reset();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_deleteDatabase(JNIEnv *env, jobject thiz) {
    nimblenetInternal::delete_database();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_ai_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_reloadModelWithEpConfig(
        JNIEnv *env, jobject thiz, jstring jmodel_name, jstring jep_config) {
    const char *model_name = env->GetStringUTFChars(jmodel_name, nullptr);
    const char *ep_config = env->GetStringUTFChars(jep_config, nullptr);

    auto res = nimblenetInternal::reload_model_with_epConfig(model_name, ep_config);

    env->ReleaseStringUTFChars(jmodel_name, model_name);
    env->ReleaseStringUTFChars(jep_config, ep_config);

    return res;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_ai_deliteai_impl_nativeBridge_impl_CoreRuntimeImpl_loadModelFromFile(JNIEnv *env, jobject thiz,
                                                                 jstring jmodel_file_path,
                                                                 jstring jinference_config_file_path,
                                                                 jstring jmodel_id, jstring jepConfigJsonChar) {
    const char *model_file_path = env->GetStringUTFChars(jmodel_file_path, nullptr);
    const char *inference_config_file_path = env->GetStringUTFChars(jinference_config_file_path, nullptr);
    const char *model_id = env->GetStringUTFChars(jmodel_id, nullptr);
    const char *epConfigJsonChar = env->GetStringUTFChars(jepConfigJsonChar, nullptr);

    auto res = nimblenetInternal::load_model_from_file(model_file_path,inference_config_file_path,model_id,epConfigJsonChar);

    env->ReleaseStringUTFChars(jmodel_file_path, model_file_path);
    env->ReleaseStringUTFChars(jinference_config_file_path, inference_config_file_path);
    env->ReleaseStringUTFChars(jmodel_id, model_id);
    env->ReleaseStringUTFChars(jepConfigJsonChar, epConfigJsonChar);

    return res;
}
