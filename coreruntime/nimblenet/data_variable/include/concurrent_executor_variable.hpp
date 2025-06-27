#pragma once

#include "data_variable.hpp"
#include "list_data_variable.hpp"
#include "thread_pool.hpp"
#include "variable_scope.hpp"

class ConcurrentExecutorVariable final : public DataVariable {
  static std::unique_ptr<ThreadPool> _threadpool;
  static int _numThreads;
  // mutex to implement sync calls
  std::mutex _mutex;
  static bool init_threadpool();

 public:
  ConcurrentExecutorVariable();

  static void set_threadpool_threads(int threadCount);

  int get_dataType_enum() const override { return DATATYPE::CONCURRENT_EXECUTOR; }

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  std::string print() override { return fallback_print(); }

  nlohmann::json to_json() const override { return "[ConcurrentExecutorVariable]"; }

  OpReturnType run_sync(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType run_parallel(const std::vector<OpReturnType>& arguments, CallStack& stack);

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;

  static void reset_threadpool() { _threadpool.reset(); }
};
