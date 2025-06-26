/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdlib.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <type_traits>

#include "ne_type_traits.hpp"

/**
 * @brief Configuration for the Time Manager.
 *
 * Indicates whether the time manager is operating in online mode.
 */
struct TimeManagerConfig {
  bool isOnline;
};

/**
 * @brief Provides time management utilities for the SDK.
 *
 * This class offers static methods for retrieving and manipulating time, including epoch time,
 * high-resolution benchmarking, and simulation support. It also manages configuration for time
 * handling modes (e.g., simulation vs. online).
 */
class Time {
  static int64_t _time;
  static TimeManagerConfig _timeManagerConfig;

 public:
  /**
   * @brief Set the current time (simulation mode only).
   *
   * @param timestamp The new time value to set (seconds since epoch).
   * @throws std::runtime_error if called outside simulation mode or if timestamp is not valid.
   */
  static void set_time(int64_t timestamp);

  constexpr static inline int64_t MILLIS_IN_SECS = 1'000;
  constexpr static inline int64_t MICROS_IN_SECS = 1'000 * MILLIS_IN_SECS;
  constexpr static inline int64_t NANOS_IN_SECS = 1'000 * MICROS_IN_SECS;
  constexpr static inline int64_t MICROS_IN_MILLIS = 1'000;

  /**
   * @brief Get the current time in seconds since the Unix epoch using system_clock.
   *
   * @return Current time in seconds since epoch.
   */
  static int64_t get_time();
  /**
   * @brief Get the current time in microseconds since the Unix epoch.
   *
   * @return Current time in microseconds since epoch.
   */
  static int64_t get_time_in_micro();
  /**
   * @brief Get the current time formatted for event store files.
   *
   * @return String in the format "<seconds>.<microseconds>".
   */
  static std::string get_time_for_event_store_file();
  /**
   * @brief Sleep until a specified number of seconds have elapsed from now.
   *
   * @param timeElapsed Number of seconds to elapse before waking.
   */
  static void sleep_until(int64_t timeElapsed);
  /**
   * @brief Get the current UTC date and time as a formatted string.
   *
   * @return String in the format "YYYY-MM-DD HH:MM:SS.ms+00:00".
   */
  static std::string get_date_UTC();

  /**
   * @brief Get the current high-resolution clock time point.
   *
   * @return High-resolution clock time point (for benchmarking).
   */
  static std::chrono::time_point<std::chrono::high_resolution_clock>
  get_high_resolution_clock_time();
  /**
   * @brief Get the elapsed time in microseconds since a given start time.
   *
   * @param start The start time point.
   * @return Elapsed time in microseconds.
   */
  static long long get_elapsed_time_in_micro(
      std::chrono::time_point<std::chrono::high_resolution_clock> start);
  /**
   * @brief Get the elapsed time in seconds since a given start time.
   *
   * @param start The start time point.
   * @return Elapsed time in seconds.
   */
  static long long get_elapsed_time_in_sec(
      std::chrono::time_point<std::chrono::high_resolution_clock> start);
  /**
   * @brief Reset the internal time (simulation mode only).
   */
  static void reset();
  /**
   * @brief Parse a timestamp string and return the corresponding epoch time in seconds.
   *
   * @param timestampString Timestamp in the format "YYYY-MM-DD HH:MM:SS".
   * @return Epoch time in seconds, or -1 on error.
   */
  static int64_t get_epoch_time_from_timestamp(const std::string& timestampString);

  /**
   * @brief Set the time manager configuration.
   *
   * @param timeManagerConfig_ The configuration to set.
   */
  static void setConfig(const TimeManagerConfig& timeManagerConfig_) {
    _timeManagerConfig = timeManagerConfig_;
  }
};

/**
 * @brief Represents a time duration with microsecond precision.
 *
 * Stores a time duration internally in microseconds and provides methods to access the duration
 * in various units. Supports arithmetic and comparison operations.
 */
class Duration {
  int64_t _timeMicros;

  constexpr explicit Duration(int64_t timeMicros) noexcept : _timeMicros(timeMicros) {}

 public:
  /**
   * @brief Construct a zero duration.
   */
  constexpr Duration() noexcept : Duration{0} {}

  /**
   * @brief Create a duration from seconds.
   *
   * @param time Number of seconds.
   * @return Duration representing the given seconds.
   */
  static constexpr Duration from_seconds(int64_t time) noexcept;
  /**
   * @brief Create a duration from milliseconds.
   *
   * @param time Number of milliseconds.
   * @return Duration representing the given milliseconds.
   */
  static constexpr Duration from_milliseconds(int64_t time) noexcept;
  /**
   * @brief Create a duration from microseconds.
   *
   * @param time Number of microseconds.
   * @return Duration representing the given microseconds.
   */
  static constexpr Duration from_microseconds(int64_t time) noexcept;

  /**
   * @brief Constant representing a zero duration.
   */
  static const Duration zero;

  /**
   * @brief Get the duration in seconds.
   *
   * @return Duration in seconds.
   */
  constexpr int64_t seconds() const noexcept;
  /**
   * @brief Get the duration in milliseconds.
   *
   * @return Duration in milliseconds.
   */
  constexpr int64_t milliseconds() const noexcept;
  /**
   * @brief Get the duration in microseconds.
   *
   * @return Duration in microseconds.
   */
  constexpr int64_t microseconds() const noexcept;

  /**
   * Supports addition, subtraction, multiplication, division, and all standard comparisons between durations.
   *
   * - operator+ / operator- : Add or subtract two durations.
   * - operator* / operator*= / operator/: Multiply or divide a duration by a factor.
   * - operator<, operator<=, operator>, operator>=, operator== : Compare durations.
   */
  
  constexpr Duration operator+(const Duration& other) const noexcept;
  constexpr Duration operator-(const Duration& other) const noexcept;
  constexpr Duration operator/(uint32_t factor) const noexcept;
  constexpr Duration operator*(uint32_t factor) const noexcept;
  constexpr Duration& operator*=(uint32_t factor) noexcept;
  constexpr bool operator<(Duration other) const noexcept;
  constexpr bool operator>(Duration other) const noexcept;
  constexpr bool operator<=(Duration other) const noexcept;
  constexpr bool operator>=(Duration other) const noexcept;
  constexpr bool operator==(Duration other) const noexcept;
};

/**
 * @brief DeviceTime allows tracking time intervals without relying on the system clock.
 */
class DeviceTime {
  timespec _deviceTime;
  static TimeManagerConfig _timeManagerConfig;

 public:
  /**
   * @brief Null device time constant.
   */
  static const DeviceTime null;

  /**
   * @brief Zero device time constant.
   */
  static const DeviceTime zero;

  /**
   * @brief Current device time (mutable in offline mode).
   */
  static DeviceTime currentTime;

  /**
   * @brief Get the current device time using a monotonic clock.
   *
   * @return Current DeviceTime.
   */
  static DeviceTime current_time() noexcept;

  /**
   * @brief Get the time difference between two DeviceTime points.
   *
   * @param d1 The first DeviceTime.
   * @param d2 The second DeviceTime.
   * @return Duration representing d1 - d2 in microseconds.
   */
  static Duration get_time_diff(DeviceTime d1, DeviceTime d2) noexcept;
  å
  /**
   * @brief Add a duration to this DeviceTime.
   *
   * @param duration The duration to add.
   * @return New DeviceTime offset by the given duration.
   */
  DeviceTime add_duration(Duration duration) const noexcept;

  /**
   * @brief Compare for equality with another DeviceTime.
   *
   * @param o The DeviceTime to compare.
   * @return True if both represent the same time.
   */
  bool operator==(const DeviceTime& o) const {
    return _deviceTime.tv_nsec == o._deviceTime.tv_nsec &&
           _deviceTime.tv_sec == o._deviceTime.tv_sec;
  }

  /**
   * @brief Set the time manager configuration for DeviceTime.
   *
   * @param timeManagerConfig_ The configuration to set.
   */
  static void setConfig(const TimeManagerConfig& timeManagerConfig_) {
    _timeManagerConfig = timeManagerConfig_;
  }

 private:
  /**
   * @brief Construct a DeviceTime from a timespec.
   *
   * @param deviceTime The timespec value.
   */
  constexpr DeviceTime(timespec deviceTime) noexcept : _deviceTime(deviceTime) {}
};

/**
 * @brief Alias for Duration representing time since epoch.
 */
using EpochTime = Duration;

/**
 * @brief PeggedDeviceTime allows conversion and comparison between device and server times.
 */
class PeggedDeviceTime {
  DeviceTime _baseDeviceTime;
  EpochTime _baseServerTime;

 public:
  /**
   * @brief Calculate the difference between two times in the pegged domain.
   *
   * @tparam T First time type (DeviceTime or EpochTime).
   * @tparam U Second time type (DeviceTime or EpochTime).
   * @param time1 First time value.
   * @param time2 Second time value.
   * @return Duration representing time1 - time2 in the pegged domain.
   */
  template <typename T, typename U>
  Duration time_diff(T time1, U time2) const {
    return get_server_time(time1) - get_server_time(time2);
  }

  /**
   * @brief Construct a PeggedDeviceTime with a base device and server time.
   *
   * @param baseDeviceTime The base DeviceTime.
   * @param baseServerTime The base EpochTime (server time).
   */
  constexpr PeggedDeviceTime(DeviceTime baseDeviceTime, EpochTime baseServerTime)
      : _baseDeviceTime(baseDeviceTime), _baseServerTime(baseServerTime) {}

  /**
   * @brief Default constructor (null base device time).
   */
  constexpr PeggedDeviceTime() : _baseDeviceTime(DeviceTime::null) {}

  /**
   * @brief Compare for equality with another PeggedDeviceTime.
   *
   * @param o The PeggedDeviceTime to compare.
   * @return True if both have the same base device and server time.
   */
  bool operator==(const PeggedDeviceTime& o) const {
    return _baseDeviceTime == o._baseDeviceTime && _baseServerTime == o._baseServerTime;
  }

  /**
   * @brief Compare for inequality with another PeggedDeviceTime.
   *
   * @param o The PeggedDeviceTime to compare.
   * @return True if not equal.
   */
  bool operator!=(const PeggedDeviceTime& o) const { return !(*this == o); }

  /**
   * @brief Get the server time corresponding to a given time value.
   *
   * @tparam T Time type (DeviceTime or EpochTime).
   * @param time The time value to convert.
   * @return Corresponding EpochTime (server time).
   */
  template <typename T, typename = std::enable_if_t<ne::is_one_of_v<T, DeviceTime, EpochTime>>>
  EpochTime get_server_time(T time) const {
    if constexpr (std::is_same_v<T, DeviceTime>) {
      return _baseServerTime + DeviceTime::get_time_diff(time, _baseDeviceTime);
    } else {
      return time;
    }
  }
};

/** Constexpr function definitions in header itself so they can be used in constant expressions */

constexpr Duration Duration::from_seconds(int64_t time_secs) noexcept {
  return Duration{time_secs * Time::MICROS_IN_SECS};
}

constexpr Duration Duration::from_milliseconds(int64_t time_ms) noexcept {
  return Duration{time_ms * Time::MICROS_IN_MILLIS};
}

constexpr Duration Duration::from_microseconds(int64_t time_us) noexcept {
  return Duration{time_us};
}

constexpr Duration Duration::zero = Duration::from_microseconds(0);

constexpr int64_t Duration::seconds() const noexcept { return _timeMicros / Time::MICROS_IN_SECS; }

constexpr int64_t Duration::milliseconds() const noexcept {
  return _timeMicros / Time::MILLIS_IN_SECS;
}

constexpr int64_t Duration::microseconds() const noexcept { return _timeMicros; }

constexpr Duration Duration::operator+(const Duration& o) const noexcept {
  return Duration{_timeMicros + o._timeMicros};
}

constexpr Duration Duration::operator-(const Duration& o) const noexcept {
  return Duration{_timeMicros - o._timeMicros};
}

constexpr Duration Duration::operator/(uint32_t factor) const noexcept {
  return Duration{_timeMicros / factor};
}

constexpr Duration Duration::operator*(uint32_t factor) const noexcept {
  return Duration{_timeMicros * factor};
}

constexpr Duration& Duration::operator*=(uint32_t factor) noexcept {
  _timeMicros *= 3;
  return *this;
}

constexpr bool Duration::operator<(Duration o) const noexcept {
  return _timeMicros < o._timeMicros;
}

constexpr bool Duration::operator<=(Duration o) const noexcept {
  return _timeMicros <= o._timeMicros;
}

constexpr bool Duration::operator>(Duration o) const noexcept {
  return _timeMicros > o._timeMicros;
}

constexpr bool Duration::operator>=(Duration o) const noexcept {
  return _timeMicros >= o._timeMicros;
}

constexpr bool Duration::operator==(Duration o) const noexcept {
  return _timeMicros == o._timeMicros;
}
