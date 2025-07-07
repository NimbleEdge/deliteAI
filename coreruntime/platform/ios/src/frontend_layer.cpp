/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "frontend_layer.h"

get_ios_object_string_subscript_type get_ios_object_string_subscript_global = nullptr;
get_ios_object_int_subscript_type get_ios_object_int_subscript_global = nullptr;
deallocate_ios_nimblenet_status_type deallocate_ios_nimblenet_status_global = nullptr;
deallocate_frontend_ctensor_type deallocate_frontend_ctensor_global = nullptr;
get_ios_object_size_type get_ios_object_size_global = nullptr;
set_ios_object_string_subscript_type set_ios_object_string_subscript_global = nullptr;
set_ios_object_int_subscript_type set_ios_object_int_subscript_global = nullptr;
ios_object_to_string_type ios_object_to_string_global = nullptr;
ios_object_arrange_type ios_object_arrange_global = nullptr;
in_ios_object_type in_ios_object_global = nullptr;
release_ios_object_type release_ios_object_global = nullptr;
get_keys_ios_object_type get_keys_ios_object_global = nullptr;

bool deallocate_frontend_tensors(CTensors cTensors) { return true; }

bool free_frontend_function_context(void *context) { return true; }
