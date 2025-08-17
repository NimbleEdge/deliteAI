/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Represents the status of the model whether it is loaded or not.
 */
typedef struct ModelStatus {
  bool isModelReady; /**< Flag indicating if the model is ready for inference. */
  char* version;     /**< String representing the version of the model. */
} ModelStatus;

/**
 * @brief Represents a single input for inference.
 */
typedef struct CUserInput {
  void* data;   /**< Pointer to the raw input data. */
  int length;   /**< Length of the input data. */
  char* name;   /**< Name of the input tensor. */
  int dataType; /**< Data type of the input. */
} CUserInput;

/**
 * @brief Represents an inference request containing one or more inputs.
 */
typedef struct InferenceRequest {
  int numInputs;      /**< Number of input tensors. */
  CUserInput* inputs; /**< Pointer to the array of input tensors. */
} InferenceRequest;

/**
 * @brief Represents the output returned by an inference operation.
 */
typedef struct InferenceReturn {
  void** outputs;          /**< Array of pointers to output data. */
  int** outputShapes;      /**< Array of output shapes corresponding to outputs. */
  int* outputLengths;      /**< Array of output data lengths. */
  int* outputShapeLengths; /**< Array of lengths of each output shape. */
  char** outputNames;      /**< Array of names corresponding to output tensors. */
  int* outputTypes;        /**< Array of data types for the outputs. */
  int numOutputs;          /**< Number of outputs returned. */
} InferenceReturn;

/**
 * @brief Status structure used to indicate success or failure in all SDK APIs.
 */
typedef struct NimbleNetStatus {
  char* message; /**< Message describing the status. */
  int code;      /**< Integer status code. */
} NimbleNetStatus;

/**
 * @brief Structure used to represent user-defined event data.
 */
typedef struct CUserEventsData {
  char* eventType;       /**< Type of the event. */
  char* eventJsonString; /**< JSON string containing event data. */
} CUserEventsData;

/**
 * @brief Represents JSON structured output exposed via C APIs in iOS.
 */
typedef struct JsonOutput {
  int dataType;    /**< Data type of the value (int, double, bool, string, object). */
  const char* key; /**< Key associated with the value. */
  bool isEnd;      /**< Flag indicating end of array/object iteration. */

  /**
   * @brief Union holding actual data depending on the data type.
   */
  union {
    int64_t i;       /**< Integer value. */
    double d;        /**< Double (floating point) value. */
    bool b;          /**< Boolean value. */
    const char* s;   /**< String value. */
    const void* obj; /**< Pointer to JSON iterator object. */
  } value;
} JsonOutput;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @brief Frees the memory allocated for an InferenceReturn object.
 *
 * @param ret Pointer to the InferenceReturn to be deallocated.
 */
__attribute__((visibility("default"))) static inline void deallocate_output_memory(
    InferenceReturn* ret) {
  for (int i = 0; i < ret->numOutputs; i++) {
    free(ret->outputs[i]);
    free(ret->outputShapes[i]);
  }
  free(ret->outputs);
  free(ret->outputNames);
  free(ret->outputTypes);
  free(ret->outputShapes);
  free(ret->outputLengths);
  free(ret->outputShapeLengths);
}

/**
 * @brief Frees the memory allocated for a NimbleNetStatus object.
 *
 * @param status Pointer to the NimbleNetStatus to be deallocated.
 */
__attribute__((visibility("default"))) static inline void deallocate_nimblenet_status(
    NimbleNetStatus* status) {
  if (status != NULL) {
    free(status->message);
    free(status);
  }
  return;
}

/**
 * @brief Frees the memory allocated for CUserEventsData.
 *
 * @param userEventsData Pointer to the CUserEventsData to be deallocated.
 */
__attribute__((visibility("default"))) static inline void deallocate_c_userevents_data(
    CUserEventsData* userEventsData) {
  free(userEventsData->eventType);
  free(userEventsData->eventJsonString);
}

#ifdef __cplusplus
}
#endif  // __cplusplus
