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
/**
 * @brief Namespace used by Ios SDK to create json input/output.
 * @note In Android this is not required as it directly uses nlohmann::json.
 */
namespace nimblejson {
#endif

// =============================
// JSON Memory Management
// =============================

/**
 * @brief Allocates a context-specific JSON memory allocator.
 *
 * @return Pointer to the created JSON allocator.
 */
void* create_json_allocator();

/**
 * @brief Deallocates and cleans up the given JSON allocator.
 *
 * @param json_allocator Pointer to the allocator to be deallocated.
 */
void deallocate_json_allocator(void* json_allocator);

// =============================
// JSON Object/Array Creation
// =============================

/**
 * @brief Creates a new JSON object using the provided allocator.
 *
 * @param json_allocator The allocator to use.
 * @return Pointer to the newly created JSON object.
 */
void* create_json_object(void* json_allocator);

/**
 * @brief Creates a new JSON array using the provided allocator.
 *
 * @param json_allocator The allocator to use.
 * @return Pointer to the newly created JSON array.
 */
void* create_json_array(void* json_allocator);

// =============================
// JSON Array Insertion (Move semantics)
// =============================

/**
 * @brief Moves a JSON object or array into an existing array.
 */
bool move_json_object_or_array_to_array(void* jsonArray, void* json_object);

/**
 * @brief Moves a 64-bit integer value into a JSON array.
 */
bool move_int64_value_to_array(void* jsonArray, const int64_t value);

/**
 * @brief Moves a double value into a JSON array.
 */
bool move_double_value_to_array(void* jsonArray, const double value);

/**
 * @brief Moves a C-style string into a JSON array.
 */
bool move_string_value_to_array(void* jsonArray, const char* value);

/**
 * @brief Moves a boolean value into a JSON array.
 */
bool move_bool_value_to_array(void* jsonArray, const bool value);

/**
 * @brief Moves a null value into a JSON array.
 */
bool move_null_value_to_array(void* jsonArray);

// =============================
// JSON Object Property Insertion
// =============================

/**
 * @brief Adds a double value to a JSON object under the specified key.
 */
bool add_double_value(const char* key, const double value, void* json);

/**
 * @brief Adds a 64-bit integer value to a JSON object under the specified key.
 */
bool add_int64_value(const char* key, const int64_t value, void* json);

/**
 * @brief Adds a boolean value to a JSON object under the specified key.
 */
bool add_bool_value(const char* key, const bool value, void* json);

/**
 * @brief Adds a string value to a JSON object under the specified key.
 */
bool add_string_value(const char* key, const char* value, void* json);

/**
 * @brief Adds a null value to a JSON object under the specified key.
 */
bool add_null_value(const char* key, void* json);

/**
 * @brief Adds another JSON object or array to the JSON object under the given key.
 *
 * @param key The property name to use in the object.
 * @param value The JSON object or array to insert.
 * @param json The target JSON object.
 */
bool add_json_object_to_json(const char* key, void* value, void* json);

// =============================
// JSON Iteration
// =============================

/**
 * @brief Creates an iterator for walking through a JSON array or object.
 *
 * @param json The JSON container to iterate.
 * @param json_allocator Allocator to use during iteration.
 * @return Pointer to the iterator.
 */
void* create_json_iterator(void* json, void* json_allocator);

/**
 * @brief Returns the next element from the iterator.
 *
 * @param jsonIterator The iterator returned by create_json_iterator.
 * @param json_allocator Allocator used for output element memory.
 * @return Pointer to the next JsonOutput object with the appropriate data and a flag isEnd set to
 * true if this is the last entry in json.
 */
void* get_next_json_element(void* jsonIterator, void* json_allocator);
#ifdef __cplusplus
}  // namespace nimblejson
#endif

#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop