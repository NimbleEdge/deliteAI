/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "input_transformers.h"

#include <utility>

#include "../impl/proto_data_variable.hpp"
#include "custom_func_data_variable.hpp"
#include "dljni/dljni.hpp"
#include "mutable_map_shadow.h"

extern JavaVM* globalJvm;
extern MutableMapShadow mutableMapShadow;

namespace {
std::shared_ptr<MapDataVariable> createForeignFunctionArgMap(
    const std::vector<OpReturnType>& args) {
  if (args.empty()) {
    return nullptr;
  }

  if (args.size() != 1) {
    THROW("calling foreign function: num args: expected = 0 or 1, actual = %zu", args.size());
  }

  const auto& arg = args[0];
  if (arg->get_containerType() != CONTAINERTYPE::MAP) {
    THROW("calling foreign function: arg container type: expected = \"Map\", actual = \"%s\"",
          arg->get_containerType_string());
  }
  return std::dynamic_pointer_cast<MapDataVariable>(arg);
}

auto invokeForeignJvmFunction(JNIEnv* env, jobject jvmFn, jobject arg) {
  // TODO (puneet): Create shadow class for "kotlin.jvm.functions.Function1"?
  auto clazz = make_jobjectlocalref(env, env->FindClass("kotlin/jvm/functions/Function1"));
  jmethodID methodId =
      env->GetMethodID(clazz.get(), "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");
  auto res = make_jobjectlocalref(env, env->CallObjectMethod(jvmFn, methodId, arg));
  if (env->ExceptionCheck()) {
    // TODO (puneet): Devise a mechanism to propagate this information to DelitePy.
    // env->ExceptionDescribe();
    // env->ExceptionOccurred();
    env->ExceptionClear();
  }
  return std::move(res);
}

CustomFuncDataVariable createForeignFunctionDataVariable(JNIEnv* env, jobject jvmFn) {
  auto jvmFnGlobalRef = make_jobjectglobalref(env, jvmFn);
  auto jvmFnGlobalRefShared =
      std::shared_ptr<decltype(jvmFnGlobalRef)::element_type>(std::move(jvmFnGlobalRef));
  return CustomFuncDataVariable([jvmFn = jvmFnGlobalRefShared](
                                    const std::vector<OpReturnType>& args,
                                    CallStack& stack) -> OpReturnType {
    if (jvmFn == nullptr) {
      LOGD("invoked foreign function is null");
      return nullptr;
    }

    auto threadAttacher = DlJniCurrentThreadAttacher(*globalJvm);
    if (threadAttacher.notAttached()) {
      return nullptr;
    }

    JNIEnv* env = threadAttacher.getEnv();
    std::shared_ptr<MapDataVariable> argMap = createForeignFunctionArgMap(args);
    auto argTensorMap = make_jobjectlocalref(
        env, argMap ? convertDataVariableMapToNimbleNetTensorMap(env, *argMap) : nullptr);
    auto resTensorMap = invokeForeignJvmFunction(env, jvmFn.get(), argTensorMap.get());
    auto resDataVariableMap = convertNimbleNetTensorMapToDataVariableMap(env, resTensorMap.get());
    return resDataVariableMap;
  });
}
}  // anonymous namespace

OpReturnType convertNimbleNetTensorToOpReturnType(JNIEnv* env, jobject tensor) {
  OpReturnType dataVariable;

  auto tensorData = make_jobjectlocalref(env, nimbleNetTensorShadow.getData(env, tensor));
  auto tensorDataType = nimbleNetTensorShadow.getDatatypeInt(env, tensor);
  auto tensorShape = make_jobjectlocalref(env, nimbleNetTensorShadow.getShape(env, tensor));
  auto tensorShapeArrayLength = nimbleNetTensorShadow.getShapeArrayLength(env, tensor);

  if (tensorShapeArrayLength == 0) {
    dataVariable = convertSingularKotlinDataToOpReturnType(env, tensorData.get(), tensorDataType);
  } else {
    dataVariable = convertTensorKotlinDataToOpReturnType(env, tensorData.get(), tensorDataType,
                                                         tensorShape.get());
  }

  return dataVariable;
}

OpReturnType convertSingularKotlinDataToOpReturnType(JNIEnv *env, jobject tensorDataKotlin, int dataTypeInt) {
    switch (dataTypeInt) {
        case DATATYPE::INT32: return std::make_shared<SingleVariable<int32_t>>(typeCasterShadow.castToInt(env, tensorDataKotlin));
        case DATATYPE::INT64: return std::make_shared<SingleVariable<int64_t>>(typeCasterShadow.castToLong(env, tensorDataKotlin));
        case DATATYPE::FLOAT: return std::make_shared<SingleVariable<float>>(typeCasterShadow.castToFloat(env, tensorDataKotlin));
        case DATATYPE::DOUBLE: return std::make_shared<SingleVariable<double>>(typeCasterShadow.castToDouble(env, tensorDataKotlin));
        case DATATYPE::BOOLEAN: return std::make_shared<SingleVariable<bool>>(typeCasterShadow.castToBoolean(env, tensorDataKotlin));
        case DATATYPE::STRING: return std::make_shared<SingleVariable<std::string>>(typeCasterShadow.castToString(env, tensorDataKotlin));
        case DATATYPE::FUNCTION:
          return std::make_shared<CustomFuncDataVariable>(
              createForeignFunctionDataVariable(env, tensorDataKotlin));

        // use same function for int8 and uint8 as uint8 interop is not supported on kotlin-jni and cast to uint8 while passing to C
        case DATATYPE::JSON: return convertJSONObjectToOpReturnType(env, tensorDataKotlin);
        case DATATYPE::FE_OBJ: return std::make_shared<ProtoDataVariable>(env, tensorDataKotlin);
        case DATATYPE::JSON_ARRAY: throw std::runtime_error("invalid singular input. please specify input shape for json array.");
        default: throw std::runtime_error("convertSingularKotlinDataToOpReturnType: Unsupported data type");
    }
}

OpReturnType
convertTensorKotlinDataToOpReturnType(JNIEnv *env, jobject tensorDataKotlin, int dataTypeInt,
                                      jintArray shapeArrayJNI) {
    //input wrapped in an array
    auto shapeVector = convertJIntArrayToInt64Vector(env, shapeArrayJNI);
    int dimsLength = env->GetArrayLength(shapeArrayJNI);
    OpReturnType tensorC;

    switch (dataTypeInt) {
        case DATATYPE::INT32: {
            auto intArrayJNI = reinterpret_cast<jintArray>(tensorDataKotlin);
            auto *elements = env->GetIntArrayElements(intArrayJNI, nullptr);
            auto ret = TensorVariable::copy_tensor_from_raw_data(elements, static_cast<DATATYPE>(dataTypeInt), shapeVector);
            // JNI_ABORT frees the buffer without copying
            env->ReleaseIntArrayElements(intArrayJNI,elements,JNI_ABORT);
            return ret;
        }
        case DATATYPE::INT64: {
            auto longArrayJNI = reinterpret_cast<jlongArray>(tensorDataKotlin);
            auto *elements = env->GetLongArrayElements(longArrayJNI, nullptr);
            auto ret = TensorVariable::copy_tensor_from_raw_data(elements, static_cast<DATATYPE>(dataTypeInt), shapeVector);
            env->ReleaseLongArrayElements(longArrayJNI,elements,JNI_ABORT);
            return ret;
        }
        case DATATYPE::FLOAT: {
            auto floatArrayJNI = reinterpret_cast<jfloatArray>(tensorDataKotlin);
            auto *elements = env->GetFloatArrayElements(floatArrayJNI, nullptr);
            auto ret = TensorVariable::copy_tensor_from_raw_data(elements, static_cast<DATATYPE>(dataTypeInt), shapeVector);
            env->ReleaseFloatArrayElements(floatArrayJNI,elements,JNI_ABORT);
            return ret;        }
        case DATATYPE::DOUBLE: {
            auto doubleArrayJNI = reinterpret_cast<jdoubleArray>(tensorDataKotlin);
            auto *elements = env->GetDoubleArrayElements(doubleArrayJNI, nullptr);
            auto ret = TensorVariable::copy_tensor_from_raw_data(elements, static_cast<DATATYPE>(dataTypeInt), shapeVector);
            env->ReleaseDoubleArrayElements(doubleArrayJNI,elements,JNI_ABORT);
            return ret;        }
        case DATATYPE::BOOLEAN: {
            auto boolArrayJNI = reinterpret_cast<jbooleanArray>(tensorDataKotlin);
            auto *elements = env->GetBooleanArrayElements(boolArrayJNI, nullptr);
            // jboolean is actually unsigned char (== uint8_t). We don't have a UINT8 DATATYPE, so we do this cast
            // The reinterpret_cast is asserted to be safe as the size of jboolean and bool is same
            static_assert(sizeof(jboolean) == sizeof(bool));
            auto ret = TensorVariable::copy_tensor_from_raw_data(reinterpret_cast<bool*>(elements), static_cast<DATATYPE>(dataTypeInt), shapeVector);
            env->ReleaseBooleanArrayElements(boolArrayJNI,elements,JNI_ABORT);
            return ret;
        }

        case DATATYPE::STRING: {
            auto stringArrayJNI = reinterpret_cast<jobjectArray>(tensorDataKotlin);
            jsize length = env->GetArrayLength(stringArrayJNI);
            std::vector<std::string> stringVector;

            for (jsize i = 0; i < length; ++i) {
                jstring jstr = (jstring) env->GetObjectArrayElement(stringArrayJNI, i);
                const char *cstr = env->GetStringUTFChars(jstr, nullptr);
                stringVector.push_back(cstr);
                env->ReleaseStringUTFChars(jstr, cstr);
                env->DeleteLocalRef(jstr);
            }

            return std::make_shared<StringTensorVariable>(std::move(stringVector),
                                                          std::move(shapeVector), dimsLength);
        }
        case DATATYPE::JSON: {
            throw std::runtime_error("invalid non-singular input. please specify input shape as null for json object.");
        }
        case DATATYPE::JSON_ARRAY: {
            return convertJSONArrayToOpReturnType(env, tensorDataKotlin);
        }
        default:
            throw std::runtime_error("convertTensorKotlinDataToOpReturnType: Unsupported data type");
    }

    return tensorC;
}

std::shared_ptr<MapDataVariable> convertNimbleNetTensorMapToDataVariableMap(JNIEnv* env,
                                                                            jobject tensorMap) {
  auto dataVariableMap = std::make_shared<MapDataVariable>();

  const auto keys = mutableMapShadow.getKeys(env, tensorMap);
  for (const std::string& key : keys) {
    auto tensor = make_jobjectlocalref(env, mutableMapShadow.get(env, tensorMap, key));
    dataVariableMap->set_value_in_map(key, convertNimbleNetTensorToOpReturnType(env, tensor.get()));
  }

  return dataVariableMap;
}

//not used externally
std::vector<int64_t> convertJIntArrayToInt64Vector(JNIEnv *env, jintArray value) {
    jsize length = env->GetArrayLength(value);
    jint *elements = env->GetIntArrayElements(value, nullptr);
    std::vector<int64_t> int64Vector(length);
    for (jsize i = 0; i < length; ++i) {
        int64Vector[i] = static_cast<int64_t>(elements[i]);
    }
    env->ReleaseIntArrayElements(value, elements, JNI_ABORT);
    return int64Vector;
}

OpReturnType convertJSONArrayToOpReturnType(JNIEnv *env, jobject jsonArrayKotlin) {
    OpReturnType list = std::make_shared<ListDataVariable>();
    auto length = jsonArrayShadow.length(env, jsonArrayKotlin);

    for (int i = 0; i < length; i++) {
        auto dataTypeInt = jsonArrayShadow.getDataType(env, jsonArrayKotlin, i);

        switch (dataTypeInt) {
            //will be triggered only in the case of kotlin NULL
            case DATATYPE::NONE: {
                list->append(std::make_shared<NoneVariable>());
                break;
            }
            case DATATYPE::INT32:
            case DATATYPE::INT64: {
                jlong value = jsonArrayShadow.getLong(env, jsonArrayKotlin, i);
                list->append(std::make_shared<SingleVariable<int64_t>>(value));
                break;
            }
            case DATATYPE::FLOAT:
            case DATATYPE::DOUBLE: {
                jdouble value = jsonArrayShadow.getDouble(env, jsonArrayKotlin, i);
                list->append(std::make_shared<SingleVariable<double>>(value));
                break;
            }
            case DATATYPE::BOOLEAN: {
                jboolean value = jsonArrayShadow.getBoolean(env, jsonArrayKotlin, i);
                list->append(std::make_shared<SingleVariable<bool>>(value));
                break;
            }
            case DATATYPE::STRING: {
                std::string value = jsonArrayShadow.getString(env, jsonArrayKotlin, i);
                list->append(std::make_shared<SingleVariable<std::string>>(value));
                break;
            }
            case DATATYPE::JSON: {
                jobject jsonObject = jsonArrayShadow.getJSONObject(env, jsonArrayKotlin, i);
                OpReturnType nestedValue = convertJSONObjectToOpReturnType(env, jsonObject);
                list->append(nestedValue);
                env->DeleteLocalRef(jsonObject);
                break;
            }
            case DATATYPE::JSON_ARRAY: {
                jobject jsonArray = jsonArrayShadow.getJSONArray(env, jsonArrayKotlin, i);
                list->append(convertJSONArrayToOpReturnType(env, jsonArray));
                env->DeleteLocalRef(jsonArray);
                break;
            }
            default:
                throw std::runtime_error("convertJSONArrayToOpReturnType: Unsupported data type");
        }
    }

    return list;
}

OpReturnType convertJSONObjectToOpReturnType(JNIEnv *env, jobject jsonObjectKotlin) {
    OpReturnType map = std::make_shared<MapDataVariable>();

    auto keys = jsonObjectShadow.keys(env, jsonObjectKotlin);
    for (const std::string &key: keys) {
        auto dataTypeInt = jsonObjectShadow.getDataType(env, jsonObjectKotlin, key);

        switch (dataTypeInt) {
            //will be triggered only in the case of kotlin NULL
            case DATATYPE::NONE: {
                map->set_value_in_map(key,  std::make_shared<NoneVariable>());
                break;
            }
            case DATATYPE::INT32:
            case DATATYPE::INT64: {
                auto value = jsonObjectShadow.getLong(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, std::make_shared<SingleVariable<int64_t>>(value));
                break;
            }
            case DATATYPE::FLOAT:
            case DATATYPE::DOUBLE: {
                auto value = jsonObjectShadow.getDouble(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, std::make_shared<SingleVariable<double>>(value));
                break;
            }
            case DATATYPE::BOOLEAN: {
                auto value = jsonObjectShadow.getBoolean(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, std::make_shared<SingleVariable<bool>>(value));
                break;
            }
            case DATATYPE::STRING: {
                auto value = jsonObjectShadow.getString(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, std::make_shared<SingleVariable<std::string>>(value));
                break;
            }
            case DATATYPE::JSON: {
                auto value = jsonObjectShadow.getJSONObject(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, convertJSONObjectToOpReturnType(env, value));
                env->DeleteLocalRef(value);
                break;
            }
            case DATATYPE::JSON_ARRAY: {
                auto value = jsonObjectShadow.getJSONArray(env, jsonObjectKotlin, key);
                map->set_value_in_map(key, convertJSONArrayToOpReturnType(env, value));
                env->DeleteLocalRef(value);
                break;
            }
            default:
                throw std::runtime_error("convertJSONObjectToOpReturnType: Unsupported data type");
        }
    }
    return map;
}
