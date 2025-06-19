/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct ModelStatus {
  bool isModelReady;
  char* version;
} ModelStatus;

typedef struct CUserInput {
  void* data;
  int length;
  char* name;
  int dataType;
} CUserInput;

typedef struct InferenceRequest {
  int numInputs;
  CUserInput* inputs;
} InferenceRequest;

typedef struct InferenceReturn {
  void** outputs;
  int** outputShapes;
  int* outputLengths;
  int* outputShapeLengths;
  char** outputNames;
  int* outputTypes;
  int numOutputs;
} InferenceReturn;

typedef struct CTensor {
  char* name;
  void* data;
  int dataType;
  int64_t* shape;
  int shapeLength;
} CTensor;

typedef struct NimbleNetStatus {
  char* message;
  int code;
} NimbleNetStatus;

typedef struct CUserEventsData {
  char* eventType;
  char* eventJsonString;
} CUserEventsData;

typedef struct CTensors {
  CTensor* tensors;
  int numTensors;
  int outputIndex;
} CTensors;

typedef struct JsonOutput {
  int dataType;
  const char* key;  // If the datatype is array the key will be nullptr
  bool isEnd;       // Denotes that we have reached the end of json array/object iteration and
                    // there is no need to call get_next() again

  union {
    int64_t i;      // Will store the int data present at key in json
    double d;       // Will store the double data present at key in json
    bool b;         // Will store the boolean data present at key in json
    const char* s;  // Will store the string data present at key in json
    const void*
        obj;  // Will store JsonIterator* for the json array or object present at key in json
  } value;

} JsonOutput;

#ifdef __cplusplus
extern "C" {
#endif
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

__attribute__((visibility("default"))) static inline void deallocate_nimblenet_status(
    NimbleNetStatus* status) {
  if (status != NULL) {
    free(status->message);
    free(status);
  }
  return;
}

__attribute__((visibility("default"))) static inline void deallocate_c_userevents_data(
    CUserEventsData* userEventsData) {
  free(userEventsData->eventType);
  free(userEventsData->eventJsonString);
}

#ifdef __cplusplus
}
#endif

typedef NimbleNetStatus* (*FrontendFunctionPtr)(void* context, const CTensors input,
                                                CTensors* output);
