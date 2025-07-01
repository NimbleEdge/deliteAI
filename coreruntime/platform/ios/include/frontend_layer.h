/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "executor_structs.h"
#include "nimble_net_util.hpp"

/**
 * @brief Function pointer type for getting a string subscript from an iOS object.
 *
 * @param obj The iOS object.
 * @param key The key to access.
 * @param child Output tensor for the result.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*get_ios_object_string_subscript_type)(IosObject obj, const char* key,
                                                                 CTensor* child);
/**
 * @brief Function pointer type for getting an integer subscript from an iOS object.
 *
 * @param obj The iOS object.
 * @param index The index to access.
 * @param child Output tensor for the result.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*get_ios_object_int_subscript_type)(IosObject obj, int index,
                                                              CTensor* child);
/**
 * @brief Function pointer type for getting the size of an iOS object.
 *
 * @param obj The iOS object.
 * @param val Output for the size.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*get_ios_object_size_type)(IosObject obj, int* val);
/**
 * @brief Function pointer type for setting a string subscript in an iOS object.
 *
 * @param obj The iOS object.
 * @param key The key to set.
 * @param value The value to assign.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*set_ios_object_string_subscript_type)(IosObject obj, const char* key,
                                                                 CTensor* value);
/**
 * @brief Function pointer type for setting an integer subscript in an iOS object.
 *
 * @param obj The iOS object.
 * @param key The index to set.
 * @param value The value to assign.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*set_ios_object_int_subscript_type)(IosObject obj, int key,
                                                              CTensor* value);
/**
 * @brief Function pointer type for converting an iOS object to a string.
 *
 * @param obj The iOS object.
 * @param str Output string pointer.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*ios_object_to_string_type)(IosObject obj, char** str);
/**
 * @brief Function pointer type for arranging an iOS object by indices.
 *
 * @param obj The iOS object.
 * @param indices Array of indices.
 * @param numIndices Number of indices.
 * @param newObj Output for the arranged object.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*ios_object_arrange_type)(IosObject obj, const int* indices,
                                                    int numIndices, IosObject* newObj);
/**
 * @brief Function pointer type for checking if a key is in an iOS object.
 *
 * @param obj The iOS object.
 * @param key The key to check.
 * @param result Output boolean result.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*in_ios_object_type)(IosObject obj, const char* key, bool* result);
/**
 * @brief Function pointer type for releasing an iOS object.
 *
 * @param obj The iOS object to release.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*release_ios_object_type)(IosObject obj);
/**
 * @brief Function pointer type for getting keys from an iOS object.
 *
 * @param obj The iOS object.
 * @param result Output tensor for the keys.
 * @return NimbleNetStatus* Status of the operation.
 */
typedef NimbleNetStatus* (*get_keys_ios_object_type)(IosObject obj, CTensor* result);

/**
 * @brief Function pointer type for deallocating a NimbleNetStatus object.
 *
 * @param status Pointer to the status to deallocate.
 */
typedef void (*deallocate_ios_nimblenet_status_type)(NimbleNetStatus* status);
/**
 * @brief Function pointer type for deallocating a frontend CTensor.
 *
 * @param ctensor Pointer to the tensor to deallocate.
 */
typedef void (*deallocate_frontend_ctensor_type)(CTensor* ctensor);

extern get_ios_object_string_subscript_type get_ios_object_string_subscript_global; /**< Global function pointer for string subscript. */
extern get_ios_object_int_subscript_type get_ios_object_int_subscript_global; /**< Global function pointer for int subscript. */
extern deallocate_ios_nimblenet_status_type deallocate_ios_nimblenet_status_global; /**< Global function pointer for NimbleNetStatus deallocation. */
extern deallocate_frontend_ctensor_type deallocate_frontend_ctensor_global; /**< Global function pointer for CTensor deallocation. */
extern get_ios_object_size_type get_ios_object_size_global; /**< Global function pointer for size. */
extern set_ios_object_string_subscript_type set_ios_object_string_subscript_global; /**< Global function pointer for string subscript set. */
extern set_ios_object_int_subscript_type set_ios_object_int_subscript_global; /**< Global function pointer for int subscript set. */
extern ios_object_to_string_type ios_object_to_string_global; /**< Global function pointer for to_string. */
extern ios_object_arrange_type ios_object_arrange_global; /**< Global function pointer for arrange. */
extern in_ios_object_type in_ios_object_global; /**< Global function pointer for in. */
extern release_ios_object_type release_ios_object_global; /**< Global function pointer for release. */
extern get_keys_ios_object_type get_keys_ios_object_global; /**< Global function pointer for get_keys. */

/**
 * @brief Deallocates memory for frontend tensors (iOS implementation).
 *
 * @param cTensors Struct containing tensors to deallocate.
 * @return bool True if successful.
 */
bool deallocate_frontend_tensors(CTensors cTensors);

/**
 * @brief Frees the memory for a frontend function context (iOS implementation).
 *
 * @param context Pointer to the context to free.
 * @return bool True if successful.
 */
bool free_frontend_function_context(void* context);
