/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <stdexcept>
#include <cstdio>

class MutableMapShadow {
private:
    jclass mapClass;
    jmethodID constructorMethodId;
    jmethodID sizeMethodId;
    jmethodID isEmptyMethodId;
    jmethodID containsKeyMethodId;
    jmethodID getMethodId;
    jmethodID putMethodId;
    jmethodID keySetMethodId;
    jmethodID entrySetMethodId;

public:
    MutableMapShadow(JNIEnv* env) {
        if(env == nullptr) return;

        jclass localClass = env->FindClass("java/util/HashMap");
        if (localClass == nullptr) {
            LOGD("Class java.util.HashMap not found.\n");
            return;
        }
        mapClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);
        if (mapClass == nullptr) {
            LOGD("Failed to create global reference for HashMap class.\n");
            return;
        }

        constructorMethodId = env->GetMethodID(mapClass, "<init>", "()V");
        if (constructorMethodId == nullptr) {
            LOGD("Constructor for HashMap not found.\n");
        }

        sizeMethodId = env->GetMethodID(mapClass, "size", "()I");
        if (sizeMethodId == nullptr) {
            LOGD("Method size() not found in HashMap class.\n");
        }

        isEmptyMethodId = env->GetMethodID(mapClass, "isEmpty", "()Z");
        if (isEmptyMethodId == nullptr) {
            LOGD("Method isEmpty() not found in HashMap class.\n");
        }

        containsKeyMethodId = env->GetMethodID(mapClass, "containsKey", "(Ljava/lang/Object;)Z");
        if (containsKeyMethodId == nullptr) {
            LOGD("Method containsKey() not found in HashMap class.\n");
        }

        getMethodId = env->GetMethodID(mapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
        if (getMethodId == nullptr) {
            LOGD("Method get() not found in HashMap class.\n");
        }

        putMethodId = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        if (putMethodId == nullptr) {
            LOGD("Method put() not found in HashMap class.\n");
        }

        keySetMethodId = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
        if (keySetMethodId == nullptr) {
            LOGD("Method keySet() not found in HashMap class.\n");
        }

        entrySetMethodId = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
        if (entrySetMethodId == nullptr) {
            LOGD("Method entrySet() not found in HashMap class.\n");
        }
    }

    jobject create(JNIEnv* env) {
        if (constructorMethodId == nullptr) {
            LOGD("Constructor method ID is null. Cannot create HashMap object.\n");
            return nullptr;
        }
        return env->NewObject(mapClass, constructorMethodId);
    }

    jint size(JNIEnv* env, jobject mapObj) {
        if (mapObj == nullptr) {
            LOGD("mapObj is null in size().\n");
            return 0;
        }
        return env->CallIntMethod(mapObj, sizeMethodId);
    }

    jboolean isEmpty(JNIEnv* env, jobject mapObj) {
        if (mapObj == nullptr) {
            LOGD("mapObj is null in isEmpty().\n");
            return JNI_TRUE;
        }
        return env->CallBooleanMethod(mapObj, isEmptyMethodId);
    }

    jboolean containsKey(JNIEnv* env, jobject mapObj, const std::string& key) {
        if (mapObj == nullptr) {
            LOGD("mapObj is null in containsKey().\n");
            return JNI_FALSE;
        }
        jstring jKey = env->NewStringUTF(key.c_str());
        jboolean result = env->CallBooleanMethod(mapObj, containsKeyMethodId, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    jobject get(JNIEnv* env, jobject mapObj, const std::string& key) {
        if (mapObj == nullptr) {
            LOGD("mapObj is null in get().\n");
            return nullptr;
        }
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject value = env->CallObjectMethod(mapObj, getMethodId, jKey);
        env->DeleteLocalRef(jKey);
        return value;
    }

    void put(JNIEnv* env, jobject mapObj, const std::string& key, jobject value) {
        if (mapObj == nullptr) {
            LOGD("mapObj is null in put().\n");
            return;
        }
        jstring jKey = env->NewStringUTF(key.c_str());
        env->DeleteLocalRef(env->CallObjectMethod(mapObj, putMethodId, jKey, value));
        env->DeleteLocalRef(jKey);
    }

    std::vector<std::string> getKeys(JNIEnv* env, jobject mapObj) {
        std::vector<std::string> keys;
        if (mapObj == nullptr) {
            LOGD("mapObj is null in getKeys().\n");
            return keys;
        }
        jobject keySet = env->CallObjectMethod(mapObj, keySetMethodId);
        if (keySet == nullptr) {
            LOGD("KeySet is null.\n");
            return keys;
        }

        jclass setClass = env->GetObjectClass(keySet);
        jmethodID iteratorMethodId = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
        jobject iterator = env->CallObjectMethod(keySet, iteratorMethodId);

        jclass iteratorClass = env->GetObjectClass(iterator);
        jmethodID hasNextMethodId = env->GetMethodID(iteratorClass, "hasNext", "()Z");
        jmethodID nextMethodId = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

        while (env->CallBooleanMethod(iterator, hasNextMethodId)) {
            jstring key = (jstring) env->CallObjectMethod(iterator, nextMethodId);
            const char* keyStr = env->GetStringUTFChars(key, nullptr);
            keys.emplace_back(keyStr);
            env->ReleaseStringUTFChars(key, keyStr);
            env->DeleteLocalRef(key);
        }

        env->DeleteLocalRef(keySet);
        env->DeleteLocalRef(iterator);
        env->DeleteLocalRef(setClass);
        env->DeleteLocalRef(iteratorClass);
        return keys;
    }
};
