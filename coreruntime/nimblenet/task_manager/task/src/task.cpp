#include "task.hpp"

#include <memory>

#include "data_variable.hpp"
#include "future_data_variable.hpp"
#include "statements.hpp"
#include "variable_scope.hpp"

#ifdef GENAI
#include "char_stream.hpp"
#endif  // GENAI

using json = nlohmann::json;
using namespace std;

Task::Task(const std::string& version, const std::string& astString, CommandCenter* commandCenter)
    : Task(version, json::parse(astString), commandCenter) {}

Task::~Task() {
#ifdef GENAI
  {
    // new scope to release lock
    // should always take lock when modifying variables used in condition variable predicates to
    // avoid race

    auto streamPushLock = get_stream_push_lock();
    _threadCleanupInitiated = true;
    // so thread reaches cleanup
  }
  _streamPushThreadCondition.notify_one();
  _streamPushThread.join();
#endif
}

Task::Task(const std::string& version, json&& astJson, CommandCenter* commandCenter)
    : _callStack(commandCenter) {
  _version = version;
  _commandCenter = commandCenter;
  _astJson = std::move(astJson);

#ifdef GENAI
  _streamPushThread = std::thread(&Task::run_background_jobs_on_new_thread, this);

  // log_script_acu_metric(taskAsset, commandCenter);
  _commandCenter->get_userEventsManager().script_loaded_trigger();
#endif
}

static inline void log_script_acu_metric(std::shared_ptr<Asset> scriptAsset,
                                         CommandCenter* commandCenter) {
  nlohmann::json acumetric;
  acumetric["Id"] = scriptAsset->name;
  acumetric["version"] = scriptAsset->version;
  acumetric["type"] = SCRIPTTYPE;
  acumetric["deploymentId"] = commandCenter->get_deployment_id();
  commandCenter->get_metrics_agent().log_metrics(ACUMETRIC, acumetric);
}

Task::Task(CommandCenter* commandCenter, std::shared_ptr<Asset> taskAsset)
    : _callStack(commandCenter) {
  auto [readSuccess, task] =
      nativeinterface::read_potentially_compressed_file(taskAsset->locationOnDisk.path);
  if (!readSuccess) {
    LOG_TO_CLIENT_ERROR("%s", "Script could not be read from file.");
    return;
  }

  _version = taskAsset->version;
  _commandCenter = commandCenter;
  _astJson = nlohmann::json::parse(task);

#ifdef GENAI
  _streamPushThread = std::thread(&Task::run_background_jobs_on_new_thread, this);
#endif

  log_script_acu_metric(taskAsset, commandCenter);
  _commandCenter->get_userEventsManager().script_loaded_trigger();
}

void Task::operate(const std::string& functionName, const MapVariablePtr inputs,
                   MapVariablePtr outputs) {
  _mainModule->operate(functionName, inputs, outputs, _callStack);
}

void Task::parse_main_module() {
  if (_mainModule) return;
  json mainAst = _astJson;
  if (_astJson.contains(MAIN_MODULE)) {
    mainAst = _astJson.at(MAIN_MODULE);
  }
  _mainModule = std::make_unique<DpModule>(_commandCenter, MAIN_MODULE, 0, mainAst, _callStack);
  LOG_TO_CLIENT_INFO("Script Loaded with version=%s", _version.c_str());
}

bool Task::has_module(const std::string& module) const {
  return _modules.find(module) != _modules.end() ||
         (_astJson.is_object() && _astJson.contains(module));
}

std::shared_ptr<DpModule> Task::get_module(const std::string& name, CallStack& stack) {
  if (_modules.find(name) != _modules.end()) {
    return _modules.at(name);
  }
  auto module = std::make_shared<DpModule>(_commandCenter, name, _modules.size() + 1,
                                           _astJson.at(name), stack);
  _modules[name] = module;
  return module;
}

#ifdef GENAI
void Task::run_background_jobs_until_condition(std::function<bool()>&& condition,
                                               std::unique_lock<std::mutex>& streamPushLock) {
  // Control should come here only after taking the stream push lock.
  assert(streamPushLock.owns_lock());

  while (!condition()) {
    {
      if (_streamPushJob) {
        // Explicitly ignoring return condition of this job run
        static_cast<void>(_streamPushJob->process_base_job());
      } else {
        // Throwing as condition will never be true, unless we are running a job here.
        THROW("%s", "No background jobs running to process to complete function");
      }
    }
    // for now leaving lock once in a while as maybe someone elses condition becomes true.
    // TODO: Do this later with condition variable syntax.
    streamPushLock.unlock();
    usleep(1);
    streamPushLock.lock();
  }
}

void Task::wait_until_stream_push_job_is_created() {
  auto lock = get_stream_push_lock();
  // wait until a streamPushJob is created or _thread needs to be stopped
  _streamPushThreadCondition.wait(
      lock, [&] { return _streamPushJob != nullptr || _threadCleanupInitiated; });
}

void Task::run_background_jobs_on_new_thread() {
  // This runs on a separate thread.
  while (true) {
    wait_until_stream_push_job_is_created();
    if (_threadCleanupInitiated) {
      break;
    }
    {
      // new scope so that destructor of lock is called
      auto lock = get_stream_push_lock();
      if (_streamPushJob) {
        auto status = _streamPushJob->process_base_job();
        if (status == BaseJob::Status::COMPLETE) {
          // remove job from thread.
          _streamPushJob = nullptr;
          // ideally should notify conditionVariable here, not notifying right now as this is the
          // only thread that waits
        }
      }
    }
    // This sleep is required so that someone waiting gets priority for acquiring lock
    usleep(1);
  }
}

BaseJob::Status FillCharStreamJob::process() {
  auto charStream = _charStream.lock();
  if (!charStream || charStream->closed()) return BaseJob::Status::COMPLETE;
  int n = _internalQueue->size();
  // Running till atmost internalQueue size, as LLM thread is continuously running and we would want
  // other functions to consume the char stream, before the entire output is present
  while (n-- && !_internalQueue->empty()) {
    char c = *_internalQueue->front();
    _internalQueue->pop();

    if (c == '\0') {
      // producer thread is finished producing
      charStream->close();
      return BaseJob::Status::COMPLETE;
    }

    charStream->push(c);
  }
  // We have parsed the output that LLM thread has streamed till now.
  // Next parts of the stream will be consumed in the next try
  return BaseJob::Status::RETRY;
}

void Task::add_stream_push_job(std::shared_ptr<BaseJob> job) {
  auto streamPushLock = get_stream_push_lock();
  _streamPushJob = job;
  // using one or all is the same, as only one thread is waiting on this condition variable
  _streamPushThreadCondition.notify_one();
}
#endif  // GENAI

void Task::save_future(std::shared_ptr<FutureDataVariable> futureVal) {
  _commandCenter->update_dependency_of_script_ready_job(futureVal->get_job());
  _pendingFutures.push_back(std::weak_ptr<FutureDataVariable>(futureVal));
}

bool Task::is_ready() noexcept {
  try {
    while (!_pendingFutures.empty()) {
      auto futureWeakVal = _pendingFutures.back();
      auto futureVal = futureWeakVal.lock();
      if (futureVal && !futureVal->is_available()) {
        return false;
      }
      _pendingFutures.pop_back();
    }
    return true;
  } catch (const std::exception& e) {
    LOG_TO_CLIENT_ERROR("Error while trying to get future value with error: %s", e.what());
    return false;
  } catch (...) {
    LOG_TO_CLIENT_ERROR("Unknown error while trying to get future value");
    return false;
  }
}
