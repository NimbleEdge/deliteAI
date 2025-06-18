/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <string>

namespace coresdkconstants {
#ifdef THREADSLEEPTIME
static inline const int LongRunningThreadSleepUTime = THREADSLEEPTIME;
#else
static inline const int LongRunningThreadSleepUTime = 1000000;
#endif
static inline const int DefaultFetchCloudConfigRetries = 3;
static inline const int DefaultThreadPriorityTries = 10;
static inline const int DefaultSendCrashLogRetries = 3;
static inline const float FileDeleteTimeInDays = 10;
static inline const int64_t TimeDurationGapForConfigUpdateInSecs = 86400;
static inline const std::string DeploymentFileName = "deployment.txt";
static inline const std::string OldDeploymentFileName = "oldDeployment.txt";
static inline const std::string CloudConfigFileName = "cloudConfig.txt";
static inline const std::string SessionFilePath = "/session.txt";
static inline const std::string DefaultCompatibilityTag = "DEFAULT-TAG";
static inline constexpr std::size_t JobSchedulerCapacity = 10;

#ifdef SIMULATION_MODE
static inline const std::string SimulatedInputType = "simulatedInputType";
static inline const std::string InputTypeEvent = "event";
#endif  // SIMULATION_MODE
}  // namespace coresdkconstants
