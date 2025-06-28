/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_SDK_JNI_STRING_H
#define ANDROID_SDK_JNI_STRING_H

#include <string>
#include <cstring>
#include "jni.h"

class JniString {
public:
    static std::string jstringToStdString(JNIEnv *env, jstring jStr) {
        if (!jStr) return "";
        const char *chars = env->GetStringUTFChars(jStr, nullptr);
        std::string result(chars);
        env->ReleaseStringUTFChars(jStr, chars);
        return result;
    }

    static jstring cStringToJstring(JNIEnv *env, const char *str) {
        return env->NewStringUTF(str);
    }

    static std::string constCharToStdString(const char *str) {
        if (!str) return "";
        return std::string(str);
    }

    static std::string charToStdString(char *str) {
        if (!str) return "";
        return std::string(str);
    }
};

#endif  // ANDROID_SDK_JNI_STRING_H
