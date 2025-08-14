/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "FunctionPointersImpl.h"

#import <Foundation/Foundation.h>
#import <zlib.h>

#import <DeliteAI/DeliteAI-Swift.h>
#import "DeliteAI/NimbleNetController.h"
#import "client.h"
#import "frontend_layer.h"
#import "DeliteAI/InputConverter.h"
#import "DeliteAI/OutputConverter.h"

BOOL fileExistsAtPath(const char *filePath) {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *path = [NSString stringWithUTF8String:filePath];
    return [fileManager fileExistsAtPath:path];
}

void renameFile(const char *currentFilePath, const char *newFilePath) {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;

    NSString *currentPath = [NSString stringWithUTF8String:currentFilePath];
    NSString *newPath = [NSString stringWithUTF8String:newFilePath];

    // Check if the file exists at the current path
    if ([fileManager fileExistsAtPath:currentPath]) {
        // Attempt to rename the file
        BOOL success = [fileManager moveItemAtPath:currentPath toPath:newPath error:&error];
        if (success) {
            NSLog(@"File renamed successfully.");
        } else {
            NSLog(@"Error renaming file: %@", error.localizedDescription);
        }
    } else {
        NSLog(@"File does not exist at path: %s", currentFilePath);
    }
}

void decompressGzippedFile(const char* gzippedFilePath) {
    // Open the gzipped file for reading in binary mode
    gzFile gzFile = gzopen(gzippedFilePath, "rb");
    if (!gzFile) {
        NSLog(@"Failed to open gzipped file for reading: %s", gzippedFilePath);
        return;
    }

    // Create a buffer for decompressed data
    const int bufferSize = 1024;
    char buffer[bufferSize];

    // Append ".out" to the gzipped file name for the decompressed file
    char decompressedFilePath[strlen(gzippedFilePath) + 5]; // +5 for ".out" and null terminator
    strcpy(decompressedFilePath, gzippedFilePath);
    strcat(decompressedFilePath, ".out");

    // Open the decompressed file for writing in binary mode
    FILE *decompressedFile = fopen(decompressedFilePath, "wb");
    if (!decompressedFile) {
        NSLog(@"Failed to create decompressed file for writing: %s", decompressedFilePath);
        gzclose(gzFile);
        return;
    }

    // Decompress and write the data to the decompressed file
    int bytesRead;
    while ((bytesRead = gzread(gzFile, buffer, bufferSize)) > 0) {
        fwrite(buffer, 1, bytesRead, decompressedFile);
    }

    // Close the files and cleanup
    gzclose(gzFile);
    fclose(decompressedFile);
    NSLog(@"File decompressed and saved to %s", decompressedFilePath);

    remove(gzippedFilePath);

    rename(decompressedFilePath, gzippedFilePath);
}

void initClientFunctionPointers(void){
    send_request_global = send_request_interop;
    get_hardware_info_global = get_hardware_info_interop;
    log_verbose_global = log_debug_interop;
    log_debug_global = log_debug_interop;
    log_info_global = log_debug_interop;
    log_warn_global = log_debug_interop;
    log_error_global = log_debug_interop;
    log_fatal_global = log_debug_interop;
    download_model_global = download_model_interop;
    set_thread_priority_max_global = set_thread_priority_max_interop;
    set_thread_priority_min_global = set_thread_priority_min_interop;

    get_phonemes_global = get_phonemes_interop;

    get_ios_object_string_subscript_global = get_ios_object_string_subscript;
    get_ios_object_int_subscript_global = get_ios_object_int_subscript;
    deallocate_ios_nimblenet_status_global = deallocate_ios_nimblenet_status;
    deallocate_frontend_ctensor_global = deallocate_frontend_ctensor;
    get_ios_object_size_global = get_ios_object_size;
    set_ios_object_string_subscript_global = set_ios_object_string_subscript;
    set_ios_object_int_subscript_global = set_ios_object_int_subscript;
    ios_object_to_string_global =  ios_object_to_string;
    ios_object_arrange_global = ios_object_arrange;
    in_ios_object_global = in_ios_object;
    release_ios_object_global = release_ios_object;
    get_keys_ios_object_global = get_keys_ios_object;
}

CNetworkResponse send_request_interop(const char *body, const char *headers, const char *url,
                                      const char *method, int length){
    return [[ConnectionLayer shared] sendRequestWithUrl:[NSString stringWithUTF8String:url] reqBody:[NSString stringWithUTF8String:body] reqHeaders:[NSString stringWithUTF8String:headers] method:[NSString stringWithUTF8String:method] length:length];
}

char *get_hardware_info_interop(void){
    HardwareInfo *hardwareInfo = [[HardwareInfo alloc] init];
    NSString *metricJson = [hardwareInfo getMetricsJSON];
    return strdup([metricJson UTF8String]);
}

void log_debug_interop(const char *message) {
    Logger *logger = [Logger shared];
    [logger debugWithMessage:[NSString stringWithUTF8String:message]];
}

void log_info_interop(const char *message) {
    Logger *logger = [Logger shared];
    [logger infoWithMessage:[NSString stringWithUTF8String:message]];
}

void log_warn_interop(const char *message) {
    Logger *logger = [Logger shared];
    [logger warnWithMessage:[NSString stringWithUTF8String:message]];
}

void log_error_interop(const char *message) {
    Logger *logger = [Logger shared];
    [logger errorWithMessage:[NSString stringWithUTF8String:message]];
}

void log_fatal_interop(const char *message) {
    Logger *logger = [Logger shared];
    [logger fatalWithMessage:[NSString stringWithUTF8String:message]];
}

bool set_thread_priority_min_interop() {
   return [NSThread setThreadPriority:0.0];
}

bool set_thread_priority_max_interop() {
   return [NSThread setThreadPriority:1.0];
}

struct FileDownloadInfo download_model_interop(const char *url, const char *headers, const char *fileName, const char *tagDir){
    NSString *urlString = [NSString stringWithUTF8String:url];
    NSString *fileNameString = [NSString stringWithUTF8String:fileName];
    NSString *tagDirString = [NSString stringWithUTF8String:tagDir];
    NSString *filePathStr = [tagDirString stringByAppendingPathComponent:fileNameString];
    NSString *headersString = [NSString stringWithUTF8String:headers];

    struct FileDownloadInfo res = [[ConnectionLayer shared]downloadFileFrom:urlString
                                                        to:filePathStr
                                                    method:@"GET"
                                                   headers:headersString
                                                fileName:fileNameString];
    return res;
}

char *get_phonemes_interop(const char *text) {
    if (EspeakNGCallbacks.textToPhonemes == nil) {
        return NULL;
    }

    NSString *textString = [NSString stringWithUTF8String:text];
    NSString *phonemeString = EspeakNGCallbacks.textToPhonemes(textString);
    return strdup([phonemeString UTF8String]);
}

// functions for proto

NimbleNetStatus* get_ios_object_string_subscript(IosObject proto, const char* key, CTensor* child) {
    id value = NULL;
    NimbleNetStatus* status = NULL;
    NSString *keyString = [NSString stringWithUTF8String:key];
    switch (proto.type) {
        case IOS_PROTO_OBJECT: {
            ProtoObjectWrapper* obj = (__bridge ProtoObjectWrapper*) proto.obj;
            status = [obj get_valueWithKey:keyString value:&value];
            break;
        }
        case IOS_ANY_OBJECT: {
            ProtoAnyWrapper* obj = (__bridge ProtoAnyWrapper*) proto.obj;
            status = [obj get_valueWithKey:keyString value:&value];
            break;
        }
        case IOS_MAP: {
            ProtoMapWrapper* obj = (__bridge ProtoMapWrapper*) proto.obj;
            status = [obj get_valueWithKey:keyString value:&value];
            break;
        }
        case IOS_ARRAY:
            return createNimbleNetStatus(@"string_subscript not supported for array object");
    }

    if (status) return status;

    status = convertSingularInputToCTensor(value,child);
    /*
     * We need to release this value due to some issue (or feature) in ObjectiveC-Swift interop
     * An id* we send from here is taken by a Swift function as UnsafeMutablePointer<AnyObject>
     * When the Swift code sets the pointee of the pointer, the retain count is incremented there
     * The retain done in Swift code is never released as it seems like the UnsafeMutablePointer<AnyObject>
     * is never released. So we balance that retain here.
     */
    CFRelease((__bridge CFTypeRef)(value));
    return status;
}

NimbleNetStatus* get_ios_object_int_subscript(IosObject proto, int index, CTensor* child) {
    id value = NULL;
    NimbleNetStatus* status = NULL;
    switch (proto.type) {
        case IOS_PROTO_OBJECT:
        case IOS_ANY_OBJECT:
        case IOS_MAP:
            return createNimbleNetStatus(@"get_int_subscript not supported");
        case IOS_ARRAY: {
            ProtoListWrapper* listWrapper = (__bridge ProtoListWrapper*) proto.obj;
            status = [listWrapper get_valueWithIndex:index value: &value];
        }
    }
    if (status) return status;

    status = convertSingularInputToCTensor(value, child);
    CFRelease((__bridge CFTypeRef)(value));
    return status;
}

void deallocate_ios_nimblenet_status(NimbleNetStatus* status){
    if (status) {
        free(status->message);
        free(status);
    }
    return;
}

void deallocate_frontend_ctensor(CTensor* ctensor){
    if (ctensor == NULL) return;
    freeCTensor(ctensor);
}

NimbleNetStatus* get_ios_object_size(IosObject proto, int* val)  {
    switch (proto.type) {
        case IOS_PROTO_OBJECT:
        case IOS_ANY_OBJECT:
            return createNimbleNetStatus(@"get_ios_object_size not supported");
        case IOS_MAP:{
            ProtoMapWrapper* mapWrapper = (__bridge ProtoMapWrapper*) proto.obj;
            *val = [mapWrapper count];
            return NULL;
        }
        case IOS_ARRAY: {
            ProtoListWrapper* listWrapper = (__bridge ProtoListWrapper*) proto.obj;
            *val = [listWrapper count];
            return NULL;
        }
    }
}

NimbleNetStatus* createNimbleNetStatus(NSString *message) {
    NimbleNetStatus *status = (NimbleNetStatus *)malloc(sizeof(NimbleNetStatus));
    if (!status) return NULL;
    status->message = strdup([message UTF8String]);
    ProtoError errorCode = ProtoErrorFailure;
    status->code = (int)errorCode;

    return status;
}

NimbleNetStatus* set_ios_object_string_subscript(IosObject proto, const char* key, CTensor* value) {
    NimbleNetStatus* status = NULL;

    switch (proto.type) {
        case IOS_PROTO_OBJECT: {
            ProtoObjectWrapper* obj = (__bridge ProtoObjectWrapper*) proto.obj;
            NSString *keyString = [NSString stringWithUTF8String:key];
            id data = castDataFromCTensor(value);
            if (data == NULL) {
                return createNimbleNetStatus(@"Unable to cast value to Objective-C type");
            }
            status = [obj set_valueWithKey:keyString value:data];
            break;
        }
        case IOS_MAP: {
            ProtoMapWrapper* obj = (__bridge ProtoMapWrapper*) proto.obj;
            id data = castDataFromCTensor(value);
            NSString *keyString = [NSString stringWithUTF8String:key];
            status = [obj set_valueWithKey:keyString value:data];
            break;
        }
        case IOS_ARRAY:
            return createNimbleNetStatus(@"string_subscript not supported for array object");
        case IOS_ANY_OBJECT:{
            ProtoAnyWrapper* obj = (__bridge ProtoAnyWrapper*) proto.obj;
            NSString *keyString = [NSString stringWithUTF8String:key];
            id data = castDataFromCTensor(value);
            status = [obj set_valueWithKey:keyString value:data];
            break;
        }
    }

    return status;
}

NimbleNetStatus* set_ios_object_int_subscript(IosObject proto, int key, CTensor* value) {
    switch (proto.type) {
        case IOS_PROTO_OBJECT:
        case IOS_ANY_OBJECT:
        case IOS_MAP:
            return createNimbleNetStatus(@"set_ios_object_int_subscript_type not supported");
        case IOS_ARRAY:{
            ProtoListWrapper* obj = (__bridge ProtoListWrapper*) proto.obj;
            id data = castDataFromCTensor(value);
            return [obj set_valueWithIndex:key value:data];
        }
    }
}

NimbleNetStatus* ios_object_to_string(IosObject obj, char** str){
    switch (obj.type) {
        case IOS_PROTO_OBJECT:{
            ProtoObjectWrapper* objectWrapper = (__bridge ProtoObjectWrapper*) obj.obj;
            return [objectWrapper ios_object_to_stringWithStr:str];
        }
        case IOS_ANY_OBJECT:{
            ProtoAnyWrapper* anyWrapper = (__bridge ProtoAnyWrapper*) obj.obj;
            return [anyWrapper ios_object_to_stringWithStr:str];
        }
        case IOS_MAP:{
            ProtoMapWrapper* mapWrapper = (__bridge ProtoMapWrapper*) obj.obj;
            return [mapWrapper ios_object_to_stringWithStr:str];
        }
        case IOS_ARRAY:{
            ProtoListWrapper* listWrapper = (__bridge ProtoListWrapper*) obj.obj;
            return [listWrapper ios_object_to_stringWithStr:str];
        }
    }

    return NULL;
}

NimbleNetStatus* ios_object_arrange(IosObject obj, const int* indices,int numIndices, IosObject* newObj){
    switch (obj.type) {
        case IOS_PROTO_OBJECT:
        case IOS_ANY_OBJECT:
        case IOS_MAP:
            return createNimbleNetStatus(@"ios_object_arrange not supported");
        case IOS_ARRAY:{
            ProtoListWrapper* list = (__bridge ProtoListWrapper*) obj.obj;
            ProtoListWrapper* newList = [list rearrangeWithIndices:indices numIndices:numIndices];

            newObj->obj = CFBridgingRetain(newList);
            newObj->type = IOS_ARRAY;
            return NULL;
        }
    }
    return NULL;
}

NimbleNetStatus* in_ios_object(IosObject obj, const char* key, bool* result){
    NSString *keyString = [NSString stringWithUTF8String:key];

    switch (obj.type) {
        case IOS_PROTO_OBJECT: {
            ProtoObjectWrapper* wrapper = (__bridge ProtoObjectWrapper*) obj.obj;
            return [wrapper in_ios_objectWithKey:keyString result:result];
        }
        case IOS_ANY_OBJECT: {
            ProtoAnyWrapper* wrapper = (__bridge ProtoAnyWrapper*) obj.obj;
            return [wrapper in_ios_objectWithKey:keyString result:result];
        }
        case IOS_MAP: {
            ProtoMapWrapper* wrapper = (__bridge ProtoMapWrapper*) obj.obj;
            return [wrapper in_ios_objectWithKey:keyString result:result];
        }
        case IOS_ARRAY:
            return createNimbleNetStatus(@"string_subscript not supported for array object");
    }

    return NULL;
}

NimbleNetStatus* get_keys_ios_object(IosObject obj, CTensor* result){
    id value = NULL;
    NSInteger arrayLength;
    NimbleNetStatus* status = NULL;

    switch (obj.type) {
        case IOS_ARRAY:
            return createNimbleNetStatus(@"get_keys not supported");
        case IOS_ANY_OBJECT:{
            ProtoAnyWrapper* wrapper = (__bridge ProtoAnyWrapper*) obj.obj;
            status = [wrapper get_keysWithValue:&value];
            arrayLength = [wrapper count];
        }
        case IOS_PROTO_OBJECT:{
            ProtoObjectWrapper* wrapper = (__bridge ProtoObjectWrapper*) obj.obj;
            status = [wrapper get_keysWithValue:&value];
            arrayLength = [wrapper count];
        }
        case IOS_MAP: {
            ProtoMapWrapper* wrapper = (__bridge ProtoMapWrapper*) obj.obj;
            status = [wrapper get_keysWithValue:&value];
            arrayLength = [wrapper count];
        }
    }
    if(status) return status;
    enum DATATYPE datatype;
    datatype = STRING;

    // array will always be 1D
    result->shapeLength = 1;

    int64_t* int64ShapeArray = (int64_t *)malloc(sizeof(int64_t) * 1);
    int64ShapeArray[0] = arrayLength;

    result->data = convertArraytoVoidPointer(value, arrayLength, datatype);
    CFRelease((__bridge CFTypeRef)(value));
    result->dataType = datatype;
    result->shape = int64ShapeArray;
    result->name = NULL;

    return NULL;
}

NimbleNetStatus* release_ios_object(IosObject obj){
    CFBridgingRelease(obj.obj); // ignoring return
    return NULL;
}
