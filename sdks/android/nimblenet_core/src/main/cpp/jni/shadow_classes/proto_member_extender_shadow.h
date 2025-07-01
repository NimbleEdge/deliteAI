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
#include <vector>

#include "../utils/jni_logger.h"
#include "../utils/jni_string.h"

//TODO: free exc
#define THROW_IF_JAVA_EXCEPTION(env)                               \
    if ((env)->ExceptionCheck()) {                                 \
        jthrowable exc = (env)->ExceptionOccurred();               \
        (env)->ExceptionDescribe();                                \
        (env)->ExceptionClear();                                   \
        jclass exceptionClass = env->GetObjectClass(exc);          \
        jmethodID getMessageMethod = env->GetMethodID(exceptionClass, "getMessage", "()Ljava/lang/String;"); \
        jstring message = (jstring)env->CallObjectMethod(exc, getMessageMethod); \
        const char* msgChars = env->GetStringUTFChars(message, nullptr); \
        std::string errorMsg = "Kotlin Call failed: ";             \
        errorMsg += msgChars ? msgChars : "Unknown Java exception";\
        env->ReleaseStringUTFChars(message, msgChars);             \
        env->DeleteLocalRef(message);                              \
        env->DeleteLocalRef(exceptionClass);                       \
        env->DeleteLocalRef(exc);                                  \
        throw std::runtime_error(errorMsg);                        \
    }


class ProtoMemberExtenderShadow {
private:
    static jclass jobjectExtenderClass;
    static jmethodID getMethodId;
    static jmethodID getValueByIndexMethodId;
    static jmethodID getValueByKeyMethodId;
    static jmethodID setValueByIndexMethodId;
    static jmethodID setValueByKeyMethodId;
    static jmethodID getKeysMethodId;
    static jmethodID containsMethodId;
    static jmethodID sizeMethodId;
    static jmethodID arrangeMethodId;
    static jmethodID printMethodId;
    static jmethodID getCoreTypeMethodId;
    static jmethodID popValueByStringMethodId;
    static jmethodID popValueByIndexMethodId;
    static jmethodID appendMethodId;
    static bool isInitialized;

    jobject jobjectExtenderObject;

public:
    static bool is_initialized(){
        return isInitialized;
    }
    static bool init(JNIEnv *env) {
        if (env == nullptr) return isInitialized = false;

        jclass localClass = env->FindClass(
                "ai/deliteai/impl/delitePy/proto/ProtoMemberExtender");
        if (localClass == nullptr) {
            LOGE("Class dev.deliteai.utils.ProtoMemberExtender not found.\n");
            return isInitialized = false;
        }

        jobjectExtenderClass = static_cast<jclass>(env->NewGlobalRef(localClass));
        env->DeleteLocalRef(localClass);
        if (jobjectExtenderClass == nullptr) {
            LOGE("Failed to create global reference for ProtoMemberExtender class.\n");
            return isInitialized = false;
        }

        getMethodId = env->GetMethodID(jobjectExtenderClass, "get", "()Ljava/lang/Object;");
        getValueByIndexMethodId = env->GetMethodID(jobjectExtenderClass, "getValue",
                                                   "(I)Ljava/lang/Object;");
        getValueByKeyMethodId = env->GetMethodID(jobjectExtenderClass, "getValue",
                                                 "(Ljava/lang/String;)Ljava/lang/Object;");
        setValueByIndexMethodId = env->GetMethodID(jobjectExtenderClass, "setValue",
                                                   "(ILjava/lang/Object;)V");
        setValueByKeyMethodId = env->GetMethodID(jobjectExtenderClass, "setValue",
                                                 "(Ljava/lang/String;Ljava/lang/Object;)V");
        getKeysMethodId = env->GetMethodID(jobjectExtenderClass, "getKeys",
                                           "()[Ljava/lang/String;");
        containsMethodId = env->GetMethodID(jobjectExtenderClass, "contains",
                                            "(Ljava/lang/String;)Z");
        sizeMethodId = env->GetMethodID(jobjectExtenderClass, "size", "()I");
        arrangeMethodId = env->GetMethodID(jobjectExtenderClass, "arrange",
                                           "([I)Lai/deliteai/impl/delitePy/proto/impl/ProtoListWrapper;");
        printMethodId = env->GetMethodID(jobjectExtenderClass, "print", "()Ljava/lang/String;");
        getCoreTypeMethodId = env->GetMethodID(jobjectExtenderClass, "getCoreType", "()I");
        popValueByIndexMethodId = env->GetMethodID(jobjectExtenderClass, "pop", "(I)Ljava/lang/Object;");
        popValueByStringMethodId = env->GetMethodID(jobjectExtenderClass, "pop", "(Ljava/lang/String;)Ljava/lang/Object;");
        appendMethodId = env->GetMethodID(jobjectExtenderClass, "append", "(Ljava/lang/Object;)V");


        if (getMethodId == nullptr || getValueByIndexMethodId == nullptr ||
            getValueByKeyMethodId == nullptr || setValueByIndexMethodId == nullptr ||
            setValueByKeyMethodId == nullptr || getKeysMethodId == nullptr ||
            containsMethodId == nullptr || sizeMethodId == nullptr ||
            arrangeMethodId == nullptr || printMethodId == nullptr ||
            getCoreTypeMethodId == nullptr || popValueByStringMethodId == nullptr ||
            popValueByIndexMethodId == nullptr || appendMethodId == nullptr) {
            log_fatal("One or more methods in ProtoMemberExtender class could not be found.\n");
            return isInitialized = false;
        }

        return isInitialized = true;
    }

    ProtoMemberExtenderShadow(JNIEnv *env, jobject obj) {
        jobjectExtenderObject = env->NewGlobalRef(obj);
    }

    ~ProtoMemberExtenderShadow() {
        JNIEnv *env;
        int getEnvStatus = globalJvm->GetEnv((void **)&env, JNI_VERSION_1_6);


        if (jobjectExtenderObject && getEnvStatus != JNI_EDETACHED) {
            env->DeleteGlobalRef(jobjectExtenderObject);
        }
    }

    jobject get_proto_wrapper_jobject() const {
        return jobjectExtenderObject;
    }
    jint getCoreType(JNIEnv *env) {
        if (env == nullptr || jobjectExtenderObject == nullptr || getCoreTypeMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call getCoreType(int).");
        }

        auto ret =  env->CallIntMethod(jobjectExtenderObject, getCoreTypeMethodId);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;
    }

    jobject get(JNIEnv *env) {
        if (env == nullptr || jobjectExtenderObject == nullptr || getMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call get(int).");
        }

        auto ret =  env->CallObjectMethod(jobjectExtenderObject, getMethodId);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;
    }

    jobject getValue(JNIEnv *env, jint index) {
        if (env == nullptr || jobjectExtenderObject == nullptr ||
            getValueByIndexMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call getValue(int).");
        }

        auto ret = env->CallObjectMethod(jobjectExtenderObject, getValueByIndexMethodId, index);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;
    }

    jobject getValue(JNIEnv *env, jstring key) {
        if (env == nullptr || jobjectExtenderObject == nullptr ||
            getValueByKeyMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call getValue(String).");
        }
        auto ret =  env->CallObjectMethod(jobjectExtenderObject, getValueByKeyMethodId, key);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;
    }

    void setValue(JNIEnv *env, jint index, jobject javaObject) {
        if (env == nullptr || jobjectExtenderObject == nullptr ||
            setValueByIndexMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call setValue(int, Object).");
        }
        env->CallVoidMethod(jobjectExtenderObject, setValueByIndexMethodId, index, javaObject);
        THROW_IF_JAVA_EXCEPTION(env);
    }

    void setValue(JNIEnv *env, jstring key, jobject javaObject) {
        if (env == nullptr || jobjectExtenderObject == nullptr ||
            setValueByKeyMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call setValue(String, Object).");
        }
        env->CallVoidMethod(jobjectExtenderObject, setValueByKeyMethodId, key, javaObject);
        THROW_IF_JAVA_EXCEPTION(env);
    }

    std::vector<std::string> getKeys(JNIEnv *env) {
        if (env == nullptr || jobjectExtenderObject == nullptr || getKeysMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call getKeys().");
        }

        jobjectArray keysArray = (jobjectArray) env->CallObjectMethod(jobjectExtenderObject,
                                                                      getKeysMethodId);
        THROW_IF_JAVA_EXCEPTION(env);
        if (keysArray == nullptr) {
            throw std::runtime_error("kotlin object: getKeys failed");
        }

        jsize length = env->GetArrayLength(keysArray);
        std::vector<std::string> keys;
        for (jsize i = 0; i < length; ++i) {
            jstring key = (jstring) env->GetObjectArrayElement(keysArray, i);
            const char *keyStr = env->GetStringUTFChars(key, nullptr);
            keys.emplace_back(keyStr);
            env->ReleaseStringUTFChars(key, keyStr);
            env->DeleteLocalRef(key);
        }

        env->DeleteLocalRef(keysArray);
        return keys;
    }

    bool contains(JNIEnv *env, jstring key) {
        if (env == nullptr || jobjectExtenderObject == nullptr || containsMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call contains(String).");
        }
        auto ret =  env->CallBooleanMethod(jobjectExtenderObject, containsMethodId, key);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;

    }

    int size(JNIEnv *env) {
        if (env == nullptr || jobjectExtenderObject == nullptr || sizeMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call size().");
        }
        auto ret = env->CallIntMethod(jobjectExtenderObject, sizeMethodId);
        THROW_IF_JAVA_EXCEPTION(env);
        return ret;

    }

    std::string print(JNIEnv *env) {
        if (env == nullptr || printMethodId == nullptr) {
            throw std::runtime_error("Invalid state to call print().");
        }

        auto jstr = env->CallObjectMethod(jobjectExtenderObject, printMethodId);
        auto cppStr = JniString::jstringToStdString(env, (jstring) jstr);
        env->DeleteLocalRef(jstr);
        THROW_IF_JAVA_EXCEPTION(env);
        return cppStr;
    }

    jobject arrange(JNIEnv *env, jobject order) {
        if (env == nullptr || arrangeMethodId == nullptr || jobjectExtenderObject == nullptr) {
            throw std::runtime_error("Invalid state to call arrange.");
        }

        jobject resultArray = env->CallObjectMethod(jobjectExtenderObject, arrangeMethodId, order);
        THROW_IF_JAVA_EXCEPTION(env);
        return resultArray;
    }

    jobject pop(JNIEnv *env, jint index) {
        if (env == nullptr || popValueByIndexMethodId == nullptr || jobjectExtenderObject == nullptr) {
            throw std::runtime_error("Invalid state to call pop()");
        }
        jobject value = env->CallObjectMethod(jobjectExtenderObject, popValueByIndexMethodId, index);
        THROW_IF_JAVA_EXCEPTION(env);
        return value;
    }

    jobject pop(JNIEnv *env, jstring key) {
        if (env == nullptr || popValueByStringMethodId == nullptr || jobjectExtenderObject == nullptr) {
            throw std::runtime_error("Invalid state to call pop()");
        }
        jobject value = env->CallObjectMethod(jobjectExtenderObject, popValueByStringMethodId, key);
        THROW_IF_JAVA_EXCEPTION(env);
        return value;
    }

    void append(JNIEnv *env, jobject jobj) {
        if (env == nullptr || appendMethodId == nullptr || jobjectExtenderObject == nullptr) {
            throw std::runtime_error("Invalid state to call append()");
        }
        env->CallVoidMethod(jobjectExtenderObject, appendMethodId, jobj);
        THROW_IF_JAVA_EXCEPTION(env)
    }
};
