/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

/**
 * @brief Default spin time (in milliseconds) for thread pool worker threads.
 */
static int DEFAULT_THREAD_SPIN_TIME_IN_MS = 50;

/**
 * @brief A thread pool for managing and executing tasks concurrently using multiple worker threads.
 *
 * The ThreadPool class allows enqueuing tasks to be executed asynchronously by a pool of worker threads.
 * Tasks are processed in FIFO order.
 */
class ThreadPool {
 public:
  /**
   * @brief Constructs a ThreadPool with a specified number of worker threads.
   * 
   * @param numThreads Number of worker threads to create.
   */
  explicit ThreadPool(size_t numThreads);

  /**
   * @brief Destructor. Stops all threads and joins them.
   */
  ~ThreadPool();

  /**
   * @brief Spin time (in milliseconds) for worker threads before sleeping.
   *
   * Controls how long a worker thread spins before going to sleep if no tasks are available.
   */
  static std::atomic<int> spinTimeInMs;

  /**
   * @brief Enqueues a new task into the thread pool for asynchronous execution.
   *
   * @tparam F Callable type (function, lambda, etc.).
   * @tparam Args Argument types for the callable.
   * @param f Callable to execute.
   * @param args Arguments to pass to the callable.
   * @return std::future holding the result of the task.
   * @throws std::runtime_error if the thread pool is stopped.
   */
  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      if (stop) throw std::runtime_error("ThreadPool is stopped");

      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
  }

  /**
   * @brief Runs a single task from the thread pool queue in the current thread, if available.
   */
  void run_threadpool_task();

 private:
  /**
   * @brief Worker threads that execute tasks from the queue.
   */
  std::vector<std::thread> workers;

  /**
   * @brief Queue of tasks to be executed by the worker threads.
   */
  std::queue<std::function<void()>> tasks;

  /**
   * @brief Mutex for synchronizing access to the task queue.
   */
  std::mutex queueMutex;

  /**
   * @brief Condition variable for notifying worker threads of new tasks or shutdown.
   */
  std::condition_variable condition;

  /**
   * @brief Flag indicating whether the thread pool is stopping.
   */
  std::atomic<bool> stop;

  /**
   * @brief Function executed by each worker thread to process tasks.
   */
  void workerThread();
};
