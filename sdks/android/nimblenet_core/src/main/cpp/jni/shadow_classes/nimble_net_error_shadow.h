/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>
#include <string>

class NimbleNetErrorShadow {
private:
    jclass nimbleNetErrorClass;
    jfieldID codeFieldId;
    jfieldID messageFieldId;
    jmethodID populateErrorObjectMethodId;

public:
    NimbleNetErrorShadow(JNIEnv *env) {
        if (env == nullptr) return;
        jclass localClass = env->FindClass("ai/nimbleedge/datamodels/NimbleNetError");
        if (localClass == nullptr) return;
        nimbleNetErrorClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
        if (nimbleNetErrorClass == nullptr) return;
        codeFieldId = env->GetFieldID(nimbleNetErrorClass, "code", "I");
        messageFieldId = env->GetFieldID(nimbleNetErrorClass, "message", "Ljava/lang/String;");
        populateErrorObjectMethodId = env->GetMethodID(nimbleNetErrorClass, "populateErrorObject", "(ILjava/lang/String;)V");
    }

    void setCode(JNIEnv *env, jobject nimbleNetErrorObject, jint code) {
        env->SetIntField(nimbleNetErrorObject, codeFieldId, code);
    }

    jint getCode(JNIEnv *env, jobject nimbleNetErrorObject) {
        return env->GetIntField(nimbleNetErrorObject, codeFieldId);
    }

    void setMessage(JNIEnv *env, jobject nimbleNetErrorObject, const std::string &message) {
        jstring jMessage = env->NewStringUTF(message.c_str());
        env->SetObjectField(nimbleNetErrorObject, messageFieldId, jMessage);
        env->DeleteLocalRef(jMessage);
    }

    std::string getMessage(JNIEnv *env, jobject nimbleNetErrorObject) {
        jstring jMessage = (jstring)env->GetObjectField(nimbleNetErrorObject, messageFieldId);
        if (jMessage == nullptr) {
            return "";
        }
        const char *chars = env->GetStringUTFChars(jMessage, nullptr);
        std::string result(chars);
        env->ReleaseStringUTFChars(jMessage, chars);
        env->DeleteLocalRef(jMessage);
        return result;
    }

    void populateErrorObject(JNIEnv *env, jobject nimbleNetErrorObject, jint code, const std::string &message) {
        jstring jMessage = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(nimbleNetErrorObject, populateErrorObjectMethodId, code, jMessage);
        env->DeleteLocalRef(jMessage);
    }
};
