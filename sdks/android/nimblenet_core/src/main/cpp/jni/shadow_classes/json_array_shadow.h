/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>
#include <string>
#include "nimble_net_util.hpp"

class JSONArrayShadow {
private:
    jclass jsonArrayClass;
    jmethodID constructorMethodID;
    jmethodID lengthMethodID;
    jmethodID getMethodID;
    jmethodID getJSONObjectMethodID;
    jmethodID getJSONArrayMethodID;
    jmethodID getStringMethodID;
    jmethodID getIntMethodID;
    jmethodID getLongMethodID;
    jmethodID getDoubleMethodID;
    jmethodID getBooleanMethodID;
    jmethodID putMethodID;

    jclass stringClass;
    jclass integerClass;
    jclass longClass;
    jclass doubleClass;
    jclass floatClass;
    jclass booleanClass;
    jclass jsonObjectClass;
    jclass jsonArrayClassRef;

    jmethodID integerConstructorMethodID;
    jmethodID longConstructorMethodID;
    jmethodID doubleConstructorMethodID;
    jmethodID booleanConstructorMethodID;

    jfieldID nullFieldID;

    void initializeMethodIDs(JNIEnv* env) {
        lengthMethodID = env->GetMethodID(jsonArrayClass, "length", "()I");
        getMethodID = env->GetMethodID(jsonArrayClass, "get", "(I)Ljava/lang/Object;");
        getJSONObjectMethodID = env->GetMethodID(jsonArrayClass, "getJSONObject", "(I)Lorg/json/JSONObject;");
        getJSONArrayMethodID = env->GetMethodID(jsonArrayClass, "getJSONArray", "(I)Lorg/json/JSONArray;");
        getStringMethodID = env->GetMethodID(jsonArrayClass, "getString", "(I)Ljava/lang/String;");
        getIntMethodID = env->GetMethodID(jsonArrayClass, "getInt", "(I)I");
        getLongMethodID = env->GetMethodID(jsonArrayClass, "getLong", "(I)J");
        getDoubleMethodID = env->GetMethodID(jsonArrayClass, "getDouble", "(I)D");
        getBooleanMethodID = env->GetMethodID(jsonArrayClass, "getBoolean", "(I)Z");
        putMethodID = env->GetMethodID(jsonArrayClass, "put", "(Ljava/lang/Object;)Lorg/json/JSONArray;");

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

        jclass localJsonObjectClass = env->FindClass("org/json/JSONObject");
        jsonObjectClass = static_cast<jclass>(env->NewGlobalRef(localJsonObjectClass));
        env->DeleteLocalRef(localJsonObjectClass);

        jclass localJsonArrayClass = env->FindClass("org/json/JSONArray");
        jsonArrayClassRef = static_cast<jclass>(env->NewGlobalRef(localJsonArrayClass));
        env->DeleteLocalRef(localJsonArrayClass);

        integerConstructorMethodID = env->GetMethodID(integerClass, "<init>", "(I)V");
        longConstructorMethodID = env->GetMethodID(longClass, "<init>", "(J)V");
        doubleConstructorMethodID = env->GetMethodID(doubleClass, "<init>", "(D)V");
        booleanConstructorMethodID = env->GetMethodID(booleanClass, "<init>", "(Z)V");

        nullFieldID = env->GetStaticFieldID(jsonObjectClass, "NULL", "Ljava/lang/Object;");
    }


public:
    JSONArrayShadow(JNIEnv* env)
            : jsonArrayClass(nullptr),
              constructorMethodID(nullptr),
              lengthMethodID(nullptr),
              getMethodID(nullptr),
              getJSONObjectMethodID(nullptr),
              getJSONArrayMethodID(nullptr),
              getStringMethodID(nullptr),
              getIntMethodID(nullptr),
              getLongMethodID(nullptr),
              getDoubleMethodID(nullptr),
              getBooleanMethodID(nullptr),
              putMethodID(nullptr),
              stringClass(nullptr),
              integerClass(nullptr),
              longClass(nullptr),
              doubleClass(nullptr),
              floatClass(nullptr),
              booleanClass(nullptr),
              jsonObjectClass(nullptr),
              jsonArrayClassRef(nullptr),
              integerConstructorMethodID(nullptr),
              longConstructorMethodID(nullptr),
              doubleConstructorMethodID(nullptr),
              booleanConstructorMethodID(nullptr),
              nullFieldID(nullptr) {
        if (env == nullptr) return;
        jclass localJsonArrayClass = env->FindClass("org/json/JSONArray");
        if (!localJsonArrayClass || env->ExceptionCheck()) {
          env->ExceptionDescribe();  // <-- print the Java-side exception
          env->ExceptionClear();
          return;
        }
        if (localJsonArrayClass != nullptr) {
            jsonArrayClass = static_cast<jclass>(env->NewGlobalRef(localJsonArrayClass));
            env->DeleteLocalRef(localJsonArrayClass);
            initializeMethodIDs(env);
        }
    }

    jobject create(JNIEnv* env) {
        constructorMethodID = env->GetMethodID(jsonArrayClass, "<init>", "()V");
        return env->NewObject(jsonArrayClass, constructorMethodID);
    }

    int length(JNIEnv* env, jobject jsonArray) {
        if (jsonArray == nullptr || lengthMethodID == nullptr) return -1;
        return env->CallIntMethod(jsonArray, lengthMethodID);
    }

    jobject getJSONObject(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getJSONObjectMethodID == nullptr) return nullptr;
        return env->CallObjectMethod(jsonArray, getJSONObjectMethodID, index);
    }

    jobject getJSONArray(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getJSONArrayMethodID == nullptr) return nullptr;
        return env->CallObjectMethod(jsonArray, getJSONArrayMethodID, index);
    }

    std::string getString(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getStringMethodID == nullptr) return "";
        auto jStr = (jstring)env->CallObjectMethod(jsonArray, getStringMethodID, index);
        if (jStr == nullptr) return "";
        const char* cStr = env->GetStringUTFChars(jStr, nullptr);
        std::string result(cStr);
        env->ReleaseStringUTFChars(jStr, cStr);
        env->DeleteLocalRef(jStr);
        return result;
    }

    int getInt(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getIntMethodID == nullptr) return 0;
        return env->CallIntMethod(jsonArray, getIntMethodID, index);
    }

    int getLong(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getLongMethodID == nullptr) return 0;
        return env->CallLongMethod(jsonArray, getLongMethodID, index);
    }

    double getDouble(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getDoubleMethodID == nullptr) return 0.0;
        return env->CallDoubleMethod(jsonArray, getDoubleMethodID, index);
    }

    bool getBoolean(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr || getBooleanMethodID == nullptr) return false;
        return env->CallBooleanMethod(jsonArray, getBooleanMethodID, index) == JNI_TRUE;
    }

    void put(JNIEnv* env, jobject jsonArray, jobject value) {
        if (jsonArray == nullptr || putMethodID == nullptr) return;

        // If value is nullptr, use JSONObject.NULL
        if (value == nullptr) {
            jfieldID nullFieldID = env->GetStaticFieldID(jsonObjectClass, "NULL", "Ljava/lang/Object;");
            value = env->GetStaticObjectField(jsonObjectClass, nullFieldID);
        }

        env->DeleteLocalRef(env->CallObjectMethod(jsonArray, putMethodID, value));
    }

    void putInt(JNIEnv* env, jobject jsonArray, jint value) {
        jobject intObject = env->NewObject(integerClass, integerConstructorMethodID, value);
        put(env, jsonArray, intObject);
        env->DeleteLocalRef(intObject);
    }

    void putLong(JNIEnv* env, jobject jsonArray, jlong value) {
        jobject longObject = env->NewObject(longClass, longConstructorMethodID, value);
        put(env, jsonArray, longObject);
        env->DeleteLocalRef(longObject);
    }

    void putDouble(JNIEnv* env, jobject jsonArray, jdouble value) {
        jobject doubleObject = env->NewObject(doubleClass, doubleConstructorMethodID, value);
        put(env, jsonArray, doubleObject);
        env->DeleteLocalRef(doubleObject);
    }

    void putBoolean(JNIEnv* env, jobject jsonArray, jboolean value) {
        jobject booleanObject = env->NewObject(booleanClass, booleanConstructorMethodID, value);
        put(env, jsonArray, booleanObject);
        env->DeleteLocalRef(booleanObject);
    }

    int getDataType(JNIEnv* env, jobject jsonArray, int index) {
        if (jsonArray == nullptr) throw std::runtime_error(std::string("jsonArray is null"));

        jobject value = env->CallObjectMethod(jsonArray, getMethodID, index);
        jobject localJsonNullObj = env->GetStaticObjectField(jsonObjectClass, nullFieldID);

        if (value == nullptr) {
            env->DeleteLocalRef(localJsonNullObj);
            throw std::runtime_error(std::string("value is nullptr"));
        }

        int dataType = 0;
        if (env->IsInstanceOf(value, stringClass)) dataType = DATATYPE::STRING;
        else if (env->IsInstanceOf(value, integerClass)) dataType = DATATYPE::INT32;
        else if (env->IsInstanceOf(value, longClass)) dataType = DATATYPE::INT64;
        else if (env->IsInstanceOf(value, doubleClass)) dataType = DATATYPE::DOUBLE;
        else if (env->IsInstanceOf(value, floatClass)) dataType = DATATYPE::FLOAT;
        else if (env->IsInstanceOf(value, booleanClass)) dataType = DATATYPE::BOOLEAN;
        else if (env->IsInstanceOf(value, jsonObjectClass)) dataType = DATATYPE::JSON;
        else if (env->IsInstanceOf(value, jsonArrayClassRef)) dataType = DATATYPE::JSON_ARRAY;
        else if (env->IsSameObject(value, localJsonNullObj)) dataType = DATATYPE::NONE;
        else {
            env->DeleteLocalRef(value);
            env->DeleteLocalRef(localJsonNullObj);
            throw std::runtime_error(std::string("Invalid datatype found in jsonArray"));
        }

        env->DeleteLocalRef(value);
        env->DeleteLocalRef(localJsonNullObj);
        return dataType;
    }
};
