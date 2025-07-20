/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tokenizers_data_variable.hpp"
#include "data_variable_enums.hpp"
#include "util.hpp"
#include "native_interface.hpp"
#include "tensor_data_variable.hpp"
#include "single_variable.hpp"
#include "map_data_variable.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp> // Added for nlohmann/json

TokenizersDataVariable::TokenizersDataVariable() : _tokenizer(nullptr) {}

TokenizersDataVariable::TokenizersDataVariable(std::unique_ptr<tokenizers::Tokenizer> tokenizer)
    : _tokenizer(std::move(tokenizer)) {}

std::unique_ptr<tokenizers::Tokenizer> TokenizersDataVariable::_create_tokenizer_from_file(const std::string& path) {
  LOG_TO_CLIENT_DEBUG("TokenizersDataVariable::_create_tokenizer_from_file: %s", path.c_str());
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    THROW("Failed to open tokenizer file: %s", path.c_str());
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  // Try to determine the type by file extension
  if (path.size() >= 5 && path.substr(path.size() - 5) == ".json") {
    return tokenizers::Tokenizer::FromBlobJSON(content);
  } else if (path.size() >= 6 && path.substr(path.size() - 6) == ".model") {
    return tokenizers::Tokenizer::FromBlobSentencePiece(content);
  } else {
    // Default to JSON format
    return tokenizers::Tokenizer::FromBlobJSON(content);
  }
}

std::unique_ptr<tokenizers::Tokenizer> TokenizersDataVariable::_create_tokenizer_from_json(const std::string& json) {
  return tokenizers::Tokenizer::FromBlobJSON(json);
}

std::unique_ptr<tokenizers::Tokenizer> TokenizersDataVariable::_create_tokenizer_from_map(const MapDataVariable* map) {
  // Convert MapDataVariable to nlohmann::json, then to string
  nlohmann::json json_obj = map->to_json();
  std::string json_str = json_obj.dump();
  return tokenizers::Tokenizer::FromBlobJSON(json_str);
}

std::unique_ptr<tokenizers::Tokenizer> TokenizersDataVariable::_create_tokenizer_from_sentencepiece(const std::string& model_path) {
  std::ifstream file(model_path, std::ios::binary);
  if (!file.is_open()) {
    THROW("Failed to open SentencePiece model file: %s", model_path.c_str());
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return tokenizers::Tokenizer::FromBlobSentencePiece(content);
}

OpReturnType TokenizersDataVariable::from_pretrained(const std::vector<OpReturnType>& arguments,
                                                    CallStack& stack) {
  if (arguments.size() != 1) {
    THROW("from_pretrained expects 1 argument, got %zu", arguments.size());
  }

  auto path_var = arguments[0];
  if (path_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("from_pretrained expects string argument");
  }

  std::string path = static_cast<SingleVariable<std::string>*>(path_var.get())->get_string();
  LOG_TO_CLIENT_DEBUG("TokenizersDataVariable::from_pretrained: %s", path.c_str());
  try {
    auto tokenizer = _create_tokenizer_from_file(path);
    return OpReturnType(new TokenizersDataVariable(std::move(tokenizer)));
  } catch (const std::exception& e) {
    THROW("Failed to create tokenizer from %s: %s", path.c_str(), e.what());
  }
}

OpReturnType TokenizersDataVariable::from_file(const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  if (arguments.size() != 1) {
    THROW("from_file expects 1 argument, got %zu", arguments.size());
  }

  auto path_var = arguments[0];
  if (path_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("from_file expects string argument");
  }

  std::string path = static_cast<SingleVariable<std::string>*>(path_var.get())->get_string();

  try {
    auto tokenizer = _create_tokenizer_from_file(path);
    return OpReturnType(new TokenizersDataVariable(std::move(tokenizer)));
  } catch (const std::exception& e) {
    THROW("Failed to create tokenizer from %s: %s", path.c_str(), e.what());
  }
}

OpReturnType TokenizersDataVariable::from_json(const std::vector<OpReturnType>& arguments,
                                              CallStack& stack) {
  if (arguments.size() != 1) {
    THROW("from_json expects 1 argument, got %zu", arguments.size());
  }

  auto json_var = arguments[0];

  try {
    std::unique_ptr<tokenizers::Tokenizer> tokenizer;

    if (json_var->get_containerType() == CONTAINERTYPE::MAP) {
      // Handle MapDataVariable input
      auto map_var = static_cast<MapDataVariable*>(json_var.get());
      tokenizer = _create_tokenizer_from_map(map_var);
    } else if (json_var->get_containerType() == CONTAINERTYPE::SINGLE) {
      // Handle string input (backward compatibility)
      std::string json = static_cast<SingleVariable<std::string>*>(json_var.get())->get_string();
      tokenizer = _create_tokenizer_from_json(json);
    } else {
      THROW("from_json expects either a dictionary (MapDataVariable) or string argument");
    }

    return OpReturnType(new TokenizersDataVariable(std::move(tokenizer)));
  } catch (const std::exception& e) {
    THROW("Failed to create tokenizer from JSON: %s", e.what());
  }
}

OpReturnType TokenizersDataVariable::from_sentencepiece(const std::vector<OpReturnType>& arguments,
                                                       CallStack& stack) {
  if (arguments.size() != 1) {
    THROW("from_sentencepiece expects 1 argument, got %zu", arguments.size());
  }

  auto path_var = arguments[0];
  if (path_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("from_sentencepiece expects string argument");
  }

  std::string path = static_cast<SingleVariable<std::string>*>(path_var.get())->get_string();

  try {
    auto tokenizer = _create_tokenizer_from_sentencepiece(path);
    return OpReturnType(new TokenizersDataVariable(std::move(tokenizer)));
  } catch (const std::exception& e) {
    THROW("Failed to create SentencePiece tokenizer from %s: %s", path.c_str(), e.what());
  }
}

OpReturnType TokenizersDataVariable::encode(const std::vector<OpReturnType>& arguments,
                                           CallStack& stack) {
  if (!_tokenizer) {
    THROW("No tokenizer loaded. Use from_pretrained, from_file, from_json, or from_sentencepiece first.");
  }

  if (arguments.size() != 1) {
    THROW("encode expects 1 argument (text), got %zu", arguments.size());
  }

  auto text_var = arguments[0];
  if (text_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("encode expects string argument");
  }

  std::string text = static_cast<SingleVariable<std::string>*>(text_var.get())->get_string();

  try {
    std::vector<int32_t> token_ids = _tokenizer->Encode(text);

    // Create a tensor to return the token IDs
    std::vector<int64_t> shape = {static_cast<int64_t>(token_ids.size())};
    auto result_tensor = TensorVariable::copy_tensor_from_raw_data(token_ids.data(), DATATYPE::INT32, shape);
    return result_tensor;
  } catch (const std::exception& e) {
    THROW("Failed to encode text: %s", e.what());
  }
}

OpReturnType TokenizersDataVariable::decode(const std::vector<OpReturnType>& arguments,
                                           CallStack& stack) {
  if (!_tokenizer) {
    THROW("No tokenizer loaded. Use from_pretrained, from_file, from_json, or from_sentencepiece first.");
  }

  if (arguments.size() != 1) {
    THROW("decode expects 1 argument (token_ids), got %zu", arguments.size());
  }

  auto ids_var = arguments[0];

  auto tensor = std::dynamic_pointer_cast<BaseTypedTensorVariable>(ids_var);
  if (tensor) {
    if (ids_var->get_dataType_enum() != DATATYPE::INT32) {
      THROW("decode expects INT32 tensor for ids");
    }

    // Use begin/end iterators to get data
    std::vector<int32_t> token_ids(tensor->begin<int32_t>(), tensor->end<int32_t>());

    try {
      std::string decoded = _tokenizer->Decode(token_ids);
      auto result = std::make_shared<SingleVariable<std::string>>(decoded);
      return OpReturnType(result);
    } catch (const std::exception& e) {
      THROW("Failed to decode token IDs: %s", e.what());
    }
  } else {
    THROW("decode expects tensor of token IDs");
  }
}

OpReturnType TokenizersDataVariable::get_vocab_size(const std::vector<OpReturnType>& arguments,
                                                   CallStack& stack) {
  if (!_tokenizer) {
    THROW("No tokenizer loaded. Use from_pretrained, from_file, from_json, or from_sentencepiece first.");
  }

  if (arguments.size() != 0) {
    THROW("get_vocab_size expects 0 arguments, got %zu", arguments.size());
  }

  try {
    int64_t vocab_size = static_cast<int64_t>(_tokenizer->GetVocabSize());
    auto result = std::make_shared<SingleVariable<int64_t>>(vocab_size);
    return OpReturnType(result);
  } catch (const std::exception& e) {
    THROW("Failed to get vocab size: %s", e.what());
  }
}

OpReturnType TokenizersDataVariable::token_to_id(const std::vector<OpReturnType>& arguments,
                                                 CallStack& stack) {
  if (!_tokenizer) {
    THROW("No tokenizer loaded. Use from_pretrained, from_file, from_json, or from_sentencepiece first.");
  }

  if (arguments.size() != 1) {
    THROW("token_to_id expects 1 argument (token), got %zu", arguments.size());
  }

  auto token_var = arguments[0];
  if (token_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("token_to_id expects string argument");
  }

  std::string token = static_cast<SingleVariable<std::string>*>(token_var.get())->get_string();

  try {
    int32_t token_id = _tokenizer->TokenToId(token);
    auto result = std::make_shared<SingleVariable<int32_t>>(token_id);
    return OpReturnType(result);
  } catch (const std::exception& e) {
    THROW("Failed to get token ID: %s", e.what());
  }
}

OpReturnType TokenizersDataVariable::id_to_token(const std::vector<OpReturnType>& arguments,
                                                 CallStack& stack) {
  if (!_tokenizer) {
    THROW("No tokenizer loaded. Use from_pretrained, from_file, from_json, or from_sentencepiece first.");
  }

  if (arguments.size() != 1) {
    THROW("id_to_token expects 1 argument (token_id), got %zu", arguments.size());
  }

  auto id_var = arguments[0];
  if (id_var->get_containerType() != CONTAINERTYPE::SINGLE) {
    THROW("id_to_token expects int argument");
  }

  int32_t token_id = static_cast<int32_t>(static_cast<SingleVariable<int32_t>*>(id_var.get())->get_int64());

  try {
    std::string token = _tokenizer->IdToToken(token_id);
    auto result = std::make_shared<SingleVariable<std::string>>(token);
    return OpReturnType(result);
  } catch (const std::exception& e) {
    THROW("Failed to get token: %s", e.what());
  }
}

OpReturnType TokenizersDataVariable::call_function(int memberFuncIndex,
                                                   const std::vector<OpReturnType>& arguments,
                                                   CallStack& stack) {
  switch (memberFuncIndex) {
    case MemberFuncType::TOKENIZERS_FROM_PRETRAINED:
      return from_pretrained(arguments, stack);
    case MemberFuncType::TOKENIZERS_FROM_FILE:
      return from_file(arguments, stack);
    case MemberFuncType::TOKENIZERS_FROM_JSON:
      return from_json(arguments, stack);
    case MemberFuncType::TOKENIZERS_FROM_SENTENCEPIECE:
      return from_sentencepiece(arguments, stack);
    case MemberFuncType::TOKENIZERS_ENCODE:
      return encode(arguments, stack);
    case MemberFuncType::TOKENIZERS_DECODE:
      return decode(arguments, stack);
    case MemberFuncType::TOKENIZERS_GET_VOCAB_SIZE:
      return get_vocab_size(arguments, stack);
    case MemberFuncType::TOKENIZERS_TOKEN_TO_ID:
      return token_to_id(arguments, stack);
    case MemberFuncType::TOKENIZERS_ID_TO_TOKEN:
      return id_to_token(arguments, stack);
    default:
      THROW("%s not implemented for tokenizers", DataVariable::get_member_func_string(memberFuncIndex));
  }
}
