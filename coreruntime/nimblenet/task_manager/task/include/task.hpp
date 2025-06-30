#pragma once

#include <cstdlib>

// #include <shared_lock>
#include <condition_variable>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "asset_manager.hpp"
#include "dp_module.hpp"
#include "job.hpp"
#include "json.hpp"
#include "rigtorp/SPSCQueue.h"
#include "variable_scope.hpp"

class CharStream;
class CommandCenter;
class Operator;
class Body;
class VariableScope;
class MapDataVariable;
typedef std::shared_ptr<MapDataVariable> MapVariablePtr;
class FutureDataVariable;

class FillCharStreamJob : public Job<void> {
  using Queue = rigtorp::SPSCQueue<char>;
  std::weak_ptr<CharStream> _charStream;
  std::shared_ptr<Queue> _internalQueue;

 public:
  FillCharStreamJob(std::weak_ptr<CharStream> charStream, std::shared_ptr<Queue> queue)
      : Job("FillCharStreamJob") {
    _charStream = charStream;
    _internalQueue = queue;
  }

  Job::Status process() override;
};

class Task {
 public:
  constexpr static inline const char* EXIT_STATUS_KEY = "__NIMBLE_EXIT_STATUS";

  // NOTE: Due to presence of this, CharStreams cannot be created from concurrent functions
  std::vector<std::weak_ptr<CharStream>> _charStreams;

 private:
  static inline const std::string MAIN_MODULE = "main";

  CommandCenter* _commandCenter = nullptr;
  std::string _version;
  std::vector<std::weak_ptr<FutureDataVariable>> _pendingFutures;

  json _astJson;
  std::unique_ptr<DpModule> _mainModule;
  std::unordered_map<std::string, std::shared_ptr<DpModule>> _modules;

  std::shared_mutex _taskMutex;
#ifdef GENAI
  std::mutex _streamPushMutex;
  std::condition_variable _streamPushThreadCondition;
  std::thread _streamPushThread;
  std::atomic<bool> _threadCleanupInitiated = false;
  std::shared_ptr<BaseJob> _streamPushJob;
#endif
  CallStack _callStack;

 private:
#ifdef GENAI
  void run_background_jobs_on_new_thread();
#endif

 public:
  Task(const std::string& version, const std::string& ast, CommandCenter* commandCenter);
  Task(const std::string& version, json&& astJson, CommandCenter* commandCenter);
  Task(CommandCenter* commandCenter, std::shared_ptr<Asset> taskAsset);

  std::string get_version() const { return _version; }

  void operate(const std::string& functionName, MapVariablePtr inputs, MapVariablePtr outputs);

  bool get_shared_lock(std::shared_lock<std::shared_mutex>& lockMutex) {
    lockMutex = std::shared_lock<std::shared_mutex>(_taskMutex);
    return true;
  }

  void parse_main_module();

  bool has_module(const std::string& module) const;

  // caller must ensure that the module exists using has_module method
  std::shared_ptr<DpModule> get_module(const std::string& name, CallStack& stack);

  std::unique_lock<std::shared_mutex> get_script_deferred_lock() {
    return std::unique_lock<std::shared_mutex>(_taskMutex, std::defer_lock);
  }

  void save_future(std::shared_ptr<FutureDataVariable> futureVal);
  bool is_ready() noexcept;
  ~Task();

#ifdef GENAI
  std::unique_lock<std::mutex> get_stream_push_lock() {
    return std::unique_lock<std::mutex>(_streamPushMutex);
  }

  void add_stream_push_job(std::shared_ptr<BaseJob> job);

  void run_background_jobs_until_condition(std::function<bool()>&& condition,
                                           std::unique_lock<std::mutex>& streamPushLock);

  void add_char_stream(std::weak_ptr<CharStream> charStream);

  void wait_until_stream_push_job_is_created();
#endif  // GENAI
};
