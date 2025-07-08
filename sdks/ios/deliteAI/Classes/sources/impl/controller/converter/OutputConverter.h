/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import "executor_structs.h"

@interface OutputConverter : NSObject

id castDataFromCTensor(CTensor *tensor);
NSDictionary* convertCTensorsToNSDictionary(NimbleNetStatus* status,CTensors ctensors, void* json_alloc);
NSMutableArray* convertvoidPointertoJSONArray(void* jsonIterator, void* json_alloc);

@end
