/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <DeliteAI/DeliteAI-Swift.h>
#import "nimblejson.hpp"
#import "executor_structs.h"
#import "InputConverter.h"


@implementation InputConverter : NSObject 

void* convertArraytoVoidPointerWithJsonAlloc(NSArray* arrayData, int arrayLength, int dataType,void* json_alloc){
    switch (dataType) {
        case JSON_ARRAY: {
            return convertJsonArrayToVoidPointer(arrayData,json_alloc);
        }
        default:
            return convertArraytoVoidPointer(arrayData,arrayLength, dataType);
    }
}

void* convertArraytoVoidPointer(NSArray* arrayData, int arrayLength, int dataType){
    switch(dataType){
        case STRING: {
            char **cArray = (char **)malloc(arrayLength * sizeof(char *));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSString *string = [arrayData objectAtIndex:i];
                if ([string isKindOfClass:[NSString class]]) {
                    const char *cString = [string UTF8String];
                    if (cString) {
                        cArray[i] = strdup(cString);
                    } else {
                        cArray[i] = NULL;
                    }
                } else {
                    cArray[i] = NULL;
                }
            }
            return (void *)cArray;
        }
            
        case INT32:{
            int *cArray = (int *)malloc(arrayLength * sizeof(int));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSNumber *number = [arrayData objectAtIndex:i];
                cArray[i] = [number intValue];
            }
            return (void*) cArray;
        }
        case BOOLEAN: {
            bool *cArray = (bool *)malloc(arrayLength * sizeof(bool));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSNumber *number = [arrayData objectAtIndex:i];
                cArray[i] = [number boolValue];
            }
            return (void *)cArray;
        }
        case FLOAT:{
            float *cArray = (float *)malloc(arrayLength * sizeof(float));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSNumber *number = [arrayData objectAtIndex:i];
                cArray[i] = [number floatValue];
            }
            return (void*) cArray;
        }
        case DOUBLE:{
            double *cArray = (double *)malloc(arrayLength * sizeof(double));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSNumber *number = [arrayData objectAtIndex:i];
                cArray[i] = [number doubleValue];
            }
            return (void*) cArray;
        }
        case INT64:{
            int64_t *cArray = (long long *)malloc(arrayLength * sizeof(long long));
            for (NSUInteger i = 0; i < arrayLength; i++) {
                NSNumber *number = [arrayData objectAtIndex:i];
                cArray[i] = [number longLongValue];
            }
            return (void*) cArray;
        }
        default:
            return nil;
            
    }
}

void* convertSingularInputtoVoidPointer(id data, int dataType, void* json_alloc) {
    switch (dataType) {
        case JSON:
            return convertJsonDictToVoidPointer((NSDictionary*)data, json_alloc);
        case STRING:
            return convertStringToVoidPointer((NSString*)data);
        case BOOLEAN:
            return convertBoolToVoidPointer((NSNumber*)data);
        case INT32:
            return convertInt32ToVoidPointer((NSNumber*)data);
        case FLOAT:
            return convertFloatToVoidPointer((NSNumber*)data);
        case DOUBLE:
            return convertDoubleToVoidPointer((NSNumber*)data);
        case INT64:
            return convertInt64ToVoidPointer((NSNumber*)data);
        case FE_OBJ:
            // Assuming that frontend will always send a proto object
            // and not a Any Object
            return convertProtoObjectToVoidPointer((ProtoObjectWrapper*)data);
        default:
            NSLog(@"Invalid singular input");
            return nil;
    }
}

NimbleNetStatus* convertSingularInputToCTensor(id data,CTensor* child) {
    initialiseCTensor(child);
    if ([data isKindOfClass:[NSString class]]) {
        convertStringToCTensor((NSString*)data, child);
    } else if ([data isKindOfClass:[NSNumber class]]) {
        NSNumber *number = (NSNumber *)data;
        const char *type = [number objCType];
        
        if (strcmp(type, @encode(char)) == 0 || strcmp(type, @encode(BOOL)) == 0) {
            convertBoolToCTensor(number,child);
        }
        else if (strcmp(type, @encode(int)) == 0) {
            convertInt32ToCTensor(number,child);
        }
        else if (strcmp(type, @encode(long)) == 0 || strcmp(type, @encode(long long)) == 0) {
            convertInt64ToCTensor(number,child);
        }
        else if (strcmp(type, @encode(float)) == 0) {
            convertFloatToCTensor(number,child);
        }
        else if (strcmp(type, @encode(double)) == 0) {
            convertDoubleToCTensor(number,child);
        }
        else{
            return createNimbleNetStatus(@"unknown type of member NSNumber class");
        }
    } else if ([data isKindOfClass:[NSDictionary class]]) {
        IosObject* iosObj = malloc(sizeof(IosObject));
        iosObj->obj = CFBridgingRetain(data);
        iosObj->type = IOS_MAP;
        child->data = iosObj;
        child->dataType = FE_OBJ;
    } else if ([data isKindOfClass:[NSArray class]]) {
        IosObject* iosObj = malloc(sizeof(IosObject));
        iosObj->obj = CFBridgingRetain(data);
        iosObj->type = IOS_ARRAY;
        child->data = iosObj;
        child->dataType = FE_OBJ;
    } else if ([data isKindOfClass:[NSNull class]]) {
        child->data = NULL;
        child->dataType = NONE;
    } else if ([data isKindOfClass:[ProtoObjectWrapper class]]) {
        child->data = convertProtoObjectToVoidPointer((ProtoObjectWrapper*) data);
        child->dataType = FE_OBJ;
    } else if ([data isKindOfClass:[ProtoAnyWrapper class]]) {
        child->data = convertProtoAnyToVoidPointer((ProtoAnyWrapper*) data);
        child->dataType = FE_OBJ;
    } else if ([data isKindOfClass:[ProtoListWrapper class]]){
        child->data = convertProtoListToVoidPointer((ProtoListWrapper*) data);
        child->dataType = FE_OBJ;
    } else if ([data isKindOfClass:[ProtoMapWrapper class]]){
        child->data = convertProtoMapToVoidPointer((ProtoMapWrapper*) data);
        child->dataType = FE_OBJ;
    } else {
        return createNimbleNetStatus(@"Could not parse data to a known datatype");
    }
    return NULL;
}

void* convertJsonArrayToVoidPointer(NSArray* jsonArray, void* json_alloc){
    void* array = create_json_array(json_alloc);
    
    
    NSInteger arrayLength = [(NSArray *)jsonArray count];
    for(int idx = 0; idx<arrayLength; idx++ ){
        id element = [(NSArray *)jsonArray objectAtIndex:idx];
        
        if ([element isKindOfClass:[NSString class]]) {
            // element wont be null
            const char *elementString = [element UTF8String];
            move_string_value_to_array(array, elementString);
            
        } else if ([element isKindOfClass:[NSNumber class]]) {
            NSNumber *number = (NSNumber *)element;
            const char *type = [number objCType];
            if (strcmp(type, @encode(char)) == 0 || strcmp(type, @encode(BOOL)) == 0) {
                bool boolValue = [number boolValue];
                move_bool_value_to_array(array, boolValue);
            }
            else if (strcmp(type, @encode(int)) == 0 || strcmp(type, @encode(long)) == 0 || strcmp(type, @encode(long long)) == 0) {
                int64_t longValue = [number longLongValue];
                move_int64_value_to_array(array, longValue);
            }
            else if (strcmp(type, @encode(float)) == 0 || strcmp(type, @encode(double)) == 0) {
                double doubleValue = [number doubleValue];
                move_double_value_to_array(array, doubleValue);
            }
            else{
                NSLog(@"unknown type of member NSNumber class: %@", number);
            }
            
            
        } else if ([element isKindOfClass:[NSDictionary class]]) {
            void* newdict =  convertJsonDictToVoidPointer(element,json_alloc);
            move_json_object_or_array_to_array(array, newdict);
            
        } else if ([element isKindOfClass:[NSArray class]]) {
            void* JsonArray = convertJsonArrayToVoidPointer(element,json_alloc);
            move_json_object_or_array_to_array(array, JsonArray);
            
        } else if ([element isKindOfClass:[NSNull class]]) {
            move_null_value_to_array(array);
        }
        else {
            NSLog(@"unknown type: %@", element);
        }
    }
    return array;
}

void* convertJsonDictToVoidPointer(NSDictionary* jsonDict,void* json_alloc){
    
    void* json = create_json_object(json_alloc);
    [jsonDict enumerateKeysAndObjectsUsingBlock:^(NSString* key, id value, BOOL *stop) {
        const char *keyCstring = [key UTF8String];
        
        if ([value isKindOfClass:[NSString class]]) {
            const char *valueCstring = [value UTF8String];
            add_string_value(keyCstring,valueCstring, json);
        }
        else if ([value isKindOfClass:[NSNumber class]]) {
            const char *type = [value objCType];
            if (strcmp(type, @encode(char)) == 0 || strcmp(type, @encode(BOOL)) == 0) {
                add_bool_value(keyCstring, [value boolValue], json);
            }
            else if (strcmp([value objCType], @encode(int)) == 0 || strcmp([value objCType], @encode(long)) == 0 || strcmp([value objCType], @encode(long long)) == 0) {
                add_int64_value(keyCstring, [value longLongValue], json);
            }
            else if (strcmp([value objCType], @encode(float)) == 0 || strcmp([value objCType], @encode(double)) == 0) {
                add_double_value(keyCstring, [value doubleValue], json);
            }
            else{
                NSLog(@"unknown type of member NSNumber class: %@", value);
            }
            
        } else if ([value isKindOfClass:[NSArray class]]) {
            void* nestedArray = convertJsonArrayToVoidPointer(value,json_alloc);
            add_json_object_to_json(keyCstring, nestedArray, json);
            
        } else if ([value isKindOfClass:[NSDictionary class]]) {
            void* nestedDict = convertJsonDictToVoidPointer(value,json_alloc);
            add_json_object_to_json(keyCstring, nestedDict, json);
        } else if ([value isKindOfClass:[NSNull class]]) {
            add_null_value(keyCstring,json);
        }
        else {
            NSLog(@"Value of an unknown type: %@, found in JsonDict for key : %@", value, key);
            
        }
    }];
    return json;
}


void initialiseCTensor(CTensor* req) {
    req->dataType = UNKNOWN;
    req->shape = NULL;
    req->name = NULL;
    req->shapeLength = 0;
}

void convertStringToCTensor(NSString* str, CTensor* req) {
    initialiseCTensor(req);
    req->data =  convertStringToVoidPointer(str);
    req->dataType = STRING;
}

void* convertStringToVoidPointer(NSString* str) {
    const char* utf8Str = [str UTF8String];
    size_t length = strlen(utf8Str) + 1;
    char** mallocStr = (char**)malloc(sizeof(char*));
    if (mallocStr != NULL) {
        *mallocStr = (char*)malloc(length);
        if (*mallocStr != NULL) {
            strcpy(*mallocStr, utf8Str);
        }
    }
    return (void*)mallocStr;
}

void convertBoolToCTensor(NSNumber *data,  CTensor* req) {
    initialiseCTensor(req);
    req->data = convertBoolToVoidPointer(data);
    req->dataType = BOOLEAN;
}

void* convertBoolToVoidPointer(NSNumber* data) {
    bool* ptr = malloc(sizeof(bool));
    *ptr = [data boolValue];
    return (void*)ptr;
}

void* convertProtoObjectToVoidPointer(ProtoObjectWrapper* wrappedClass) {
    IosObject* obj = malloc(sizeof(IosObject));
    obj->type = IOS_PROTO_OBJECT;
    obj->obj = CFBridgingRetain(wrappedClass);
    return obj;
}

void convertInt32ToCTensor(NSNumber* data,  CTensor* req) {
    initialiseCTensor(req);
    req->data = convertInt32ToVoidPointer(data);
    req->dataType = INT32;
}

void* convertInt32ToVoidPointer(NSNumber* data) {
    int32_t* ptr = malloc(sizeof(int32_t));
    *ptr = [data intValue];
    return (void*)ptr;
}

void* convertFloatToVoidPointer(NSNumber* data) {
    float* ptr = malloc(sizeof(float));
    *ptr = [data floatValue];
    return (void*)ptr;
}

void* convertDoubleToVoidPointer(NSNumber* data) {
    double* ptr = malloc(sizeof(double));
    *ptr = [data doubleValue];
    return (void*)ptr;
}

void* convertInt64ToVoidPointer(NSNumber* data) {
    int64_t* ptr = malloc(sizeof(int64_t));
    *ptr = [data longLongValue];
    return (void*)ptr;
}

void* convertProtoAnyToVoidPointer(ProtoAnyWrapper* wrappedClass) {
    IosObject* obj = malloc(sizeof(IosObject));
    obj->type = IOS_ANY_OBJECT;
    obj->obj = CFBridgingRetain(wrappedClass);
    return obj;
}

void* convertProtoListToVoidPointer(ProtoListWrapper* wrappedClass) {
    IosObject* obj = malloc(sizeof(IosObject));
    obj->type = IOS_ARRAY;
    obj->obj = CFBridgingRetain(wrappedClass);
    return obj;
}

void* convertProtoMapToVoidPointer(ProtoMapWrapper* wrappedClass) {
    IosObject* obj = malloc(sizeof(IosObject));
    obj->type = IOS_MAP;
    obj->obj = CFBridgingRetain(wrappedClass);
    return obj;
}

void convertInt64ToCTensor(NSNumber* data, CTensor* req) {
    initialiseCTensor(req);
    req->data = convertInt32ToVoidPointer(data);
    req->dataType = INT64;
}

void convertFloatToCTensor(NSNumber* data, CTensor* req) {
    initialiseCTensor(req);
    req->data = convertFloatToVoidPointer(data);
    req->dataType = FLOAT;
}

void convertDoubleToCTensor(NSNumber* data, CTensor* req) {
    initialiseCTensor(req);
    req->data = convertDoubleToVoidPointer(data);
    req->dataType = DOUBLE;
}

@end
