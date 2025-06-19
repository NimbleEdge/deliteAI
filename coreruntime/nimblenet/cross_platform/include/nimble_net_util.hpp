/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

enum STATUS {
  // SUCCESS
  SUCCESS = 200,
  REG_ACCEPT = 201,

  UNMODIFIED = 304,

  // ERRORS
  BAD_REQUEST_ERR = 400,
  AUTH_ERR = 401,
  RESOURCE_NOT_FOUND_ERR = 404,

  SERVER_ERR = 500,

  RESOURCE_MISMATCH_ERR = 1304,

  INVALID_ENCODING_ERR = 1400,
  CYCLE_REJECTED_ERR = 1403,
  CYCLE_NOT_FOUND_ERR = 1404,

  // INTERNAL NON-RETRYABLE ERRORS
  JSON_PARSE_ERR = 5000,
  EXECUTOR_LOAD_MODEL_ERR = 5001,
  TERMINAL_ERROR = 5002,
  // INTERNAL RETRYABLE ERRORS
  RETRYABLE_ERROR = -1,
  EMPTY_ERROR_CODE = 900,

  // JNI ERRORS
  DATA_TYPE_NOT_SUPPORTED_ERROR = 2001,
  JNI_ERROR_CODE = 8000
};

// Custom data type enum for nimbleSDK since that is compatible with ONNX enums for data type. For
// all other executors they will need to maintain a map from this to their own data type
enum DATATYPE {
  NONE = 667,
  EMPTY = 668,  // added only for printing
  NIMBLENET = 669,
  JSON = 670,
  NIMBLENET_INTERNAL = 671,
  RAW_EVENTS_STORE = 673,
  TABLE_EVENT = 674,
  FILTERED_DATAFRAME = 675,
  DATAFRAME = 676,
  NIMBLENET_REGEX = 677,
  NIMBLENET_REGEX_MATCHOBJECT = 678,
  CHAR_STREAM = 679,
  JSON_STREAM = 680,
  JSON_ARRAY = 681,
  FUNCTION = 682,
  CONCURRENT_EXECUTOR = 683,
  EXCEPTION = 684,
  UNKNOWN = 0,
  FLOAT = 1,
  BOOLEAN = 9,
  INT32 = 6,
  INT64 = 7,
  DOUBLE = 11,
  STRING = 8,
  UNICODE_STRING = 112,
  INT32_ARRAY = 106,
  INT64_ARRAY = 107,
  FLOAT_ARRAY = 101,
  DOUBLE_ARRAY = 111,
  STRING_ARRAY = 108,
  FE_OBJ = 700,
};

enum FileDownloadStatus {
  DOWNLOAD_RUNNING = 10001,
  DOWNLOAD_SUCCESS = 10002,
  DOWNLOAD_FAILURE = 10003,
  DOWNLOAD_PAUSED = 10004,
  DOWNLOAD_PENDING = 10005,
  DOWNLOAD_UNKNOWN = 10006,
};

#define MODELDOWNLOADMETRIC "modelDownload"
#define PINGMETRIC "pingTime"
#define INFERENCEMETRIC "inference"
#define STATICDEVICEMETRICS "staticDevice"
#define DYNAMICDEVICEMETRICS "dynamicDevice"
#define INFERENCEV4 "inferencev4"
#define SESSIONMETRICS "sessionMetrics"
#define ACUMETRIC "acumetric"
#define MODELTYPE "model"
#define SCRIPTTYPE "script"
#define INTERNALSTORAGEMETRICS "internalStorage"

struct CNetworkResponse {
  int statusCode;
  char* headers;
  char* body;
  int bodyLength;
};

struct FileDownloadInfo {
  long requestId;
  enum FileDownloadStatus prevStatus;
  enum FileDownloadStatus currentStatus;
  long timeElapsedInMicro;
  int currentStatusReasonCode;
};

typedef struct CNetworkResponse CNetworkResponse;

typedef enum { IOS_PROTO_OBJECT, IOS_MAP, IOS_ARRAY, IOS_ANY_OBJECT } IosObjectType;

typedef struct {
  const void* obj;
  IosObjectType type;
} IosObject;
