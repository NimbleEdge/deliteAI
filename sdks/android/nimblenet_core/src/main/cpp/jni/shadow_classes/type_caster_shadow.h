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

class TypeCasterShadow {
private:
    jclass typeCasterClass;

    jmethodID castToIntMethodId;
    jmethodID castToDoubleMethodId;
    jmethodID castToFloatMethodId;
    jmethodID castToBooleanMethodId;
    jmethodID castToLongMethodId;
    jmethodID castToByteMethodId;
    jmethodID castToStringMethodId;

public:
    TypeCasterShadow(JNIEnv *env) {
        if (env == nullptr) return;

        // Find the TypeCaster class
        jclass localClass = env->FindClass("dev/deliteai/impl/common/utils/TypeCaster");
        if (localClass == nullptr) {
            LOGD("Class dev.deliteai.utils.TypeCaster not found.\n");
            return;
        }
        typeCasterClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
        if (typeCasterClass == nullptr) {
            LOGD("Failed to create global reference for TypeCaster class.\n");
            return;
        }

        // Get method IDs for each function
        castToIntMethodId = env->GetStaticMethodID(typeCasterClass, "castToInt", "(Ljava/lang/Object;)I");
        castToDoubleMethodId = env->GetStaticMethodID(typeCasterClass, "castToDouble", "(Ljava/lang/Object;)D");
        castToFloatMethodId = env->GetStaticMethodID(typeCasterClass, "castToFloat", "(Ljava/lang/Object;)F");
        castToBooleanMethodId = env->GetStaticMethodID(typeCasterClass, "castToBoolean", "(Ljava/lang/Object;)Z");
        castToLongMethodId = env->GetStaticMethodID(typeCasterClass, "castToLong", "(Ljava/lang/Object;)J");
        castToByteMethodId = env->GetStaticMethodID(typeCasterClass, "castToByte", "(Ljava/lang/Object;)B");
        castToStringMethodId = env->GetStaticMethodID(typeCasterClass, "castToString", "(Ljava/lang/Object;)Ljava/lang/String;");

        // Check if any method ID is null
        if (!castToIntMethodId || !castToDoubleMethodId || !castToFloatMethodId || !castToBooleanMethodId ||
            !castToLongMethodId || !castToByteMethodId || !castToStringMethodId) {
            LOGD("One or more methods in TypeCaster class could not be found.\n");
        }
    }

    jint castToInt(JNIEnv *env, jobject value) {
        return env->CallStaticIntMethod(typeCasterClass, castToIntMethodId, value);
    }

    jdouble castToDouble(JNIEnv *env, jobject value) {
        return env->CallStaticDoubleMethod(typeCasterClass, castToDoubleMethodId, value);
    }

    jfloat castToFloat(JNIEnv *env, jobject value) {
        return env->CallStaticFloatMethod(typeCasterClass, castToFloatMethodId, value);
    }

    jboolean castToBoolean(JNIEnv *env, jobject value) {
        return env->CallStaticBooleanMethod(typeCasterClass, castToBooleanMethodId, value);
    }

    jlong castToLong(JNIEnv *env, jobject value) {
        return env->CallStaticLongMethod(typeCasterClass, castToLongMethodId, value);
    }

    jbyte castToByte(JNIEnv *env, jobject value) {
        return env->CallStaticByteMethod(typeCasterClass, castToByteMethodId, value);
    }

    std::string castToString(JNIEnv *env, jobject value) {
        jstring jstr = (jstring)env->CallStaticObjectMethod(typeCasterClass, castToStringMethodId, value);
        if (jstr == nullptr) {
            return "";
        }
        const char *chars = env->GetStringUTFChars(jstr, nullptr);
        std::string result(chars);
        env->ReleaseStringUTFChars(jstr, chars);
        env->DeleteLocalRef(jstr);
        return result;
    }
};
