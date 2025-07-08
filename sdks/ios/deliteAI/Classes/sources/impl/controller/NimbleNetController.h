/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NimbleNetController_h
#define NimbleNetController_h
#import "executor_structs.h"
#import "nimble_net_util.hpp"
#import <Foundation/Foundation.h>


@interface NimbleNetController : NSObject
-(NSDictionary*)initialize_nimblenet_controller:(NSString *)configJson assetsJson:(NSString*)assetsJson;
-(NSDictionary*)add_event_controller:(NSString *)eventMapJsonString eventType:(NSString *)eventType;
-(NSDictionary*)is_ready_controller;
-(NSDictionary*)run_task_controller:(NSString *)taskName
               modelInputsWithShape:(NSDictionary *)modelInputsWithShape;
-(Boolean)load_task_controller:(NSString *)taskName taskCode:(NSString *)taskCode;

-(void)write_metric_controller:(const char*)metricType metricJson:(const char*)metricJson;
-(void)write_run_method_metric_controller:(const char*)modelId totalTimeInUSecs:(int64_t)totalTimeInUSecs;
-(void)internet_switched_on_controller;
-(void)reset_app_state_controller;
-(void)restartSession;
-(void)restartSessionWithId:(NSString *)sessionId;
void freeCTensor(CTensor* tensor);

@end

#endif
