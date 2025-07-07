/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thread_pool.hpp"

#include "client.h"
#include "logger.hpp"

std::atomic<int> ThreadPool::spinTimeInMs = DEFAULT_THREAD_SPIN_TIME_IN_MS;

// Constructor: Create worker threads
ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
  for (size_t i = 0; i < numThreads; ++i) {
    workers.emplace_back(&ThreadPool::workerThread, this);
  }
}

// Destructor: Join all threads
ThreadPool::~ThreadPool() {
  stop.store(true);
  condition.notify_all();  // Wake up all threads to finish execution
  for (std::thread& worker : workers) {
    if (worker.joinable()) worker.join();
  }
}

// Worker function that processes tasks
void ThreadPool::workerThread() {
  bool attached = false;
  auto spinEndTime = Time::get_high_resolution_clock_time();
  while (true) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(queueMutex);

      if (tasks.empty() && (Time::get_high_resolution_clock_time() > spinEndTime)) {
#ifdef ANDROID_ABI
        // detach before sleeping
        if (attached) {
          globalJvm->DetachCurrentThread();
          attached = false;
        }
#endif

        // sleep
        condition.wait(lock, [this] { return stop || !tasks.empty(); });
        spinEndTime = Time::get_high_resolution_clock_time() +
                      std::chrono::milliseconds(ThreadPool::spinTimeInMs);
      }

      if (stop) return;  // Exit thread if stopping and no tasks left
      if (tasks.empty()) continue;
      task = std::move(tasks.front());
      tasks.pop();
    }
#ifdef ANDROID_ABI
    // attach before doing tasks
    if (!attached) {
      globalJvm->AttachCurrentThread(&_threadLocalEnv, NULL);
      attached = true;
    }
#endif
    task();  // Execute task outside lock
    spinEndTime = Time::get_high_resolution_clock_time() +
                  std::chrono::milliseconds(ThreadPool::spinTimeInMs);
  }
}

void ThreadPool::run_threadpool_task() {
  std::function<void()> task;
  {
    std::unique_lock<std::mutex> lock(queueMutex);
    if (stop) return;  // Exit thread if stopping and no tasks left
    if (tasks.empty()) return;
    task = std::move(tasks.front());
    tasks.pop();
  }
  task();  // Execute task outside lock
}
