/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "executor_structs.h"
#include "nimble_net_util.hpp"

typedef NimbleNetStatus* (*get_ios_object_string_subscript_type)(IosObject obj, const char* key,
                                                                 CTensor* child);
typedef NimbleNetStatus* (*get_ios_object_int_subscript_type)(IosObject obj, int index,
                                                              CTensor* child);
typedef NimbleNetStatus* (*get_ios_object_size_type)(IosObject obj, int* val);
typedef NimbleNetStatus* (*set_ios_object_string_subscript_type)(IosObject obj, const char* key,
                                                                 CTensor* value);
typedef NimbleNetStatus* (*set_ios_object_int_subscript_type)(IosObject obj, int key,
                                                              CTensor* value);
typedef NimbleNetStatus* (*ios_object_to_string_type)(IosObject obj, char** str);
typedef NimbleNetStatus* (*ios_object_arrange_type)(IosObject obj, const int* indices,
                                                    int numIndices, IosObject* newObj);
typedef NimbleNetStatus* (*in_ios_object_type)(IosObject obj, const char* key, bool* result);
typedef NimbleNetStatus* (*release_ios_object_type)(IosObject obj);
typedef NimbleNetStatus* (*get_keys_ios_object_type)(IosObject obj, CTensor* result);

typedef void (*deallocate_ios_nimblenet_status_type)(NimbleNetStatus* status);
typedef void (*deallocate_frontend_ctensor_type)(CTensor* ctensor);

extern get_ios_object_string_subscript_type get_ios_object_string_subscript_global;
extern get_ios_object_int_subscript_type get_ios_object_int_subscript_global;
extern deallocate_ios_nimblenet_status_type deallocate_ios_nimblenet_status_global;
extern deallocate_frontend_ctensor_type deallocate_frontend_ctensor_global;
extern get_ios_object_size_type get_ios_object_size_global;
extern set_ios_object_string_subscript_type set_ios_object_string_subscript_global;
extern set_ios_object_int_subscript_type set_ios_object_int_subscript_global;
extern ios_object_to_string_type ios_object_to_string_global;
extern ios_object_arrange_type ios_object_arrange_global;
extern in_ios_object_type in_ios_object_global;
extern release_ios_object_type release_ios_object_global;
extern get_keys_ios_object_type get_keys_ios_object_global;

bool deallocate_frontend_tensors(CTensors cTensors);

bool free_frontend_function_context(void* context);
