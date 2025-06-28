/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_SDK_INPUT_TRANSFORMERS_H
#define ANDROID_SDK_INPUT_TRANSFORMERS_H

#include <jni.h>

#include <cstdint>
#include <vector>

#include "json_array_shadow.h"
#include "json_object_shadow.h"
#include "map_data_variable.hpp"
#include "ne_fwd.hpp"
#include "type_caster_shadow.h"

extern TypeCasterShadow typeCasterShadow;
extern JSONObjectShadow jsonObjectShadow;
extern JSONArrayShadow jsonArrayShadow;

OpReturnType convertJSONArrayToOpReturnType(JNIEnv *env, jobject jsonArrayKotlin);
OpReturnType convertJSONObjectToOpReturnType(JNIEnv *env, jobject jsonObjectKotlin);
OpReturnType convertNimbleNetTensorToOpReturnType(JNIEnv* env, jobject tensor);
OpReturnType convertSingularKotlinDataToOpReturnType(JNIEnv *env, jobject tensorDataKotlin, int dataTypeInt);
OpReturnType convertTensorKotlinDataToOpReturnType(JNIEnv *env, jobject tensorDataKotlin, int dataTypeInt, jintArray shapeArrayJNI);
std::shared_ptr<MapDataVariable> convertNimbleNetTensorMapToDataVariableMap(JNIEnv* env,
                                                                            jobject tensorMap);
std::vector<int64_t> convertJIntArrayToInt64Vector(JNIEnv *env, jintArray value);

#endif  // ANDROID_SDK_INPUT_TRANSFORMERS_H
