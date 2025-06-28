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

class UserEventDataShadow {
private:
    jclass userEventDataClass;
    jmethodID setEventTypeMethodId;
    jmethodID setEventJsonStringMethodId;

public:
    UserEventDataShadow(JNIEnv *env) {
        if (env == nullptr) return;

        jclass localClass = env->FindClass("ai/nimbleedge/datamodels/UserEventData");
        if (localClass == nullptr) {
            LOGE("Class ai.nimbleedge.datamodels.UserEventData not found.\n");
            return;
        }

        userEventDataClass = static_cast<jclass>(env->NewGlobalRef(localClass));
        env->DeleteLocalRef(localClass);
        if (userEventDataClass == nullptr) {
            LOGE("Failed to create global reference for UserEventData class.\n");
            return;
        }

        setEventTypeMethodId = env->GetMethodID(userEventDataClass, "setEventType", "(Ljava/lang/String;)V");
        setEventJsonStringMethodId = env->GetMethodID(userEventDataClass, "setEventJsonString", "(Ljava/lang/String;)V");

        if (setEventTypeMethodId == nullptr || setEventJsonStringMethodId == nullptr) {
            LOGE("One or more setter methods in UserEventData class could not be found.\n");
        }
    }

    void setEventType(JNIEnv *env, jobject userEventDataObject, jstring jeventType) {
        if (env == nullptr || userEventDataObject == nullptr || setEventTypeMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call setEventType.");
        }

        env->CallVoidMethod(userEventDataObject, setEventTypeMethodId, jeventType);
    }

    void setEventJsonString(JNIEnv *env, jobject userEventDataObject, jstring jeventJsonString) {
        if (env == nullptr || userEventDataObject == nullptr || setEventJsonStringMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call setEventJsonString.");
        }

        env->CallVoidMethod(userEventDataObject, setEventJsonStringMethodId, jeventJsonString);
    }
};
