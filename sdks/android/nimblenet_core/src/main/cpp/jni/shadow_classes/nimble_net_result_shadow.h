/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <cstdio>
#include <stdexcept>
#include <string>

#include "../utils/jni_logger.h"

class NimbleNetResultShadow {
private:
    jclass resultClass;
    jmethodID constructorMethodId;

    jfieldID statusFieldId;
    jfieldID payloadFieldId;
    jfieldID errorFieldId;

public:
 NimbleNetResultShadow(JNIEnv *env) {
        if (env == nullptr) return;

        jclass localClass = env->FindClass("dev/deliteai/datamodels/NimbleNetResult");
        if (localClass == nullptr) {
            LOGE("Class dev.deliteai.datamodels.NimbleNetResult not found.\n");
            return;
        }
        resultClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
        if (resultClass == nullptr) {
            LOGE("Failed to create global reference for NimbleNetResult class.\n");
            return;
        }

        constructorMethodId = env->GetMethodID(resultClass, "<init>", "(ZLjava/lang/Object;Ldev/deliteai/datamodels/NimbleNetError;)V");
        if (constructorMethodId == nullptr) {
            LOGE("Constructor for NimbleNetResult not found.\n");
        }

        statusFieldId = env->GetFieldID(resultClass, "status", "Z");
        if (statusFieldId == nullptr) {
            LOGE("Field 'status' not found in NimbleNetResult class.\n");
        }

        payloadFieldId = env->GetFieldID(resultClass, "payload", "Ljava/lang/Object;");
        if (payloadFieldId == nullptr) {
            LOGE("Field 'data' not found in NimbleNetResult class.\n");
        }

        errorFieldId = env->GetFieldID(resultClass, "error", "Ldev/deliteai/datamodels/NimbleNetError;");
        if (errorFieldId == nullptr) {
            LOGE("Field 'error' not found in NimbleNetResult class.\n");
        }
    }

    void setStatus(JNIEnv *env, jobject resultObj, jboolean status) {
        if (resultObj == nullptr) {
            LOGE("resultObj is null in setStatus.\n");
            return;
        }
        env->SetBooleanField(resultObj, statusFieldId, status);
    }

    jobject getData(JNIEnv *env, jobject resultObj) {
        if (resultObj == nullptr) {
            LOGE("resultObj is null in getData.\n");
            return nullptr;
        }
        return env->GetObjectField(resultObj, payloadFieldId);
    }

    void setData(JNIEnv *env, jobject resultObj, jobject data) {
        if (resultObj == nullptr) {
            LOGE("resultObj is null in setData.\n");
            return;
        }
        env->SetObjectField(resultObj, payloadFieldId, data);
    }

    jobject getError(JNIEnv *env, jobject resultObj) {
        if (resultObj == nullptr) {
            LOGE("resultObj is null in getError.\n");
            return nullptr;
        }
        return env->GetObjectField(resultObj, errorFieldId);
    }

    void setError(JNIEnv *env, jobject resultObj, jobject error) {
        if (resultObj == nullptr) {
            LOGE("resultObj is null in setError.\n");
            return;
        }
        env->SetObjectField(resultObj, errorFieldId, error);
    }
};
