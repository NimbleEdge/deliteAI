#include "llm_data_variable.hpp"

#include <memory>

#include "asset_load_job.hpp"
#include "char_stream.hpp"
#include "command_center.hpp"
#include "core_sdk_structs.hpp"
#include "data_variable.hpp"
#ifdef GEMINI
#include "gemini_nano_executor.hpp"
#endif  // GEMINI
#include "native_interface.hpp"
#ifdef ONNXGENAI_EXECUTOR
#include "onnx_llm_executor.hpp"
#endif  // ONNXGENAI_EXECUTOR
#ifdef EXECUTORCH_EXECUTOR
#include "executorch_llm_executor.hpp"
#endif  // EXECUTORCH_EXECUTOR
#include "llm_utils.hpp"
#include "nimble_net_util.hpp"
#include "single_variable.hpp"
#include "stream_data_variable.hpp"
#include "task.hpp"

LLMDataVariable::LLMDataVariable(const std::shared_ptr<Asset> llmAsset,
                                 CommandCenter* commandCenter) {
  if (llmAsset->osProvided) {
#ifdef GEMINI
    if (llmAsset->name == rmconstants::GeminiModelName) {
      _llmExecutor = std::make_unique<GeminiNanoExecutor>(commandCenter->get_task(), commandCenter);
      return;
    }
#endif  // GEMINI
    THROW("No executor apart from GEMINI supported for os provided LLM");
  } else {
    const auto completeLlmPath =
        nativeinterface::get_full_file_path_common(llmAsset->get_file_name_on_device());
#ifdef ONNXGENAI_EXECUTOR
    _llmExecutor = std::make_unique<ONNXLLMExecutor>(completeLlmPath, commandCenter->get_task(),
                                                     commandCenter);
    return;
#endif  // ONNXGENAI_EXECUTOR
#ifdef EXECUTORCH_EXECUTOR
    std::string endOfTurnToken = "";
    int maxTokensToGenerate = 2000;
    float temperature = 0.8;
    std::string tokenizerFileName = "";
    if (!llmAsset->metadataFromScript.empty() &&
        llmAsset->metadataFromScript.contains("endOfTurnToken") &&
        llmAsset->metadataFromScript.contains("tokenizerFileName")) {
      endOfTurnToken = llmAsset->metadataFromScript.at("endOfTurnToken");
      tokenizerFileName = llmAsset->metadataFromScript.at("tokenizerFileName");
      if (llmAsset->metadataFromScript.contains("temperature")) {
        temperature = llmAsset->metadataFromScript.at("temperature");
      }
      if (llmAsset->metadataFromScript.contains("maxTokensToGenerate")) {
        maxTokensToGenerate = llmAsset->metadataFromScript.at("maxTokensToGenerate");
      }
    } else {
      THROW(
          "endOfTurnToken and tokenizerFileName should be present in metadata "
          "in nm.llm({'name': '...', 'metadata': {}}) metadata when loading LLM via executorch.");
    }
    _llmExecutor = std::make_unique<ExecutorchLLMExecutor>(
        completeLlmPath, commandCenter->get_task(), commandCenter, llmAsset->name, endOfTurnToken,
        maxTokensToGenerate, temperature, tokenizerFileName);
    return;
#endif  // EXECUTORCH_EXECUTOR
    THROW("No executor apart from onnx and executorch supported for custom LLM");
  }
}

OpReturnType LLMDataVariable::call_function(int memberFuncIndex,
                                            const std::vector<OpReturnType>& arguments,
                                            CallStack& stack) {
  switch (memberFuncIndex) {
    case PROMPT: {
      return std::make_shared<CharStreamIterDataVariable>(prompt(arguments, stack));
    }
    case ADD_CONTEXT:
      return add_context(arguments, stack);
    case MAX_INPUT_NUM_TOKENS:
      return std::make_shared<SingleVariable<int>>(_llmExecutor->max_input_num_tokens());
    case CANCEL:
      return cancel_generation(arguments, stack);
    case CLEAR_CONTEXT:
      return _llmExecutor->clear_context();
  }
  THROW("%s not implemented for llm", DataVariable::get_member_func_string(memberFuncIndex));
}

std::shared_ptr<CharStream> LLMDataVariable::prompt(const std::vector<OpReturnType>& arguments,
                                                    CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, PROMPT);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0, PROMPT);
  std::string prompt = arguments[0]->get_string();
  return _llmExecutor->run_prompt(std::move(prompt));
}

std::shared_ptr<FutureDataVariable> LLMDataVariable::load_async(
    const std::map<std::string, OpReturnType>& llmConfig, CommandCenter* commandCenter) {
  if (llmConfig.find("name") == llmConfig.end()) {
    THROW("llm requires a 'name' field");
  }
  std::string name = llmConfig.at("name")->get_string();

  std::string provider = llmConfig.find("provider") != llmConfig.end()
                             ? llmConfig.at("provider")->get_string()
                             : llmutil::Provider::CUSTOM;

  nlohmann::json assetMetadataFromScript = llmConfig.find("metadata") != llmConfig.end()
                                               ? llmConfig.at("metadata")->to_json()
                                               : nlohmann::json::object();

  std::shared_ptr<Asset> llmAsset;
  if (provider == llmutil::Provider::OS) {
    llmAsset = std::make_shared<Asset>();
    llmAsset->type = AssetType::LLM;
    llmAsset->name = name;
    llmAsset->osProvided = true;
    llmAsset->valid = true;
  } else {
    const auto& deployment = commandCenter->get_deployment();
    llmAsset = deployment.get_module(name, AssetType::LLM);
    if (llmAsset == nullptr) {
      THROW("LLM %s not present in deployment", name.c_str());
    }
  }
  // The deployment from cloud/disk won't have this field, so adding it here
  llmAsset->metadataFromScript = assetMetadataFromScript;
  auto llmLoadJob = std::make_shared<AssetLoadJob>(llmAsset, commandCenter);
  return llmLoadJob->init();
}

OpReturnType LLMDataVariable::cancel_generation(const std::vector<OpReturnType>& arguments,
                                                CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 0, CANCEL);
  _llmExecutor->cancel();
  return std::make_shared<NoneVariable>();
}

OpReturnType LLMDataVariable::add_context(const std::vector<OpReturnType>& arguments,
                                          CallStack& stack) {
  THROW_ARGUMENTS_NOT_MATCH(arguments.size(), 1, ADD_CONTEXT);
  THROW_ARGUMENT_DATATYPE_NOT_MATCH(arguments[0]->get_dataType_enum(), DATATYPE::STRING, 0, PROMPT);
  std::string prompt = arguments[0]->get_string();
  _llmExecutor->add_prompt(prompt);
  return std::make_shared<NoneVariable>();
}
