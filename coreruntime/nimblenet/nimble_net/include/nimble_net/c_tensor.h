/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "executor_structs.h"
#include "nimble_net/config.h"

DELITEAI_EXPORT_BEGIN

DELITEAI_EXTERN_C_BEGIN

/**
 * @brief Represents a single tensor with its metadata.
 *
 * @note Use this struct to send data to run_method function only if C interop is present,
 * e.g., in iOS. For Android and nimblenet_py, directly use MapDataVariable.
 */
typedef struct CTensor {
  char* name;      /**< Name of the tensor. */
  void* data;      /**< Pointer to the tensor data. */
  int dataType;    /**< Data type of the tensor. */
  int64_t* shape;  /**< Pointer to the tensor's shape dimensions. */
  int shapeLength; /**< Number of dimensions in the shape. */
} CTensor;

/**
 * @brief Wrapper around an array of tensors.
 */
typedef struct CTensors {
  CTensor* tensors; /**< Pointer to an array of tensors. */
  int numTensors;   /**< Number of tensors in the array. */
  int outputIndex;  /**< In case this is an output tensor then index is used to deallocate
                       memory later. */
} CTensors;

// =================================================================================================

void c_tensor_init(CTensor* tensor);
// void c_tensor_deinit(CTensor* tensor);

// =================================================================================================

bool* c_tensor_create_boolean_data(bool value);
bool c_tensor_get_boolean_data(void* data);

// =================================================================================================

int32_t* c_tensor_create_int32_data(int32_t value);
int32_t c_tensor_get_int32_data(void* data);

// =================================================================================================

int64_t* c_tensor_create_int64_data(int64_t value);
int64_t c_tensor_get_int64_data(void* data);

// =================================================================================================

float* c_tensor_create_float_data(float value);
float c_tensor_get_float_data(void* data);

// =================================================================================================

double* c_tensor_create_double_data(double value);
double c_tensor_get_double_data(void* data);

// =================================================================================================

char** c_tensor_create_string_data(const char* str);
char* c_tensor_get_string_data(void* data);

// =================================================================================================

/**
 * @brief A marker type for the user-defined context for a foreign function.
 */
typedef struct CForeignFunctionContext CForeignFunctionContext;

/**
 * @brief Function pointer type for deleting a foreign function context.
 */
typedef void (*CForeignFunctionContextDeleter)(CForeignFunctionContext* context);

/**
 * @brief Function pointer type for a foreign function that can be invoked as a callback from a
 *        DelitePy script.
 *
 * @param context The user-provided context pointer.
 * @param input A struct containing the input tensors.
 * @param output Pointer to a struct where the output tensors should be stored.
 * @return NimbleNetStatus* Status of invoking the foreign function.
 */
typedef NimbleNetStatus* (*CForeignFunctionPtr)(CForeignFunctionContext* context,
                                                const CTensors input, CTensors* output);

/**
 * @brief An opaque type encapsulating a `CForeignFunctionPtr`, a `CForeignFunctionContext*`,
 *        and a `CForeignFunctionContextDeleter`.
 */
typedef struct CForeignFunctionObject CForeignFunctionObject;

/**
 * @brief Creates a CForeignFunctionObject instance encapsulating the provided inputs.
 *
 * The return value can be assigned to `CTensor::data`
 * when the corresponding `CTensor::dataType` value is `DATATYPE::FUNCTION`.
 * It can be deleted using `c_tensor_delete_data(CTensor*)`.
 *
 * @param fn A foreign function pointer.
 * @param context A user-defined context pointer to be passed along to the foreign function.
 * @param context_deleter A function pointer to delete the provided context.
 * @return Pointer to the created CForeignFunctionObject instance.
 */
CForeignFunctionObject* c_tensor_create_function_data(
    CForeignFunctionPtr fn, CForeignFunctionContext* context,
    CForeignFunctionContextDeleter context_deleter);

// =================================================================================================

bool c_tensor_delete_data(CTensor* tensor);

DELITEAI_EXTERN_C_END

DELITEAI_EXPORT_END
