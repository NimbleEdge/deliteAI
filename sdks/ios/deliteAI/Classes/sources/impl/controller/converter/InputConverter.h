/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import "executor_structs.h"

@interface InputConverter : NSObject

NimbleNetStatus* convertSingularInputToCTensor(id data,CTensor* child);
void* convertSingularInputtoVoidPointer(id data, int dataType, void* json_alloc);
void* convertArraytoVoidPointerWithJsonAlloc(NSArray* arrayData, int arrayLength, int dataType,void* json_alloc);
void* convertArraytoVoidPointer(NSArray* arrayData, int arrayLength, int dataType);


@end
