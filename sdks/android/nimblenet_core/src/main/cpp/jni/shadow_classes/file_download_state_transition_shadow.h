/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <stdexcept>

#include "../utils/jni_logger.h"

extern JavaVM* globalJvm;

class FileDownloadStateTransitionShadow {
private:
    static jclass fileDownloadStateTransitionClass;
    static jmethodID getCurrentStateMethodId;
    static jmethodID getPreviousStateMethodId;
    static jmethodID getTimeTakenMethodId;
    static jmethodID getCurrentStateReasonCodeMethodId;
    static jmethodID getDownloadManagerDownloadIdMethodId;

    jobject fileDownloadStateTransitionObject;

public:
    static bool init(JNIEnv* env) {
        if (env == nullptr) return false;

        jclass localClass = env->FindClass("ai/nimbleedge/impl/io/datamodels/FileDownloadStateTransition");
        if (localClass == nullptr) {
            LOGE("Class ai.nimbleedge.datamodels.FileDownloadStateTransition not found.\n");
            return  false;
        }
        fileDownloadStateTransitionClass = static_cast<jclass>(env->NewGlobalRef(localClass));
        env->DeleteLocalRef(localClass);
        if (!fileDownloadStateTransitionClass) {
            LOGE("Failed to create global reference for FileDownloadStateTransition.\n");
            return false;
        }

        getCurrentStateMethodId = env->GetMethodID(fileDownloadStateTransitionClass, "getCurrentState", "()I");
        getPreviousStateMethodId = env->GetMethodID(fileDownloadStateTransitionClass, "getPreviousState", "()I");
        getTimeTakenMethodId = env->GetMethodID(fileDownloadStateTransitionClass, "getTimeTaken", "()J");
        getCurrentStateReasonCodeMethodId = env->GetMethodID(fileDownloadStateTransitionClass, "getCurrentStateReasonCode", "()I");
        getDownloadManagerDownloadIdMethodId = env->GetMethodID(fileDownloadStateTransitionClass, "getDownloadManagerDownloadId", "()J");
        if (!getCurrentStateMethodId || !getPreviousStateMethodId || !getTimeTakenMethodId || !getCurrentStateReasonCodeMethodId) {
            LOGE("One or more methods in FileDownloadStateTransition not found.\n");
            return false;
        }

        return true;
    }

    FileDownloadStateTransitionShadow(JNIEnv* env, jobject obj) {
        if (env == nullptr || obj == nullptr) {
            throw std::runtime_error("Invalid constructor arguments.");
        }
        fileDownloadStateTransitionObject = env->NewGlobalRef(obj);
    }

    ~FileDownloadStateTransitionShadow() {
        JNIEnv* env;
        int getEnvStatus = globalJvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (fileDownloadStateTransitionObject && getEnvStatus != JNI_EDETACHED && env != nullptr) {
            env->DeleteGlobalRef(fileDownloadStateTransitionObject);
        }
    }

    jint getCurrentState(JNIEnv* env) {
        if (!env || !fileDownloadStateTransitionObject || !getCurrentStateMethodId) {
            throw std::runtime_error("Invalid state to call getCurrentState.");
        }
        jint ret = env->CallIntMethod(fileDownloadStateTransitionObject, getCurrentStateMethodId);
        return ret;
    }

    jint getPreviousState(JNIEnv* env) {
        if (!env || !fileDownloadStateTransitionObject || !getPreviousStateMethodId) {
            throw std::runtime_error("Invalid state to call getPreviousState.");
        }
        jint ret = env->CallIntMethod(fileDownloadStateTransitionObject, getPreviousStateMethodId);
        return ret;
    }

    jlong getTimeTaken(JNIEnv* env) {
        if (!env || !fileDownloadStateTransitionObject || !getTimeTakenMethodId) {
            throw std::runtime_error("Invalid state to call getTimeTaken.");
        }
        jlong ret = env->CallLongMethod(fileDownloadStateTransitionObject, getTimeTakenMethodId);
        return ret;
    }

    jint getCurrentStateReasonCode(JNIEnv* env) {
        if (!env || !fileDownloadStateTransitionObject || !getCurrentStateReasonCodeMethodId) {
            throw std::runtime_error("Invalid state to call getCurrentStateReasonCode.");
        }
        jint ret = env->CallIntMethod(fileDownloadStateTransitionObject, getCurrentStateReasonCodeMethodId);
        return ret;
    }

    jlong getDownloadManagerDownloadId(JNIEnv* env){
        if (!env || !fileDownloadStateTransitionObject || !getDownloadManagerDownloadIdMethodId) {
            throw std::runtime_error("Invalid state to call getDownloadManagerDownloadId.");
        }
        jlong ret = env->CallLongMethod(fileDownloadStateTransitionObject,
                                        getDownloadManagerDownloadIdMethodId);
        return ret;
    }
};
