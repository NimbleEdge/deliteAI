/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ANDROID_SDK_OUTPUT_TRANSFORMERS_H
#define ANDROID_SDK_OUTPUT_TRANSFORMERS_H

#include <jni.h>

#include <memory>

#include "json_array_shadow.h"
#include "json_object_shadow.h"
#include "map_data_variable.hpp"
#include "nimble_net_tensor_shadow.h"
#include "type_caster_shadow.h"

extern TypeCasterShadow typeCasterShadow;
extern NimbleNetTensorShadow nimbleNetTensorShadow;
extern JSONObjectShadow jsonObjectShadow;
extern JSONArrayShadow jsonArrayShadow;

jobject convertDataVariableToJSONObject(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable);
jobject convertDataVariableToJSONArray(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable);

jobject convertDataVariableToNimbleNetTensor(JNIEnv *env, const std::shared_ptr<DataVariable> dataVariable);
jobject convertDataVariableMapToNimbleNetTensorMap(JNIEnv* env,
                                                   const MapDataVariable& dataVariableMap);

#endif  // ANDROID_SDK_OUTPUT_TRANSFORMERS_H
