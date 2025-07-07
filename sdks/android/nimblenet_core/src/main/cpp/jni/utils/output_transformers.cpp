/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "output_transformers.h"

#include "../impl/proto_data_variable.hpp"
#include "../jni_common.h"
#include "mutable_map_shadow.h"

extern MutableMapShadow mutableMapShadow;

jobject convertDataVariableToNimbleNetTensor(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable) {
    const int containerType = dataVariable->get_containerType();
    const int dataType = dataVariable->get_dataType_enum();

    jintArray shapeArray = nullptr;
    if (containerType == CONTAINERTYPE::LIST || containerType == CONTAINERTYPE::VECTOR)
        shapeArray = createIntArrayFromVector(env, dataVariable->get_shape());

    if (containerType == CONTAINERTYPE::SINGLE) {
        switch (dataType) {
            case DATATYPE::INT32: return nimbleNetTensorShadow.createInt(env, dataVariable->get_int32(), shapeArray);
            case DATATYPE::INT64: return nimbleNetTensorShadow.createLong(env, dataVariable->get_int64(), shapeArray);
            case DATATYPE::FLOAT: return nimbleNetTensorShadow.createFloat(env, dataVariable->get_float(), shapeArray);
            case DATATYPE::DOUBLE: return nimbleNetTensorShadow.createDouble(env, dataVariable->get_double(), shapeArray);
            case DATATYPE::BOOLEAN: return nimbleNetTensorShadow.createBoolean(env, dataVariable->get_bool(), shapeArray);
            case DATATYPE::STRING: {
                jstring jstr = env->NewStringUTF(dataVariable->get_string().c_str());
                jobject tensor = nimbleNetTensorShadow.createAny(env, jstr, DATATYPE::STRING, shapeArray);
                env->DeleteLocalRef(jstr);
                return tensor;
            }
            case DATATYPE::FE_OBJ: {
                auto protoDataVariable = std::dynamic_pointer_cast<ProtoDataVariable>(dataVariable);
                jobject globalRef = protoDataVariable->get_proto_shadow()->get_proto_wrapper_jobject();
                jobject tensor = nimbleNetTensorShadow.createAny(env, globalRef, DATATYPE::FE_OBJ, shapeArray);
                return tensor;
            }
            case DATATYPE::NONE: return nimbleNetTensorShadow.createAny(env, nullptr, DATATYPE::NONE, shapeArray);
            default: throw std::runtime_error("Unsupported data type for SINGLE container");
        }
    }

    if (containerType == CONTAINERTYPE::VECTOR) {
        const jsize size = dataVariable->get_numElements();
        auto rawPtr = dataVariable->get_raw_ptr();
        jobject tensorRet = nullptr;
        switch (dataType) {
            case DATATYPE::INT32: {
                jintArray array = env->NewIntArray(size);
                env->SetIntArrayRegion(array, 0, size, reinterpret_cast<const jint *>(rawPtr));
                jobject tensor = nimbleNetTensorShadow.createIntArray(env, array, shapeArray);
                env->DeleteLocalRef(array);
                tensorRet = tensor;
                break;
            }
            case DATATYPE::INT64: {
                jlongArray array = env->NewLongArray(size);
                env->SetLongArrayRegion(array, 0, size, reinterpret_cast<const jlong *>(rawPtr));
                jobject tensor = nimbleNetTensorShadow.createLongArray(env, array, shapeArray);
                env->DeleteLocalRef(array);
                tensorRet = tensor;
                break;
            }
            case DATATYPE::FLOAT: {
                jfloatArray array = env->NewFloatArray(size);
                env->SetFloatArrayRegion(array, 0, size, reinterpret_cast<const jfloat *>(rawPtr));
                jobject tensor = nimbleNetTensorShadow.createFloatArray(env, array, shapeArray);
                env->DeleteLocalRef(array);
                tensorRet = tensor;
                break;
            }
            case DATATYPE::DOUBLE: {
                jdoubleArray array = env->NewDoubleArray(size);
                env->SetDoubleArrayRegion(array, 0, size, reinterpret_cast<const jdouble *>(rawPtr));
                jobject tensor = nimbleNetTensorShadow.createDoubleArray(env, array, shapeArray);
                env->DeleteLocalRef(array);
                tensorRet = tensor;
                break;
            }
            case DATATYPE::BOOLEAN: {
                jbooleanArray array = env->NewBooleanArray(size);
                env->SetBooleanArrayRegion(array, 0, size, reinterpret_cast<const jboolean *>(rawPtr));
                jobject tensor = nimbleNetTensorShadow.createBooleanArray(env, array, shapeArray);
                env->DeleteLocalRef(array);
                tensorRet = tensor;
                break;
            }
            case DATATYPE::STRING: {
                jclass stringClass = env->FindClass("java/lang/String");
                jobjectArray array = env->NewObjectArray(size, stringClass, nullptr);
                auto strings = dataVariable->get_string_ptr();
                for (jsize i = 0; i < size; ++i) {
                    jstring jstr = env->NewStringUTF(strings[i]);
                    env->SetObjectArrayElement(array, i, jstr);
                    env->DeleteLocalRef(jstr);
                }
                jobject tensor = nimbleNetTensorShadow.createAny(env, array, DATATYPE::STRING, shapeArray);
                env->DeleteLocalRef(array);
                env->DeleteLocalRef(stringClass);
                tensorRet = tensor;
                break;
            }
            default: throw std::runtime_error("Unsupported data type for VECTOR container");
        }

        env->DeleteLocalRef(shapeArray);
        return tensorRet;
    }

    if (containerType == CONTAINERTYPE::MAP){
        auto nestedJson = convertDataVariableToJSONObject(env, dataVariable);
        auto ret = nimbleNetTensorShadow.createAny(env, nestedJson, DATATYPE::JSON, shapeArray);
        env->DeleteLocalRef(nestedJson);
        return ret;
    }

    if (containerType == CONTAINERTYPE::LIST){
        auto nestedJsonArray = convertDataVariableToJSONArray(env, dataVariable);
        auto ret = nimbleNetTensorShadow.createAny(env, nestedJsonArray, DATATYPE::JSON_ARRAY, shapeArray);
        env->DeleteLocalRef(nestedJsonArray);
        env->DeleteLocalRef(shapeArray);
        return ret;
    }

    throw std::runtime_error("Unsupported container type");
}

jobject convertDataVariableMapToNimbleNetTensorMap(JNIEnv* env,
                                                   const MapDataVariable& dataVariableMap) {
  auto tensorMap = mutableMapShadow.create(env);

  const auto& map = const_cast<MapDataVariable&>(dataVariableMap).get_map();
  for (const auto& [key, value] : map) {
    jobject tensor = convertDataVariableToNimbleNetTensor(env, value);
    mutableMapShadow.put(env, tensorMap, key, tensor);
    env->DeleteLocalRef(tensor);
  }

  return tensorMap;
}

jobject convertDataVariableToJSONObject(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable) {
    jobject jsonObject = jsonObjectShadow.create(env);
    const auto &map = dataVariable->get_map();
    for (auto it = map.begin(); it != map.end(); ++it) {
        auto key = it->first;
        auto value = it->second;

        if (value == nullptr || value->is_none()) {
            jsonObjectShadow.put(env, jsonObject, key, nullptr);
            continue;
        }

        const int containerType = value->get_containerType();
        const int dataType = value->get_dataType_enum();

        if (containerType == CONTAINERTYPE::SINGLE) {
            switch (dataType) {
                case DATATYPE::INT32:
                case DATATYPE::INT64: jsonObjectShadow.putLong(env, jsonObject, key, value->get_int64()); break;
                case DATATYPE::FLOAT:
                case DATATYPE::DOUBLE: jsonObjectShadow.putDouble(env, jsonObject, key, value->get_double()); break;
                case DATATYPE::BOOLEAN: jsonObjectShadow.putBoolean(env, jsonObject, key, value->get_bool()); break;
                case DATATYPE::STRING: jsonObjectShadow.putString(env, jsonObject, key, value->get_string()); break;
                default: throw std::runtime_error("Unsupported data type for SINGLE container");
            }
        } else if (containerType == CONTAINERTYPE::MAP) {
            {
                auto nestedJson = convertDataVariableToJSONObject(env, value);
                jsonObjectShadow.put(env, jsonObject, key, nestedJson);
                env->DeleteLocalRef(nestedJson);
            }
        } else if (containerType == CONTAINERTYPE::LIST) {
            {
                auto nestedJsonList = convertDataVariableToJSONArray(env, value);
                jsonObjectShadow.put(env, jsonObject, key, nestedJsonList);
                env->DeleteLocalRef(nestedJsonList);
            }
        }

        else throw std::runtime_error("Unsupported containerType");
    }
    return jsonObject;
}

jobject convertDataVariableToJSONArray(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable) {
    jobject jsonArray = jsonArrayShadow.create(env);
    const int size = dataVariable->get_numElements();
    for (int i = 0; i < size; ++i) {
        auto value = dataVariable->get_int_subscript(i);

        if (value == nullptr || value->is_none()) {
            jsonArrayShadow.put(env, jsonArray, nullptr);
            continue;
        }

        const int containerType = value->get_containerType();
        const int dataType = value->get_dataType_enum();

        if (containerType == CONTAINERTYPE::SINGLE) {
            switch (dataType) {
                case DATATYPE::INT32:
                case DATATYPE::INT64: jsonArrayShadow.putLong(env, jsonArray, value->get_int64()); break;
                case DATATYPE::FLOAT:
                case DATATYPE::DOUBLE: jsonArrayShadow.putDouble(env, jsonArray, value->get_double()); break;
                case DATATYPE::BOOLEAN: jsonArrayShadow.putBoolean(env, jsonArray, value->get_bool()); break;
                case DATATYPE::STRING: {
                    auto string = value->get_string();
                    auto str = env->NewStringUTF(string.c_str());
                    jsonArrayShadow.put(env, jsonArray, str);
                    env->DeleteLocalRef(str);
                    break;
                }
                default: throw std::runtime_error("Unsupported data type for SINGLE container");
            }
        } else if (containerType == CONTAINERTYPE::MAP) {
            {
                auto nestedJson = convertDataVariableToJSONObject(env, value);
                jsonArrayShadow.put(env, jsonArray, nestedJson);
                env->DeleteLocalRef(nestedJson);
            }
        } else if (containerType == CONTAINERTYPE::LIST) {
            {
                auto nestedJsonList = convertDataVariableToJSONArray(env, value);
                jsonArrayShadow.put(env, jsonArray, nestedJsonList);
                env->DeleteLocalRef(nestedJsonList);
            }
        }
        else throw std::runtime_error("Unsupported containerType");
    }
    return jsonArray;
}
