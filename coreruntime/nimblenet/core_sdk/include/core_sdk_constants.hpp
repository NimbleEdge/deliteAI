/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <string>

/**
 * @namespace coresdkconstants
 * @brief Contains constant values used throughout the Core SDK, including configuration
 * timing, file names, retry counts, and simulation-specific values.
 */
namespace coresdkconstants {

/**
 * @brief Sleep time in microseconds for long-running threads.
 *
 * If THREADSLEEPTIME cmake flag is defined, uses that value; otherwise defaults to 1,000,000 μs (1
 * second).
 */
#ifdef THREADSLEEPTIME
static inline const int LongRunningThreadSleepUTime = THREADSLEEPTIME;
#else
static inline const int LongRunningThreadSleepUTime = 1000000;
#endif

/**
 * @brief Default number of retry attempts for fetching cloud configuration.
 */
static inline const int DefaultFetchCloudConfigRetries = 3;

/**
 * @brief Default number of attempts to set thread priority.
 */
static inline const int DefaultThreadPriorityTries = 10;

/**
 * @brief Default number of retry attempts for sending crash logs.
 */
static inline const int DefaultSendCrashLogRetries = 3;

/**
 * @brief Duration (in days) after which files are eligible for deletion.
 */
static inline const float FileDeleteTimeInDays = 10;

/**
 * @brief Time interval (in seconds) to wait before allowing another config update.
 *
 * @note To be deprecated.
 */
static inline const int64_t TimeDurationGapForConfigUpdateInSecs = 86400;

/**
 * @brief Suffix to be added when saving active deployment configuration file on device.
 */
static inline const std::string DeploymentFileName = "deployment.txt";

/**
 * @brief Suffix to be added when saving previous (backup) deployment configuration file.
 */
static inline const std::string OldDeploymentFileName = "oldDeployment.txt";

/**
 * @brief Suffix to be added to the cloud configuration file stored on disk.
 */
static inline const std::string CloudConfigFileName = "cloudConfig.txt";

/**
 * @brief File path used to persist session-related data.
 */
static inline const std::string SessionFilePath = "/session.txt";

/**
 * @brief Default compatibility tag used when none is specified.
 */
static inline const std::string DefaultCompatibilityTag = "DEFAULT-TAG";

/**
 * @brief Maximum number of jobs the job scheduler can hold.
 */
static inline constexpr std::size_t JobSchedulerCapacity = 10;

#ifdef SIMULATION_MODE
/**
 * @brief Key to be used when adding a simulated user event.
 */
static inline const std::string SimulatedInputType = "simulatedInputType";

/**
 * @brief Value to be used for the corresponding SimulatedInputType key when the simulated input is
 * an event.
 */
static inline const std::string InputTypeEvent = "event";
#endif  // SIMULATION_MODE
}  // namespace coresdkconstants
