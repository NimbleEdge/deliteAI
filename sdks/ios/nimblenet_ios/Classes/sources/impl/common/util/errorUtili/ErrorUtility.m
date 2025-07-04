/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "ErrorUtility.h"

NSDictionary* populateErrorReturnObject(int errorCode, NSString* message) {
    return @{
        @"status":@(false),
        @"data":[NSNull null],
        @"error":@{
            @"code":@(errorCode),
            @"message":message
        }
    };
}
