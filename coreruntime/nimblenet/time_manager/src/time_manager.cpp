/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "time_manager.hpp"

#include <sys/time.h>
#include <time.h>

#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif  // __MACH__

#include "logger.hpp"

int64_t Time::_time;

TimeManagerConfig Time::_timeManagerConfig;

int64_t Time::get_time() {
#ifdef SIMULATION_MODE
  return _time;
#else
  auto now = std::chrono::system_clock::now();
  auto seconds_since_epoch =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
  return seconds_since_epoch;
#endif
}

std::string Time::get_time_for_event_store_file() {
  auto now = std::chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  int64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  int64_t micros =
      std::chrono::duration_cast<std::chrono::microseconds>(duration).count() % 1000000;
  return std::to_string(seconds) + "." + std::to_string(micros);
}

int64_t Time::get_time_in_micro() {
  auto now = std::chrono::system_clock::now();
  auto microseconds_since_epoch =
      std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
  return microseconds_since_epoch;
}

void Time::sleep_until(int64_t secondsToElapse) {
  auto now = std::chrono::system_clock::now();
  auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto next_second = now_seconds + std::chrono::seconds(secondsToElapse);
  std::this_thread::sleep_until(next_second);
}

std::string Time::get_date_UTC() {
  auto now = std::chrono::system_clock::now();
  auto currentTimePointCast = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  int milliseconds = (currentTimePointCast.time_since_epoch().count()) % 1000;

  std::time_t time_now = std::chrono::system_clock::to_time_t(now);
  std::chrono::system_clock::time_point timePoint =
      std::chrono::system_clock::from_time_t(time_now);

  // std::gmtime is not threadsafe, it returns a pointer to an internal static variable
  std::tm timeinfo;
  {
    static auto gmtimeMutex = new std::mutex{};
    std::lock_guard<std::mutex> gmtimeLockGuard{*gmtimeMutex};
    struct std::tm* timeinfoPtr = std::gmtime(&time_now);
    memcpy(&timeinfo, timeinfoPtr, sizeof(timeinfo));
  }

  char buffer[20];
  strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &timeinfo);
  char currentTime[30] = "";
  snprintf(currentTime, 30, "%s.%03d+00:00", buffer, milliseconds);
  return currentTime;
}

int64_t Time::get_epoch_time_from_timestamp(const std::string& timestampString) {
  std::tm tm = {};
  std::istringstream iss(timestampString);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail()) {
    LOG_TO_ERROR("Error parsing timestamp=%s", timestampString.c_str());
    return -1;
  }
  auto time = std::mktime(&tm) - timezone;  // timezone is defined in time.h
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(time);
  int64_t ret = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
  return ret;
}

std::chrono::time_point<std::chrono::high_resolution_clock> Time::get_high_resolution_clock_time() {
  return std::chrono::high_resolution_clock::now();
}

long long Time::get_elapsed_time_in_sec(
    std::chrono::time_point<std::chrono::high_resolution_clock> start) {
  const auto end = get_high_resolution_clock_time();
  return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}

long long Time::get_elapsed_time_in_micro(
    std::chrono::time_point<std::chrono::high_resolution_clock> start) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::high_resolution_clock::now() - start)
      .count();
}

void Time::set_time(int64_t timestamp) {
#ifdef SIMULATION_MODE
  if (_time > timestamp) {
    LOG_TO_ERROR("%s", "Time cannot be set to a value less than or equal to the current time");
    throw std::runtime_error(
        "Time cannot be set to a value less than or equal to the current time");
    return;
  }
  _time = timestamp;

  if (!_timeManagerConfig.isOnline) {
    DeviceTime::currentTime = DeviceTime::zero.add_duration(Duration::from_seconds(_time));
  }

#else   // SIMULATION_MODE
  throw std::runtime_error("Time cannot be set outside of simulation mode");
#endif  // SIMULATION_MODE
}

void Time::reset() { _time = 0; }

TimeManagerConfig DeviceTime::_timeManagerConfig;
const DeviceTime DeviceTime::null = DeviceTime{timespec{0, 0}};
const DeviceTime DeviceTime::zero = DeviceTime{timespec{0, 0}};

timespec get_zero_timespec() {
  timespec t;
  t.tv_nsec = 0;
  t.tv_sec = 0;
  return t;
}

DeviceTime DeviceTime::currentTime(get_zero_timespec());

DeviceTime DeviceTime::current_time() noexcept {
  if (!_timeManagerConfig.isOnline) {
    return DeviceTime::currentTime;
  }

  timespec time;

#ifdef __MACH__
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  time.tv_sec = mts.tv_sec;
  time.tv_nsec = mts.tv_nsec;
#else   // __MACH__
  /*
    CLOCK_BOOTTIME (since Linux 2.6.39; Linux-specific)
            A nonsettable system-wide clock that is identical to
            CLOCK_MONOTONIC, except that it also includes any time that
            the system is suspended.  This allows applications to get a
            suspend-aware monotonic clock without having to deal with the
            complications of CLOCK_REALTIME
  */
  clock_gettime(CLOCK_BOOTTIME, &time);
#endif  // __MACH__
  return DeviceTime{time};
}

Duration DeviceTime::get_time_diff(DeviceTime d1, DeviceTime d2) noexcept {
  const auto time_sec = d1._deviceTime.tv_sec - d2._deviceTime.tv_sec;
  const auto time_nsec = d1._deviceTime.tv_nsec - d2._deviceTime.tv_nsec;

  // This calculation works even when time_nsec becomes negative. For example:
  // 2s 10ms - 1s 50ms = 1s -40ms = 1000ms - 40ms - 960ms
  // Or = 2010 - 1050 = 960 ms
  return Duration::from_microseconds(time_sec * 1'000'000 + time_nsec / 1'000);
}

DeviceTime DeviceTime::add_duration(Duration duration) const noexcept {
  DeviceTime res = *this;
  const auto duration_micros = duration.microseconds();
  res._deviceTime.tv_nsec += (duration_micros % 1'000'000) * 1'000;
  res._deviceTime.tv_sec += duration_micros / 1'000'000;
  if (res._deviceTime.tv_nsec >= 1'000'000'000) {
    res._deviceTime.tv_nsec -= 1'000'000'000;
    res._deviceTime.tv_sec += 1;
  }

  return res;
}
