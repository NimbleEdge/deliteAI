/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**
 * @file data_variable_enums.hpp
 * @brief Enumeration definitions for data variable container types and member function types
 *
 * This file defines the core enumeration types used throughout the data variable system
 * for identifying container types and member function operations.
 */

// when adding a new containerType it should also be put in the get_containerType_string
/**
 * @brief Enumeration defining the different container types supported by the data variable system
 *
 * This enum represents the various data structures and container types that can be
 * handled by the DataVariable class hierarchy. Each type corresponds to a specific
 * data organization pattern.
 */
enum CONTAINERTYPE {
  SINGLE = 1,
  VECTOR = 2,
  TUPLE = 3,
  MAP = 4,
  SLICE = 5,
  RANGE = 6,
  LIST = 7,
  FUNCTIONDEF = 8,
  CLASS = 9,
};

// when adding a new member function it should be placed in _memberFuncMap,_inverseMemberFuncMap for
// appropriate calling
/**
 * @brief Enumeration defining all member function types supported by data variables
 *
 * This enum defines the complete set of operations that can be performed on data variables.
 * Each enum value corresponds to a specific member function that can be called on
 * DataVariable instances. The enum values are used for function dispatch and mapping
 * between string function names and their corresponding implementations.
 */
enum MemberFuncType {
  LOADMODEL,
  RUNMODEL,
  RESHAPE,
  CREATETENSOR,
  GETSHAPE,
  FEATURE_FILTER,
  FEATURE_FETCH,
  GETMODELSTATUS,
  CREATE_PROCESSOR_INIT,
  CREATE_ROLLINGWINDOW_PROCESSOR,
  CREATE_GROUPBY_COLUMNS_PROCESSOR,
  ADD_COMPUTATION_PROCESSOR,
  GET_PROCESSOR_OUTPUT,
  GET_PROCESSOR_OUTPUT_FOR_GROUP,
  CREATE_PROCESSOR,
  APPEND,
  TO_TENSOR,
  FEATURE_FILTER_ALL,
  GET_TIME,
  GET_CHRONO_TIME,
  FEATURE_FILTER_FUNCTION,
  NUM_KEYS,
  GET_CONFIG,
  GET_RAW_EVENTS_STORE,
  GET_DATAFRAME,
  EXP,
  POW,
  SORT,
  ARGSORT,
  TOPK,
  ARRANGE,
  ISINTEGER,
  ISFLOAT,
  ISSTRING,
  MIN,
  MAX,
  SUM,
  MEAN,
  PARSE_JSON,
  LOG,
  REGEX_MATCH,
  REGEX_SEARCH,
  REGEX_FULLMATCH,
  REGEX_SPLIT,
  REGEX_FINDALL,
  REGEX_FINDITER,
  REGEX_SUB,
  REGEX_SUBN,
  REGEX_MATCHOBJECT_GROUP,
  REGEX_MATCHOBJECT_GROUPS,
  REGEX_MATCHOBJECT_START,
  REGEX_MATCHOBJECT_END,
  REGEX_MATCHOBJECT_SPAN,
  STRING_UPPER,
  STRING_LOWER,
  STRING_STRIP,
  STRING_JOIN,
  CREATE_SIM_CHAR_STREAM,
  TO_JSON_STREAM,
  FINISHED,
  ITERATOR,
  NEXT,
  NEXT_AVAILABLE,
  GET_BLOCKING,
  NEXT_BLOCKING,
  WAIT_FOR_COMPLETION,
  GET_BLOCKING_STR,
  LLM,
  PROMPT,
  SKIP_TEXT_AND_GET_JSON_STREAM,
  RETRIEVER,
  POP,
  KEYS,
  JSON_DOCUMENT,
  MAX_INPUT_NUM_TOKENS,
  CONSTRUCTOR,
  UNICODE,
  SYNC,
  RUNPARALLEL,
  CREATE_CONCURRENT_EXECUTOR,
  SET_THREADS,
  CANCEL,
  CLEAR_CONTEXT,
  ADD_CONTEXT,
  LIST_COMPATIBLE_LLMS,
  LASTTYPE,  // should be last
};
