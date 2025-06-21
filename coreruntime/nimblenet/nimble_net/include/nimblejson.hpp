/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "executor_structs.h"
#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
namespace nimblejson {
#endif

// Json Allocator for storing context
void* create_json_allocator();
void deallocate_json_allocator(void* json_allocator);

// Create json object and array
void* create_json_object(void* json_allocator);
void* create_json_array(void* json_allocator);

// Move elements to array
bool move_json_object_or_array_to_array(void* jsonArray, void* json_object);
bool move_int64_value_to_array(void* jsonArray, const int64_t value);
bool move_double_value_to_array(void* jsonArray, const double value);
bool move_string_value_to_array(void* jsonArray, const char* value);
bool move_bool_value_to_array(void* jsonArray, const bool value);
bool move_null_value_to_array(void* jsonArray);

// Add values(single/arrays) to json object
bool add_double_value(const char* key, const double value, void* json);
bool add_int64_value(const char* key, const int64_t value, void* json);
bool add_bool_value(const char* key, const bool value, void* json);
bool add_string_value(const char* key, const char* value, void* json);
bool add_null_value(const char* key, void* json);

// Add json object/array to json object
bool add_json_object_to_json(const char* key, void* value, void* json);

// Functions for getting json output
void* create_json_iterator(void* json, void* json_allocator);
void* get_next_json_element(void* jsonIterator, void* json_allocator);
#ifdef __cplusplus
}  // namespace nimblejson
#endif

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop