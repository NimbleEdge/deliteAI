/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <DeliteAI/DeliteAI-Swift.h>
#import "nimblejson.hpp"
#import "executor_structs.h"
#import "OutputConverter.h"
#import "ErrorUtility.h"

@implementation OutputConverter : NSObject

id castDataFromCTensor(CTensor *tensor){
    void* data = tensor->data;
    switch (tensor->dataType) {
        case STRING: {
            char *castedData = ((char **)tensor->data)[0];
            NSString *string = [NSString stringWithUTF8String:castedData];
            return string;
        }
        case INT32: {
            return @(*(int *)tensor->data);
        }
        case BOOLEAN: {
            bool castedData = *(bool *)tensor->data;
            return @(castedData);
        }
            
        case FLOAT: {
            float castedData = *(float *)tensor->data;
            return @(castedData);
        }
            
        case DOUBLE: {
            double castedData = *(double *)tensor->data;
            return @(castedData);
        }
            
        case INT64: {
            int64_t castedData = *(int64_t *)tensor->data;
            return @(castedData);
        }
        case FE_OBJ: {
            IosObject* proto = (IosObject *)(tensor->data);
            switch (proto->type) {
                case IOS_PROTO_OBJECT: {
                    return (__bridge ProtoObjectWrapper*) proto->obj;
                }
                case IOS_ANY_OBJECT: {
                    return (__bridge ProtoAnyWrapper*) proto->obj;
                }
                case IOS_MAP: {
                    return (__bridge ProtoMapWrapper*) proto->obj;
                }
                case IOS_ARRAY: {
                    return (__bridge ProtoListWrapper*) proto->obj;
                }
            }
        }
    }
    return NULL;
}

NSDictionary* convertCTensorsToNSDictionary(NimbleNetStatus* status,CTensors ctensors, void* json_alloc) {
    NSDictionary *resultDict = [NSMutableDictionary dictionary];
    NSDictionary *outputsTensorDict= [NSMutableDictionary dictionary];
    
    
    if(status==NULL){
        for (int i = 0; i < ctensors.numTensors; i++) {
            CTensor tensor = ctensors.tensors[i];
            NSString *name = [NSString stringWithUTF8String:tensor.name];
            
            int dataLength = tensor.shapeLength;
            NSMutableArray *shape = [NSMutableArray array];
            int totalArrayLength = 1;
            
            for (int j = 0; j < tensor.shapeLength; j++) {
                [shape addObject:@(tensor.shape[j])];
                totalArrayLength*=tensor.shape[j];
            }
            id data = nil;
            if(dataLength == 0){
                switch (tensor.dataType) {
                    case STRING: {
                        char *castedData = ((char **)tensor.data)[0];
                        NSString *string = [NSString stringWithUTF8String:castedData];
                        data = string;
                        break;
                    }
                    case INT32: {
                        int castedData = *(int *)tensor.data;
                        data = @(castedData);
                        break;
                    }
                        
                    case BOOLEAN: {
                        bool castedData = *(bool *)tensor.data;
                        data = @(castedData);
                        break;
                    }
                        
                    case FLOAT: {
                        float castedData = *(float *)tensor.data;
                        data = @(castedData);
                        break;
                    }
                        
                    case DOUBLE: {
                        double castedData = *(double *)tensor.data;
                        data = @(castedData);
                        break;
                    }
                        
                    case INT64: {
                        int64_t castedData = *(int64_t *)tensor.data;
                        data = @(castedData);
                        break;
                    }
                        
                    case JSON: {
                        void *jsonIterator = create_json_iterator(tensor.data, json_alloc);
                        NSDictionary *dict = convertvoidPointertoJsonObject(jsonIterator, json_alloc);
                        data = dict ? dict : [NSNull null];
                        break;
                    }
                    case FE_OBJ: {
                        IosObject* proto = (IosObject *)(tensor.data);
                        switch (proto->type) {
                            case IOS_PROTO_OBJECT: {
                                ProtoObjectWrapper* castedObject = (__bridge ProtoObjectWrapper*) proto->obj;
                                data = castedObject;
                                break;
                            }
                            case IOS_ANY_OBJECT: {
                                ProtoAnyWrapper* castedObject = (__bridge ProtoAnyWrapper*) proto->obj;
                                data = castedObject;
                                break;
                            }
                            case IOS_MAP: {
                                NSDictionary* dict = (__bridge NSDictionary*) proto->obj;
                                data = dict;
                                break;
                            }
                            case IOS_ARRAY: {
                                NSArray* array = (__bridge NSArray*) proto->obj;
                                data = array;
                                break;
                            }
                        }
                        
                        break;
                }
                        
                    default: {
                        data = [NSNull null];
                        return populateErrorReturnObject(5000, @"Output type not supported");
                        break;
                    }
                }
                
                
            }
            else{
                NSMutableArray *dataArray = [NSMutableArray array];
                switch (tensor.dataType) {
                    case STRING: {
                        char **castedArrayData = (char **)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            if (castedArrayData[j] != NULL) {
                                NSString *string = [NSString stringWithUTF8String:castedArrayData[j]];
                                [dataArray addObject:string ?: [NSNull null]];
                                
                            } else {
                                [dataArray addObject:[NSNull null]];
                            }
                        }
                        break;
                    }
                        
                    case INT32:{
                        int* castedArrayData = (int *)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            [dataArray addObject:@(castedArrayData[j])];
                        }
                        break;
                    }
                        
                    case BOOLEAN:{
                        bool* castedArrayData = (bool *)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            [dataArray addObject:@(castedArrayData[j])];
                        }
                        break;
                    }
                        
                    case FLOAT:{
                        float* castedArrayData = (float *)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            [dataArray addObject:@(castedArrayData[j])];
                        }
                        break;
                    }
                    case DOUBLE:{
                        double* castedArrayData = (double *)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            [dataArray addObject:@(castedArrayData[j])];
                        }
                        break;
                    }
                    case INT64:{
                        int64_t *castedArrayData = (int64_t *)tensor.data;
                        for (int j = 0; j < totalArrayLength; j++) {
                            [dataArray addObject:@(castedArrayData[j])];
                        }
                        break;
                    }
                        
                    case JSON_ARRAY:{
                        void* jsonIterator = create_json_iterator(tensor.data,json_alloc);
                        NSArray* jsonArray = convertvoidPointertoJSONArray(jsonIterator, json_alloc);
                        [dataArray addObject:jsonArray];
                        break;
                    }
                        
                    default:
                        return populateErrorReturnObject(5000, @"Output type not supported");
                }
                data = dataArray;
            }
            
            NSNumber *dataType = @(tensor.dataType);
            
            NSDictionary *tensorData = @{
                @"data": data,
                @"shape": shape,
                @"type": dataType,
            };
            [outputsTensorDict setValue:tensorData forKey:name];
        }
        
    }
    
    resultDict = @{
        @"status":@(status==NULL?true:false),
        @"data":status!=NULL?[NSNull null]:@{
            @"outputs":outputsTensorDict,
            @"size":@(ctensors.numTensors)
        },
        @"error":status==NULL?[NSNull null]:@{
            @"code":@(status->code),
            @"message":@(status->message)
        }
    };
    
    
    return [resultDict copy];
}

NSMutableArray* convertvoidPointertoJSONArray( void* jsonIterator, void* json_alloc){
    NSMutableArray *resultArray = [NSMutableArray array];
    
    while(true){
        
        JsonOutput* nextElement = get_next_json_element(jsonIterator,json_alloc);
        if (nextElement->isEnd) {
            return resultArray;
        }
        
        switch (nextElement->dataType) {
            case STRING: {
                const char* stringValue = nextElement->value.s;
                NSString *value = [NSString stringWithUTF8String:stringValue];
                [resultArray addObject:value];
                break;
            }
                
            case DOUBLE: {
                double doubleValue = nextElement->value.d;
                [resultArray addObject:@(doubleValue)];
                break;
            }
            case BOOLEAN: {
                bool boolValue = nextElement->value.b;
                [resultArray addObject:@(boolValue)];
                break;
            }
            case INT64: {
                int64_t intValue = nextElement->value.i;
                [resultArray addObject:@(intValue)];
                break;
            }
            case JSON_ARRAY: {
                NSMutableArray *nestedArray;
                nestedArray = convertvoidPointertoJSONArray(nextElement->value.obj,json_alloc);
                [resultArray addObject:nestedArray];
                break;
            }
            case JSON: {
                NSDictionary *nestedObject = convertvoidPointertoJsonObject(nextElement->value.obj, json_alloc);
                [resultArray addObject:nestedObject];
                break;
            }
            default: {
                NSLog(@"Unknown data type encountered.");
                break;
            }
        }
        
    }
    
    return resultArray;
}

NSDictionary* convertvoidPointertoJsonObject(void* jsonIterator, void* json_alloc){
    NSMutableDictionary *resultDict = [NSMutableDictionary dictionary];
    
    while(true){
        
        JsonOutput* nextElement = get_next_json_element(jsonIterator,json_alloc);
        if (nextElement->isEnd) {
            return resultDict;
        }
        
        
        const char* keyPointer = nextElement->key;
        NSString *key = [NSString stringWithUTF8String:keyPointer];
        switch (nextElement->dataType) {
            case STRING: {
                const char* stringValue = nextElement->value.s;
                NSString *value = [NSString stringWithUTF8String:stringValue];
                resultDict[key] = value;
                break;
            }
                
            case BOOLEAN: {
                bool boolValue = nextElement->value.b;
                resultDict[key] = @(boolValue);
                break;
            }
                
            case DOUBLE: {
                double doubleValue = nextElement->value.d;
                resultDict[key] = @(doubleValue);
                break;
            }
            case INT64: {
                int64_t intValue = nextElement->value.i;
                resultDict[key] = @(intValue);
                break;
            }
            case JSON_ARRAY: {
                NSMutableArray *arrayValue;
                arrayValue = convertvoidPointertoJSONArray(nextElement->value.obj,json_alloc);
                resultDict[key] = arrayValue;
                break;
            }
            case JSON: {
                NSDictionary *nestedObject = convertvoidPointertoJsonObject(nextElement->value.obj,json_alloc);
                resultDict[key] = nestedObject;
                break;
            }
            default: {
                NSLog(@"Unknown data type encountered.");
                break;
            }
        }
    }
    
    return resultDict;
    
}

@end
