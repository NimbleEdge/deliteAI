/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "map_data_variable.hpp"
#include "tokenizers_cpp.h"
#include <memory>
#include <string>
#include <vector>

class TokenizersDataVariable : public DataVariable {
 public:
  TokenizersDataVariable();
  TokenizersDataVariable(std::unique_ptr<tokenizers::Tokenizer> tokenizer);
  ~TokenizersDataVariable() override = default;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }
  bool get_bool() override { return true; }
  int get_dataType_enum() const override { return DATATYPE::NIMBLENET_TOKENIZERS; }
  nlohmann::json to_json() const override { return "[Tokenizers]"; }
  std::string print() override { return "[Tokenizers]"; }

  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments, CallStack& stack) override;

  // Static factory methods for creating tokenizer instances
  static OpReturnType from_pretrained(const std::vector<OpReturnType>& arguments, CallStack& stack);
  static OpReturnType from_file(const std::vector<OpReturnType>& arguments, CallStack& stack);
  static OpReturnType from_json(const std::vector<OpReturnType>& arguments, CallStack& stack);
  static OpReturnType from_sentencepiece(const std::vector<OpReturnType>& arguments, CallStack& stack);

  // Instance member methods that operate on the tokenizer
  OpReturnType encode(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType decode(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType get_vocab_size(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType token_to_id(const std::vector<OpReturnType>& arguments, CallStack& stack);
  OpReturnType id_to_token(const std::vector<OpReturnType>& arguments, CallStack& stack);

 private:
  std::unique_ptr<tokenizers::Tokenizer> _tokenizer;

  // Helper functions
  static std::unique_ptr<tokenizers::Tokenizer> _create_tokenizer_from_file(const std::string& path);
  static std::unique_ptr<tokenizers::Tokenizer> _create_tokenizer_from_json(const std::string& json);
  static std::unique_ptr<tokenizers::Tokenizer> _create_tokenizer_from_map(const MapDataVariable* map);
  static std::unique_ptr<tokenizers::Tokenizer> _create_tokenizer_from_sentencepiece(const std::string& model_path);
};
