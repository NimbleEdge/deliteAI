/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "nimble_net_error_shadow.h"
#include "nimble_net_result_shadow.h"
#include "nimblenet.hpp"
#include "utils/jni_string.h"

extern NimbleNetResultShadow nimbleNetResultShadow;
extern NimbleNetErrorShadow nimbleNetErrorShadow;

static void inline populateNimbleNetResult(JNIEnv *env, jobject nimblenet_result_android,
                                           NimbleNetStatus *nimbleNetStatus, jobject dataObject,
                                           jstring errorMessage) {
    if (nimbleNetStatus == nullptr && errorMessage == nullptr) {
        nimbleNetResultShadow.setStatus(env, nimblenet_result_android, true);
        nimbleNetResultShadow.setData(env, nimblenet_result_android, dataObject);
        nimbleNetResultShadow.setError(env, nimblenet_result_android, nullptr);
    } else {
        nimbleNetResultShadow.setStatus(env, nimblenet_result_android, false);
        nimbleNetResultShadow.setData(env, nimblenet_result_android, dataObject);
        // Send data object to frontend even in case of error so script can propagate error details outside

        jobject nimbleNetErrorObject = nimbleNetResultShadow.getError(env,
                                                                      nimblenet_result_android);

        if (errorMessage != nullptr) {
            nimbleNetErrorShadow.populateErrorObject(env, nimbleNetErrorObject, JNI_ERROR_CODE,
                                                     JniString::jstringToStdString(env,
                                                                                   errorMessage));
        } else {
            nimbleNetErrorShadow.populateErrorObject(env, nimbleNetErrorObject,
                                                     nimbleNetStatus->code,
                                                     JniString::charToStdString(
                                                             nimbleNetStatus->message));
        }
        env->DeleteLocalRef(nimbleNetErrorObject);
    }
    deallocate_nimblenet_status(nimbleNetStatus);
}

static void inline attachCrashReporter() {
    auto t = nimblenetInternal::attach_cleanup_to_thread();
}

static jintArray inline
createIntArrayFromVector(JNIEnv *env, const std::vector<int64_t> &shapeVector) {
    jintArray intArray = env->NewIntArray(shapeVector.size());
    std::vector<jint> temp(shapeVector.begin(), shapeVector.end());
    env->SetIntArrayRegion(intArray, 0, temp.size(), temp.data());
    return intArray;
}

static int inline getMaxLocalRefsAllowedInTheCurrentFrame(JNIEnv *env) {
    int allowedRefs = 0;

    #ifdef JNITESTING
        while (true) {
            if (env->EnsureLocalCapacity(allowedRefs + 1) != 0) {
                if (env->ExceptionCheck()) {
                    env->ExceptionClear();
                }
                break;
            }

            allowedRefs++;
        }
    #endif

    return allowedRefs;
}

static void inline checkForUndeletedLocalReferences(JNIEnv *env, int initialAllowedRefCount,
                                                    std::string functionName) {
    auto finalLocalRefsAllowed = getMaxLocalRefsAllowedInTheCurrentFrame(env);
    if (initialAllowedRefCount != finalLocalRefsAllowed) {
        LOGE("%d Local reference(s) have not been cleared in the JNI %s",
             initialAllowedRefCount - finalLocalRefsAllowed, functionName.c_str());
    }
}
