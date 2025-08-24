/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "executor_structs.h"
#import "nimble_net/c_tensor.h"

NSDictionary* cTensorsInitWithDict(CTensors* self_, NSDictionary* tensorsDict, void* json_alloc);

void cTensorDelete(CTensor* self_);
void cTensorsDelete(CTensors* self_, NSUInteger index);

// =================================================================================================

@interface InputConverter : NSObject

NimbleNetStatus* convertSingularInputToCTensor(id data, CTensor* child);
void* convertSingularInputtoVoidPointer(id data, int dataType, void* json_alloc);
void* convertArraytoVoidPointerWithJsonAlloc(NSArray* arrayData, NSUInteger arrayLength,
                                             int dataType, void* json_alloc);
void* convertArraytoVoidPointer(NSArray* arrayData, NSUInteger arrayLength, int dataType);

@end
