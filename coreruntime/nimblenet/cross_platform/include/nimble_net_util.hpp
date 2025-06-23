/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @brief Enum representing various status codes for API and internal operations.
 */
enum STATUS {
  // SUCCESS
  SUCCESS = 200, /**< Status code used to denote inference success. */
  REG_ACCEPT = 201,

  UNMODIFIED = 304, /**< Status code used check if the cloudConfig present in SaaS platform is
                       unmodified and the deployment present on disk is the latest. This is done by
                       sending the eTag in request. */

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
  TERMINAL_ERROR = 5002, /**< Status code returned when there is an error while running inference or
                            during nimblenet initialize. */
  // INTERNAL RETRYABLE ERRORS
  RETRYABLE_ERROR = -1,
  EMPTY_ERROR_CODE = 900,

  // JNI ERRORS
  DATA_TYPE_NOT_SUPPORTED_ERROR = 2001,
  JNI_ERROR_CODE = 8000
};

/**
 * @brief Enum representing custom data types used in delitepy.
 *
 * @note These map to ONNX-compatible data types where applicable.
 */
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

/**
 * @brief Enum representing the current status of a file download. Used by Job scheduler to know
 * whether the file has downloaded or not and start further processing.
 */
enum FileDownloadStatus {
  DOWNLOAD_RUNNING = 10001,
  DOWNLOAD_SUCCESS = 10002,
  DOWNLOAD_FAILURE = 10003,
  DOWNLOAD_PAUSED = 10004,
  DOWNLOAD_PENDING = 10005,
  DOWNLOAD_UNKNOWN = 10006,
};

/**
 * @name Metric types
 * Macros defining various metric types.
 */
///@{
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
///@}

/**
 * @brief Represents a network response object with status and content.
 */
struct CNetworkResponse {
  int statusCode; /**< HTTP-like status code of the response. */
  char* headers;  /**< Response headers as a string. */
  char* body;     /**< Response body content. */
  int bodyLength; /**< Length of the response body. */
};

/**
 * @brief Contains detailed information about a file download request.
 */
struct FileDownloadInfo {
  long requestId;                        /**< Unique identifier for the download request. */
  enum FileDownloadStatus prevStatus;    /**< Previous status of the download. */
  enum FileDownloadStatus currentStatus; /**< Current status of the download. */
  long timeElapsedInMicro;               /**< Time elapsed since the start, in microseconds. */
  int currentStatusReasonCode;           /**< Reason code explaining the current status. */
};

typedef struct CNetworkResponse CNetworkResponse;

/**
 * @brief Enum defining types of complex iOS-compatible objects.
 *
 * @note These are over and above the DATAYPE enum defined earlier.
 */
typedef enum {
  IOS_PROTO_OBJECT, /**< Protobuf-style object. */
  IOS_MAP,          /**< Key-value map. */
  IOS_ARRAY,        /**< Ordered array. */
  IOS_ANY_OBJECT    /**< Any object type (generic). */
} IosObjectType;

/**
 * @brief Represents a generic iOS object and its type.
 */
typedef struct {
  const void* obj;    /**< Pointer to the actual object. */
  IosObjectType type; /**< Type of the object. */
} IosObject;
