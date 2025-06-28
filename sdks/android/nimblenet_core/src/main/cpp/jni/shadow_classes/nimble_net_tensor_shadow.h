/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <jni.h>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../utils/jni_logger.h"

class NimbleNetTensorShadow {
private:
    jclass tensorClass;
    std::unordered_map<std::string, jmethodID> constructors; // Map to store constructor method IDs

    jfieldID dataFieldId;
    jfieldID datatypeFieldId;
    jfieldID shapeFieldId;

    jmethodID getDatatypeIntMethodId;
    jmethodID getShapeArrayLengthMethodId;

public:
    NimbleNetTensorShadow(JNIEnv *env) {
        if (env == nullptr) {
            return;
        }

        jclass localClass = env->FindClass("ai/nimbleedge/datamodels/NimbleNetTensor");
        if (localClass == nullptr) {
            LOGE("Class ai.nimbleedge.datamodels.NimbleNetTensor not found.\n");
            return;
        }
        tensorClass = (jclass)env->NewGlobalRef(localClass);
        env->DeleteLocalRef(localClass);

        if (tensorClass == nullptr) {
            LOGE("Failed to create global reference for NimbleNetTensor class.\n");
            return;
        }

        constructors["Int"] = env->GetMethodID(tensorClass, "<init>", "(I[I)V");
        constructors["Long"] = env->GetMethodID(tensorClass, "<init>", "(J[I)V");
        constructors["Float"] = env->GetMethodID(tensorClass, "<init>", "(F[I)V");
        constructors["Double"] = env->GetMethodID(tensorClass, "<init>", "(D[I)V");
        constructors["Boolean"] = env->GetMethodID(tensorClass, "<init>", "(Z[I)V");
        constructors["String"] = env->GetMethodID(tensorClass, "<init>", "(Ljava/lang/String;[I)V");
        constructors["IntArray"] = env->GetMethodID(tensorClass, "<init>", "([I[I)V");
        constructors["LongArray"] = env->GetMethodID(tensorClass, "<init>", "([J[I)V");
        constructors["FloatArray"] = env->GetMethodID(tensorClass, "<init>", "([F[I)V");
        constructors["DoubleArray"] = env->GetMethodID(tensorClass, "<init>", "([D[I)V");
        constructors["BooleanArray"] = env->GetMethodID(tensorClass, "<init>", "([Z[I)V");
        constructors["Any"] = env->GetMethodID(tensorClass, "<init>", "(Ljava/lang/Object;I[I)V");

        for (const auto& pair : constructors) {
            if (pair.second == nullptr) {
                LOGE("Constructor for type %s not found.\n", pair.first.c_str());
            }
        }

        dataFieldId = env->GetFieldID(tensorClass, "data", "Ljava/lang/Object;");
        datatypeFieldId = env->GetFieldID(tensorClass, "datatype", "Lai/nimbleedge/impl/common/DATATYPE;");
        shapeFieldId = env->GetFieldID(tensorClass, "shape", "[I");

        if (dataFieldId == nullptr || datatypeFieldId == nullptr || shapeFieldId == nullptr) {
            LOGE("One or more field IDs not found in NimbleNetTensor class.\n");
        }

        getDatatypeIntMethodId = env->GetMethodID(tensorClass, "getDatatypeInt", "()I");
        getShapeArrayLengthMethodId = env->GetMethodID(tensorClass, "getShapeArrayLength", "()I");

        if (getDatatypeIntMethodId == nullptr || getShapeArrayLengthMethodId == nullptr) {
            LOGE("One or more method IDs not found in NimbleNetTensor class.\n");
        }
    }

    jobject createInt(JNIEnv *env, jint data, jintArray shape) {
        jmethodID constructor = constructors["Int"];
        if (constructor == nullptr) {
            LOGE("Constructor for Int not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createLong(JNIEnv *env, jlong data, jintArray shape) {
        jmethodID constructor = constructors["Long"];
        if (constructor == nullptr) {
            LOGE("Constructor for Long not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createFloat(JNIEnv *env, jfloat data, jintArray shape) {
        jmethodID constructor = constructors["Float"];
        if (constructor == nullptr) {
            LOGE("Constructor for Float not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createDouble(JNIEnv *env, jdouble data, jintArray shape) {
        jmethodID constructor = constructors["Double"];
        if (constructor == nullptr) {
            LOGE("Constructor for Double not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createBoolean(JNIEnv *env, jboolean data, jintArray shape) {
        jmethodID constructor = constructors["Boolean"];
        if (constructor == nullptr) {
            LOGE("Constructor for Boolean not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createIntArray(JNIEnv *env, jintArray data, jintArray shape) {
        jmethodID constructor = constructors["IntArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for IntArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createLongArray(JNIEnv *env, jlongArray data, jintArray shape) {
        jmethodID constructor = constructors["LongArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for LongArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createFloatArray(JNIEnv *env, jfloatArray data, jintArray shape) {
        jmethodID constructor = constructors["FloatArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for FloatArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createDoubleArray(JNIEnv *env, jdoubleArray data, jintArray shape) {
        jmethodID constructor = constructors["DoubleArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for DoubleArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createBooleanArray(JNIEnv *env, jbooleanArray data, jintArray shape) {
        jmethodID constructor = constructors["BooleanArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for BooleanArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createStringArray(JNIEnv *env, jobjectArray data, jintArray shape) {
        jmethodID constructor = constructors["StringArray"];
        if (constructor == nullptr) {
            LOGE("Constructor for StringArray not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, shape);
    }

    jobject createAny(JNIEnv *env, jobject data, jint datatypeInt, jintArray shape) {
        jmethodID constructor = constructors["Any"];
        if (constructor == nullptr) {
            LOGE("Constructor for Any not found.\n");
            return nullptr;
        }
        return env->NewObject(tensorClass, constructor, data, datatypeInt, shape);
    }

    // Field Accessors

    jobject getData(JNIEnv *env, jobject tensorObj) {
        if (tensorObj == nullptr) {
            LOGE("tensorObj is null in getData.\n");
            return nullptr;
        }
        return env->GetObjectField(tensorObj, dataFieldId);
    }

    jobject getDatatype(JNIEnv *env, jobject tensorObj) {
        if (tensorObj == nullptr) {
            LOGE("tensorObj is null in getDatatype.\n");
            return nullptr;
        }
        return env->GetObjectField(tensorObj, datatypeFieldId);
    }

    jintArray getShape(JNIEnv *env, jobject tensorObj) {
        if (tensorObj == nullptr) {
            LOGE("tensorObj is null in getShape.\n");
            return nullptr;
        }
        return (jintArray) env->GetObjectField(tensorObj, shapeFieldId);
    }

    jint getDatatypeInt(JNIEnv *env, jobject tensorObj) {
        if (tensorObj == nullptr) {
            LOGE("tensorObj is null in getDatatypeInt.\n");
            return -1;
        }
        return env->CallIntMethod(tensorObj, getDatatypeIntMethodId);
    }

    jint getShapeArrayLength(JNIEnv *env, jobject tensorObj) {
        if (tensorObj == nullptr) {
            LOGE("tensorObj is null in getShapeArrayLength.\n");
            return -1;
        }
        return env->CallIntMethod(tensorObj, getShapeArrayLengthMethodId);
    }
};
