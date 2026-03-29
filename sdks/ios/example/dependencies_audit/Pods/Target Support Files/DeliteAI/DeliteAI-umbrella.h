#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else
#ifndef FOUNDATION_EXPORT
#if defined(__cplusplus)
#define FOUNDATION_EXPORT extern "C"
#else
#define FOUNDATION_EXPORT extern
#endif
#endif
#endif

#import "CrashReporterUtil.h"
#import "ErrorUtility.h"
#import "InputConverter.h"
#import "OutputConverter.h"
#import "FunctionPointersImpl.h"
#import "NimbleNetController.h"
#import "deliteAI.h"

FOUNDATION_EXPORT double DeliteAIVersionNumber;
FOUNDATION_EXPORT const unsigned char DeliteAIVersionString[];

