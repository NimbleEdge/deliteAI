/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <string>
#include <vector>

#include "../utils/jni_logger.h"
#include "nimble_net_util.hpp"

class JSONObjectShadow {
private:
    jclass jsonObjectClass;
    jmethodID hasMethodID;
    jmethodID getStringMethodID;
    jmethodID getIntMethodID;
    jmethodID getLongMethodID;
    jmethodID getDoubleMethodID;
    jmethodID putMethodID;
    jmethodID keysMethodID;
    jmethodID getJSONObjectMethodID;
    jmethodID getJSONArrayMethodID;
    jmethodID getMethodID;
    jmethodID getBoolMethodID;

    jmethodID putIntMethodID;
    jmethodID putLongMethodID;
    jmethodID putDoubleMethodID;
    jmethodID putBooleanMethodID;

    jclass stringClass;
    jclass integerClass;
    jclass longClass;
    jclass doubleClass;
    jclass floatClass;
    jclass booleanClass;
    jclass jsonArrayClass;
    jclass jsonObjectClassRef;

    jmethodID integerConstructorMethodID;
    jmethodID longConstructorMethodID;
    jmethodID doubleConstructorMethodID;
    jmethodID booleanConstructorMethodID;

    jfieldID nullFieldID;

    void initializeMethodIDs(JNIEnv* env) {
        hasMethodID = env->GetMethodID(jsonObjectClass, "has", "(Ljava/lang/String;)Z");
        getStringMethodID = env->GetMethodID(jsonObjectClass, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
        getIntMethodID = env->GetMethodID(jsonObjectClass, "getInt", "(Ljava/lang/String;)I");
        getLongMethodID = env->GetMethodID(jsonObjectClass, "getLong", "(Ljava/lang/String;)J");
        getBoolMethodID = env->GetMethodID(jsonObjectClass, "getBoolean", "(Ljava/lang/String;)Z");
        getDoubleMethodID = env->GetMethodID(jsonObjectClass, "getDouble", "(Ljava/lang/String;)D");
        putMethodID = env->GetMethodID(jsonObjectClass, "put", "(Ljava/lang/String;Ljava/lang/Object;)Lorg/json/JSONObject;");
        keysMethodID = env->GetMethodID(jsonObjectClass, "keys", "()Ljava/util/Iterator;");
        getJSONObjectMethodID = env->GetMethodID(jsonObjectClass, "getJSONObject", "(Ljava/lang/String;)Lorg/json/JSONObject;");
        getJSONArrayMethodID = env->GetMethodID(jsonObjectClass, "getJSONArray", "(Ljava/lang/String;)Lorg/json/JSONArray;");
        getMethodID = env->GetMethodID(jsonObjectClass, "get", "(Ljava/lang/String;)Ljava/lang/Object;");

        jclass localStringClass = env->FindClass("java/lang/String");
        stringClass = static_cast<jclass>(env->NewGlobalRef(localStringClass));
        env->DeleteLocalRef(localStringClass);

        jclass localIntegerClass = env->FindClass("java/lang/Integer");
        integerClass = static_cast<jclass>(env->NewGlobalRef(localIntegerClass));
        env->DeleteLocalRef(localIntegerClass);

        jclass localLongClass = env->FindClass("java/lang/Long");
        longClass = static_cast<jclass>(env->NewGlobalRef(localLongClass));
        env->DeleteLocalRef(localLongClass);

        jclass localDoubleClass = env->FindClass("java/lang/Double");
        doubleClass = static_cast<jclass>(env->NewGlobalRef(localDoubleClass));
        env->DeleteLocalRef(localDoubleClass);

        jclass localFloatClass = env->FindClass("java/lang/Float");
        floatClass = static_cast<jclass>(env->NewGlobalRef(localFloatClass));
        env->DeleteLocalRef(localFloatClass);

        jclass localBooleanClass = env->FindClass("java/lang/Boolean");
        booleanClass = static_cast<jclass>(env->NewGlobalRef(localBooleanClass));
        env->DeleteLocalRef(localBooleanClass);

        jclass localJsonArrayClass = env->FindClass("org/json/JSONArray");
        jsonArrayClass = static_cast<jclass>(env->NewGlobalRef(localJsonArrayClass));
        env->DeleteLocalRef(localJsonArrayClass);

        jclass localJsonObjectClass = env->FindClass("org/json/JSONObject");
        jsonObjectClassRef = static_cast<jclass>(env->NewGlobalRef(localJsonObjectClass));
        env->DeleteLocalRef(localJsonObjectClass);

        integerConstructorMethodID = env->GetMethodID(integerClass, "<init>", "(I)V");
        longConstructorMethodID = env->GetMethodID(longClass, "<init>", "(J)V");
        doubleConstructorMethodID = env->GetMethodID(doubleClass, "<init>", "(D)V");
        booleanConstructorMethodID = env->GetMethodID(booleanClass, "<init>", "(Z)V");

        nullFieldID = env->GetStaticFieldID(jsonObjectClass, "NULL", "Ljava/lang/Object;");
    }


public:
    JSONObjectShadow(JNIEnv* env)
            : jsonObjectClass(nullptr),
              hasMethodID(nullptr),
              getStringMethodID(nullptr),
              getIntMethodID(nullptr),
              getLongMethodID(nullptr),
              getDoubleMethodID(nullptr),
              putMethodID(nullptr),
              keysMethodID(nullptr),
              getJSONObjectMethodID(nullptr),
              getJSONArrayMethodID(nullptr),
              getMethodID(nullptr),
              putIntMethodID(nullptr),
              putLongMethodID(nullptr),
              putDoubleMethodID(nullptr),
              putBooleanMethodID(nullptr),
              stringClass(nullptr),
              integerClass(nullptr),
              longClass(nullptr),
              doubleClass(nullptr),
              floatClass(nullptr),
              booleanClass(nullptr),
              jsonArrayClass(nullptr),
              jsonObjectClassRef(nullptr),
              integerConstructorMethodID(nullptr),
              longConstructorMethodID(nullptr),
              doubleConstructorMethodID(nullptr),
              booleanConstructorMethodID(nullptr),
              nullFieldID(nullptr){
        if(env == nullptr) return;
        jclass localJsonObjectClass = env->FindClass("org/json/JSONObject");
        if (localJsonObjectClass != nullptr) {
            jsonObjectClass = static_cast<jclass>(env->NewGlobalRef(localJsonObjectClass));
            env->DeleteLocalRef(localJsonObjectClass);
            initializeMethodIDs(env);
        }
    }

    jobject create(JNIEnv* env) {
        if (putMethodID == nullptr || jsonObjectClass == nullptr) {
            LOGD("Required method IDs or classes are not initialized. Cannot create JSONObject.\n");
            return nullptr;
        }
        jmethodID constructorMethodID = env->GetMethodID(jsonObjectClass, "<init>", "()V");
        if (constructorMethodID == nullptr) {
            LOGD("Default constructor for JSONObject not found.\n");
            return nullptr;
        }
        return env->NewObject(jsonObjectClass, constructorMethodID);
    }

    bool has(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || hasMethodID == nullptr) return false;
        jstring jKey = env->NewStringUTF(key.c_str());
        jboolean result = env->CallBooleanMethod(jsonObject, hasMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result == JNI_TRUE;
    }

    std::string getString(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getStringMethodID == nullptr) return "";
        jstring jKey = env->NewStringUTF(key.c_str());
        jstring jStr = (jstring)env->CallObjectMethod(jsonObject, getStringMethodID, jKey);
        env->DeleteLocalRef(jKey);
        if (jStr == nullptr) return "";
        const char* cStr = env->GetStringUTFChars(jStr, nullptr);
        std::string result(cStr);
        env->ReleaseStringUTFChars(jStr, cStr);
        env->DeleteLocalRef(jStr);
        return result;
    }

    int getInt(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getIntMethodID == nullptr) return 0;
        jstring jKey = env->NewStringUTF(key.c_str());
        auto result = env->CallIntMethod(jsonObject, getIntMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    long getLong(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getIntMethodID == nullptr) return 0;
        jstring jKey = env->NewStringUTF(key.c_str());
        auto result = env->CallLongMethod(jsonObject, getLongMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    double getDouble(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getDoubleMethodID == nullptr) return 0.0;
        jstring jKey = env->NewStringUTF(key.c_str());
        auto result = env->CallDoubleMethod(jsonObject, getDoubleMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    bool getBoolean(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getDoubleMethodID == nullptr) return 0.0;
        jstring jKey = env->NewStringUTF(key.c_str());
        auto result = env->CallBooleanMethod(jsonObject, getBoolMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    void put(JNIEnv* env, jobject jsonObject, const std::string& key, jobject value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;

        jstring jKey = env->NewStringUTF(key.c_str());

        // If value is nullptr, use JSONObject.NULL
        if (value == nullptr) {
            jfieldID nullFieldID = env->GetStaticFieldID(jsonObjectClass, "NULL", "Ljava/lang/Object;");
            value = env->GetStaticObjectField(jsonObjectClass, nullFieldID);
        }

        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, value));
        env->DeleteLocalRef(jKey);
    }


    void putInt(JNIEnv* env, jobject jsonObject, const std::string& key, int value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject intValue = env->NewObject(integerClass, integerConstructorMethodID, value);
        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, intValue));
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(intValue);
    }

    void putLong(JNIEnv* env, jobject jsonObject, const std::string& key, jlong value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject longValue = env->NewObject(longClass, longConstructorMethodID, value);
        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, longValue));
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(longValue);
    }

    void putDouble(JNIEnv* env, jobject jsonObject, const std::string& key, jdouble value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject doubleValue = env->NewObject(doubleClass, doubleConstructorMethodID, value);
        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, doubleValue));
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(doubleValue);
    }

    void putBoolean(JNIEnv* env, jobject jsonObject, const std::string& key, jboolean value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject boolValue = env->NewObject(booleanClass, booleanConstructorMethodID, value);
        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, boolValue));
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(boolValue);
    }

    void putString(JNIEnv* env, jobject jsonObject, const std::string& key, const std::string& value) {
        if (jsonObject == nullptr || putMethodID == nullptr) return;
        jstring jKey = env->NewStringUTF(key.c_str());
        jstring jValue = env->NewStringUTF(value.c_str());
        env->DeleteLocalRef(env->CallObjectMethod(jsonObject, putMethodID, jKey, jValue));
        env->DeleteLocalRef(jKey);
        env->DeleteLocalRef(jValue);
    }

    std::vector<std::string> keys(JNIEnv* env, jobject jsonObject) {
        std::vector<std::string> keyList;
        if (jsonObject == nullptr || keysMethodID == nullptr) return keyList;
        jobject iterator = env->CallObjectMethod(jsonObject, keysMethodID);
        if (iterator == nullptr) return keyList;
        jclass iteratorClass = env->FindClass("java/util/Iterator");
        jmethodID hasNextMethodID = env->GetMethodID(iteratorClass, "hasNext", "()Z");
        jmethodID nextMethodID = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
        while (env->CallBooleanMethod(iterator, hasNextMethodID) == JNI_TRUE) {
            jstring jKey = (jstring)env->CallObjectMethod(iterator, nextMethodID);
            const char* cKey = env->GetStringUTFChars(jKey, nullptr);
            keyList.push_back(std::string(cKey));
            env->ReleaseStringUTFChars(jKey, cKey);
            env->DeleteLocalRef(jKey);
        }
        env->DeleteLocalRef(iterator);
        env->DeleteLocalRef(iteratorClass);
        return keyList;
    }

    jobject getJSONObject(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getJSONObjectMethodID == nullptr) return nullptr;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject result = env->CallObjectMethod(jsonObject, getJSONObjectMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    jobject getJSONArray(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr || getJSONArrayMethodID == nullptr) return nullptr;
        jstring jKey = env->NewStringUTF(key.c_str());
        jobject result = env->CallObjectMethod(jsonObject, getJSONArrayMethodID, jKey);
        env->DeleteLocalRef(jKey);
        return result;
    }

    int getDataType(JNIEnv* env, jobject jsonObject, const std::string& key) {
        if (jsonObject == nullptr) throw std::runtime_error(std::string("json is null"));

        jstring jKey = env->NewStringUTF(key.c_str());
        jobject value = env->CallObjectMethod(jsonObject, getMethodID, jKey);
        jobject localJsonNullObj = env->GetStaticObjectField(jsonObjectClass, nullFieldID);
        env->DeleteLocalRef(jKey);

        if (value == nullptr) {
            env->DeleteLocalRef(localJsonNullObj);
            throw std::runtime_error(std::string("value is nullptr against the key ") + key);
        }

        int dataType = 0;
        if (env->IsInstanceOf(value, stringClass)) dataType = DATATYPE::STRING;
        else if (env->IsInstanceOf(value, integerClass)) dataType = DATATYPE::INT32;
        else if (env->IsInstanceOf(value, longClass)) dataType = DATATYPE::INT64;
        else if (env->IsInstanceOf(value, doubleClass)) dataType = DATATYPE::DOUBLE;
        else if (env->IsInstanceOf(value, floatClass)) dataType = DATATYPE::FLOAT;
        else if (env->IsInstanceOf(value, booleanClass)) dataType = DATATYPE::BOOLEAN;
        else if (env->IsInstanceOf(value, jsonObjectClassRef)) dataType = DATATYPE::JSON;
        else if (env->IsInstanceOf(value, jsonArrayClass)) dataType = DATATYPE::JSON_ARRAY;
        else if (env->IsSameObject(value, localJsonNullObj)) dataType = DATATYPE::NONE;
        else {
            env->DeleteLocalRef(value);
            env->DeleteLocalRef(localJsonNullObj);
            throw std::runtime_error(
                    std::string("Invalid datatype found in json against the key ") + key);
        }

        env->DeleteLocalRef(value);
        env->DeleteLocalRef(localJsonNullObj);
        return dataType;
    }
};
