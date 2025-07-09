/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "NimbleNetController.h"
#import "nimblenet.h"
#import <DeliteAI/FunctionPointersImpl.h>
#import <Network/Network.h>
#import <DeliteAI/DeliteAI-Swift.h>
#import "executor_structs.h"
#import "nimble_net_util.hpp"
#import "nimblejson.hpp"
#import <sys/utsname.h>
#import "InputConverter.h"
#import "OutputConverter.h"
#import "ErrorUtility.h"

static NSString *const nimbleSDKFolder = @"nimbleSDK";
@import CrashReporter;

@interface NimbleNetController()

@property (strong, nonatomic) nw_path_monitor_t pathMonitor;
@property (strong, nonatomic) HardwareInfo *hardwareInfo;

@end

@interface CUserInputWrapper : NSObject
typedef struct SinglePreprocessorInput SinglePreprocessorInput;
@end

@implementation NimbleNetController


-(NSDictionary*)initialize_nimblenet_controller:(NSString*)configJson assetsJson:(NSString*)assetsJson{
    NSLog(@"init called  detected!");
    initClientFunctionPointers();
    
    NSString* nimbleSdkDirectoryPath = getNimbleSdkDirectoryPath();
    NSDictionary *sizes = getInternalStorageFolderSizes(nimbleSdkDirectoryPath);
    NSError *directorySizeError;
    NSData *directorySizeJsonData = [NSJSONSerialization dataWithJSONObject:sizes options:0 error:&directorySizeError];
    
    NSString* nimbleSdkDirectory = createNimbleSdkDirectory();
    
    configJson = setInternalDeviceIdInConfig(configJson);
    
    if(configJson==nil){
        return populateErrorReturnObject(500, @"Invalid config json");
    }
    
    if(nimbleSdkDirectory!=nil){
        // Subscribe to internet connection notifications.
        self.pathMonitor = nw_path_monitor_create();
        
        __weak typeof(self) weakSelf = self;
        nw_path_monitor_set_update_handler(self.pathMonitor, ^(nw_path_t  _Nonnull path) {
            
            __strong typeof(weakSelf) strongSelf = weakSelf;
            
            if (strongSelf) {
                
                nw_path_status_t status = nw_path_get_status(path);
                
                // Check the path status
                if (status == nw_path_status_satisfied) {
                    [self internet_switched_on_controller];
                } else {
                    NSLog(@"No internet connection");
                }
                
            }
            
        });
        nw_path_monitor_start(self.pathMonitor);
        
        if (assetsJson != nil) {
                    NimbleNetStatus* status = load_modules([assetsJson UTF8String], [nimbleSdkDirectory UTF8String]);
                    if (status != nil) {
                        NSDictionary* res = @{
                            @"status":@false,
                            @"data":[NSNull null],
                            @"error":@{
                                @"code":@(status->code),
                                @"message":@(status->message)
                            }
                        };
                        return res;
                    }
                }
        
        NimbleNetStatus* status = initialize_nimblenet([configJson UTF8String], [nimbleSdkDirectory UTF8String]);
        self.hardwareInfo = [[HardwareInfo alloc] init];
        NSString *staticMetricsJsonString = [self.hardwareInfo getStaticDeviceMetrics];
        
        if(status == NULL){
            if (!directorySizeError){
                NSString *jsonString = [[NSString alloc] initWithData:directorySizeJsonData encoding:NSUTF8StringEncoding];
                const char *jsonCString = [jsonString UTF8String];
                write_metric(INTERNALSTORAGEMETRICS, jsonCString);
            }
            
            const char *metricsCString = [staticMetricsJsonString UTF8String];
            [self write_metric_controller:STATICDEVICEMETRICS metricJson:metricsCString];
        }
        
        [self initCrashReporter];
        
        NSDictionary* res = @{
            @"status":@(status==NULL?true:false),
            @"data":[NSNull null],
            @"error":status==NULL?[NSNull null]:@{
                @"code":@(status->code),
                @"message":@(status->message)
            }
        };
        
        if(status!=NULL){
            deallocate_nimblenet_status(status);
            
        }
        return res;
        
    }
    
    return populateErrorReturnObject(500, @"Error creating directory");
}
-(void)restartSession{
    NSString *sessionId = @"";
    const char *csessionId = [sessionId UTF8String];
    update_session(csessionId);
};

-(void)restartSessionWithId:(NSString*)sessionId{
    const char *csessionId = [sessionId UTF8String];
    update_session(csessionId);
    
};

NSDictionary* convertCUserEventsDataToNSDictionary(CUserEventsData* data) {
    
    NSString *eventType = data->eventType ? [NSString stringWithUTF8String:data->eventType] : nil;
    NSString *eventJsonString = data->eventJsonString ? [NSString stringWithUTF8String:data->eventJsonString] : nil;
    
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    if (eventType) {
        dict[@"eventType"] = eventType;
    }
    if (eventJsonString) {
        dict[@"eventJsonString"] = eventJsonString;
    }
    return dict;
}


-(NSDictionary*)add_event_controller:(NSString*)eventMapJsonString eventType:(NSString*)eventType{
    CUserEventsData cUserEventsData ;
    NimbleNetStatus* status = add_event([eventMapJsonString UTF8String], [eventType UTF8String],&cUserEventsData);
    NSDictionary* userEventdataDict;
    if(status == NULL){
        userEventdataDict = convertCUserEventsDataToNSDictionary(&cUserEventsData);
        deallocate_c_userevents_data(&cUserEventsData);
    }
    
    NSDictionary* res = @{
        @"status":@(status==NULL?true:false),
        @"data": (status != NULL) ? [NSNull null] : userEventdataDict,
        @"error":status==NULL?[NSNull null]:@{
            @"code":@(status->code),
            @"message":@(status->message)
        }
    };
    
    deallocate_nimblenet_status(status);
    
    return res;
}

-(NSDictionary*)is_ready_controller{
    NimbleNetStatus* status = is_ready();
    
    NSDictionary* res = @{
        @"status":@(status==NULL?true:false),
        @"data":[NSNull null],
        @"error":status==NULL?[NSNull null]:@{
            @"code":@(status->code),
            @"message":@(status->message)
        }
    };
    
    if(status!=NULL){
        deallocate_nimblenet_status(status);
    }
    
    return res;
};

- (void)initCrashReporter {
    if (![CrashReporterUtil is_debugger_attached]) {
        PLCrashReporterConfig *config = [[PLCrashReporterConfig alloc] initWithSignalHandlerType: PLCrashReporterSignalHandlerTypeBSD
                                                                           symbolicationStrategy: PLCrashReporterSymbolicationStrategyAll];
        PLCrashReporter *crashReporter = [[PLCrashReporter alloc] initWithConfiguration: config];
        
        NSError *error;
        if (![crashReporter enableCrashReporterAndReturnError: &error]) {
            NSLog(@"Warning: Could not enable crash reporter: %@", error);
            return;
        }
        
        if ([crashReporter hasPendingCrashReport]) {
            NSError *error;
            NSData *data = [crashReporter loadPendingCrashReportDataAndReturnError: &error];
            if (data == nil) {
                NSLog(@"Failed to load crash report data: %@", error);
                return;
            }
            
            PLCrashReport *report = [[PLCrashReport alloc] initWithData: data error: &error];
            if (report == nil) {
                NSLog(@"Failed to parse crash report: %@", error);
                return;
            }
            NSString *text = [PLCrashReportTextFormatter stringValueForCrashReport: report withTextFormat: PLCrashReportTextFormatiOS];
            
            [self parseCrashReportwithKeyword:text keyword:@"NimbleNetiOS"];
            [crashReporter purgePendingCrashReport];
        }
    }
}

- (NSString *)parseCrashReportwithKeyword:(NSString *)crashLog keyword:(NSString *)keyword {
    NSMutableString *result = [NSMutableString string];
    NSNumber *crashedThreadNumber = nil;
    NSArray *lines = [crashLog componentsSeparatedByString:@"\n"];
    
    BOOL metadataExtracted = NO;
    NSMutableString *metadata = [NSMutableString string];
    
    for (NSString *line in lines) {
        if ([line containsString:@"Crashed Thread:"]) {
            metadataExtracted = YES;
            break;
        }
        [metadata appendString:line];
        [metadata appendString:@"\n"];
    }
    
    [result appendString:metadata];
    [result appendString:@"\n"];
    
    for (NSString *line in lines) {
        if ([line containsString:@"Crashed Thread:"]) {
            NSString *threadNumString = [[line componentsSeparatedByString:@"Crashed Thread:"].lastObject stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            if (threadNumString && threadNumString.length > 0) {
                crashedThreadNumber = @([threadNumString intValue]);
                break;
            }
        }
    }
    
    
    BOOL isCrashedThread = NO;
    NSString *threadPrefix = [NSString stringWithFormat:@"Thread %@", crashedThreadNumber];
    
    for (NSString *line in lines) {
        if ([line containsString:threadPrefix]) {
            isCrashedThread = YES;
        } else if (isCrashedThread && ([line hasPrefix:@"Thread"] || [line hasPrefix:@"Binary Images:"])) {
            isCrashedThread = NO;
        }
        
        if (isCrashedThread && [line containsString:keyword]) {
            [result appendString:line];
            [result appendString:@"\n"];
        }
    }
    
    if (result.length == 0 || [result isEqualToString:[NSString stringWithFormat:@"%@\n", metadata]]) {
        return [NSString stringWithFormat:@"%@\nNo lines with the keyword '%@' found in the crashed thread.", metadata, keyword];
    }
    [self parseAndLogCrashLog:result];
    return result;
}

- (NSString *)parseAndLogCrashLog:(NSString *)log {
    NSArray<NSString *> *lines = [log componentsSeparatedByString:@"\n"];
    NSString *dateTime = nil;
    NSString *exceptionType = nil;
    NSString *exceptionCode = nil;
    NSMutableArray<NSString *> *backtrace = [NSMutableArray array];
    
    BOOL isBacktrace = NO;
    
    for (NSString *line in lines) {
        if ([line hasPrefix:@"Date/Time:"]) {
            dateTime = [[line substringFromIndex:[@"Date/Time:" length]] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            
            // Remove the "+00" time zone part
            NSRange timeZoneRange = [dateTime rangeOfString:@" +0000"];
            if (timeZoneRange.location != NSNotFound) {
                dateTime = [dateTime stringByReplacingCharactersInRange:timeZoneRange withString:@""];
            }
            
            // Append .000 for milliseconds
            dateTime = [dateTime stringByAppendingString:@".000"];
            
        } else if ([line hasPrefix:@"Exception Type:"]) {
            exceptionType = [[line substringFromIndex:[@"Exception Type:" length]] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        } else if ([line hasPrefix:@"Exception Codes:"]) {
            exceptionCode = [[line substringFromIndex:[@"Exception Codes:" length]] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
        } else if ([line isEqualToString:@""]) {
            // Stop capturing backtrace if we hit an empty line
            isBacktrace = YES;
        } else if (isBacktrace) {
            // Use a regex to check if the line contains an address or starts with a number
            NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:@"^(\\d+|0x[0-9A-Fa-f]+)" options:0 error:nil];
            NSRange range = [regex rangeOfFirstMatchInString:line options:0 range:NSMakeRange(0, line.length)];
            
            if (range.location != NSNotFound) {
                [backtrace addObject:[line stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]]];
            }
        }
    }
    
    if (!dateTime || !exceptionType || !exceptionCode) {
        return nil;
    }
    
    NSString *signalCode = exceptionType;
    NSString *errorCode = exceptionCode;
    NSString *backtraceString = [backtrace componentsJoinedByString:@"\\n"];
    
    NSString *output = [NSString stringWithFormat:@"METRICS::: %@ ::: crash ::: {\"errorCode\":\"%@\",\"backtrace\":\"%@\",\"signalCode\":\"%@\"}",
                        dateTime, errorCode, backtraceString, signalCode];
    
    const char *cString = [output UTF8String];
    send_crash_log(cString);
    return output;
}

-(void)write_metric_controller:(const char*)metricType metricJson:(const char*)metricJson{
    write_metric(metricType, metricJson);
};

-(void)write_run_method_metric_controller:(const char*)modelId totalTimeInUSecs:(int64_t)totalTimeInUSecs{
    write_run_method_metric(modelId, totalTimeInUSecs);
};

-(void)internet_switched_on_controller{
    internet_switched_on();
};

-(Boolean)load_task_controller:(NSString *)taskName taskCode:(NSString *)taskCode{
    return false;
}

-(NSDictionary*)run_task_controller:(NSString *)taskName
               modelInputsWithShape:(NSDictionary *)modelInputsWithShape
{
    
    void* json_alloc = create_json_allocator();
    NSMutableArray<CUserInputWrapper *> *userInputArray = [NSMutableArray array];
    
    NSUInteger modelInputsLength = [modelInputsWithShape count];
    
    
    CTensors req;
    CTensors ret;
    req.numTensors = modelInputsLength;
    req.tensors = (CTensor*)malloc((req.numTensors) * sizeof(CTensor));
    
    
    //model input transformer
    
    NSArray *keys = modelInputsWithShape.allKeys;
    for (NSUInteger index = 0; index < keys.count; index++) {
        NSString *inputName = keys[index];
        NSDictionary* modelInputObjectData = modelInputsWithShape[inputName];
        NSArray* modelInputObjectShape = modelInputsWithShape[inputName][@"shape"];
        int shapeArrayLength = 0;
        void* voidCastedData;
        int inputDataType;
        int64_t* int64ShapeArray = NULL;
        if(modelInputObjectShape != [NSNull null]) {
            shapeArrayLength = [modelInputObjectShape count];
            int64ShapeArray = (int64_t *)malloc(sizeof(int64_t) * shapeArrayLength);
            for (NSUInteger i = 0; i < shapeArrayLength; i++) {
                int64ShapeArray[i] = [(NSNumber*)modelInputObjectShape[i] longLongValue];
            }
            
            NSArray* arrayData = modelInputObjectData[@"data"];
            inputDataType = [modelInputObjectData[@"type"] intValue];
            NSUInteger arrayLength = [arrayData count];
            voidCastedData = convertArraytoVoidPointerWithJsonAlloc(arrayData, arrayLength, inputDataType,json_alloc);
            
        }
        else{
            id data = modelInputObjectData[@"data"];
            inputDataType = [modelInputObjectData[@"type"] intValue];
            voidCastedData = convertSingularInputtoVoidPointer(data, inputDataType,json_alloc);
        }
        
        
        if(voidCastedData==nil){
            freeCTensors(&req,index);
            if (int64ShapeArray != NULL) {
                free(int64ShapeArray);
                int64ShapeArray = NULL;
            }
            deallocate_json_allocator(json_alloc);
            return populateErrorReturnObject(5000,@"Datatype not yet supported");
        }
        
        req.tensors[index].name = strdup([inputName UTF8String]);
        req.tensors[index].data = voidCastedData;
        req.tensors[index].dataType = inputDataType;
        req.tensors[index].shape = int64ShapeArray;
        req.tensors[index].shapeLength = shapeArrayLength;
    }
    
    CFTimeInterval startTime = CACurrentMediaTime();
    NimbleNetStatus *nimbleNetStatus = run_method([taskName UTF8String], req, &ret);
    if (nimbleNetStatus == NULL) {
        CFTimeInterval elapsedTime = CACurrentMediaTime() - startTime;
        long long int elapsedTimeinMicro = (long long int)(elapsedTime * 1000000);
        const char *cString = [taskName UTF8String];
        write_run_method_metric(cString, elapsedTimeinMicro);
    }
    
    bool status = false;
    
    if(nimbleNetStatus==NULL){
        status = true;
    }
    
    NSDictionary *output = convertCTensorsToNSDictionary(nimbleNetStatus,ret,json_alloc);
    
    
    freeCTensors(&req,req.numTensors);
    
    if(nimbleNetStatus!=NULL){
        deallocate_nimblenet_status(nimbleNetStatus);
    }
    else{
        deallocate_output_memory2(&ret);
    }
    
    deallocate_json_allocator(json_alloc);
    return output;
}

-(void)reset_app_state_controller{
    reset();
    delete_database();
}

//private functions
void freeCTensor(CTensor* tensor){
    if (tensor->dataType == STRING){
        
        int totalArrayLength = 1;
        
        for (int j = 0; j < tensor->shapeLength; j++) {
            totalArrayLength*=tensor->shape[j];
        }
        
        for(int k = 0; k<totalArrayLength;k++ ){
            char ** stringArray = (char**) (tensor->data);
            free(stringArray[k]);
        }
        free(tensor->data);
    } else if (tensor->dataType != JSON && tensor->dataType != JSON_ARRAY){
        free(tensor->data);
    }
    
    if (tensor->shape != NULL) {
        free(tensor->shape);
    }
    free(tensor->name);

}
void freeCTensors(CTensors* req,NSUInteger index){
    for (int i = 0; i < index; i++) {
        freeCTensor(&req->tensors[i]);
    }
    free(req->tensors);
}

NSString *getNimbleSdkDirectoryPath(void) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *subdirectory = nimbleSDKFolder;
    NSString *directoryPath = [documentsDirectory stringByAppendingPathComponent:subdirectory];
    
    return directoryPath;
}

NSString *createNimbleSdkDirectory(void) {
    
    NSString *directoryPath = getNimbleSdkDirectoryPath();
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:directoryPath]) {
        NSError *error = nil;
        if ([fileManager createDirectoryAtPath:directoryPath withIntermediateDirectories:YES attributes:nil error:&error]) {
            return directoryPath;
        }
    } else {
        return directoryPath;
    }
    
    return nil;
}


// This function calculates the total size of a given folder (including its subdirectories).
unsigned long long folderSizeAtPath(NSString *folderPath) {
    unsigned long long totalFolderSize = 0;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *filesArray = [fileManager subpathsOfDirectoryAtPath:folderPath error:nil];
    
    if (!filesArray) {
        return 0;
    }
    
    for (NSString *fileName in filesArray) {
        NSString *filePath = [folderPath stringByAppendingPathComponent:fileName];
        NSDictionary *fileAttributes = [fileManager attributesOfItemAtPath:filePath error:nil];
        
        if ([fileAttributes[NSFileType] isEqualToString:NSFileTypeRegular]) {
            totalFolderSize += [fileAttributes[NSFileSize] unsignedLongLongValue];
        }
    }
    return totalFolderSize;
}

NSDictionary *getInternalStorageFolderSizes(NSString *folderPath) {
    NSArray *subdirectories = @[ @"metrics", @"logs" ];
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    unsigned long long nimbleSDKSize = folderSizeAtPath(folderPath);
    NSString *nimbleSDKDirectory = nimbleSDKFolder;
    result[nimbleSDKDirectory] = @(nimbleSDKSize);
    for (NSString *subdirectory in subdirectories) {
        NSString *subdirectoryPath = [folderPath stringByAppendingPathComponent:subdirectory];
        unsigned long long subdirectorySize = folderSizeAtPath(subdirectoryPath);
        result[subdirectory] = @(subdirectorySize);
    }
    
    return result;
}

NSString *setInternalDeviceIdInConfig(NSString *configJsonString){
    return [HardwareInfo setInternalDeviceIdInConfigWithConfigJsonString:configJsonString];
}

@end
