/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FunctionPointersImpl_h
#define FunctionPointersImpl_h

#import "nimble_net_util.hpp"
#import "executor_structs.h"

void initClientFunctionPointers(void);
CNetworkResponse send_request_interop(const char *body, const char *headers, const char *url,
                                      const char *method, int length);
char *get_hardware_info_interop(void);
void log_debug_interop(const char *message);
void log_info_interop(const char *message);
void log_warn_interop(const char *message);
void log_error_interop(const char *message);
void log_fatal_interop(const char *message);
struct FileDownloadInfo download_model_interop(const char *url, const char *headers, const char *fileName, const char *tagDir);
bool set_thread_priority_min_interop();
bool set_thread_priority_max_interop();

char *get_phonemes_interop(const char *text);

NimbleNetStatus* get_ios_object_string_subscript(IosObject proto, const char* key, CTensor* child);
NimbleNetStatus* get_ios_object_int_subscript(IosObject proto, int key, CTensor* child);
void deallocate_ios_nimblenet_status(NimbleNetStatus* status);
void deallocate_frontend_ctensor(CTensor* ctensor);
NimbleNetStatus* get_ios_object_size(IosObject proto, int* val);
NimbleNetStatus* createNimbleNetStatus(NSString *message);
NimbleNetStatus* set_ios_object_string_subscript(IosObject proto, const char* key, CTensor* value);
NimbleNetStatus* set_ios_object_int_subscript(IosObject proto, int key, CTensor* value);
NimbleNetStatus* ios_object_to_string(IosObject obj, char** str);
NimbleNetStatus* ios_object_arrange(IosObject obj, const int* indices,int numIndices, IosObject* newObj);
NimbleNetStatus* in_ios_object(IosObject obj, const char* key, bool* result);
NimbleNetStatus* release_ios_object(IosObject obj);
NimbleNetStatus* get_keys_ios_object(IosObject obj, CTensor* result);

#endif /* FunctionPointersImpl_h */
