/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "data_variable.hpp"
#include "frontend_data_variable.hpp"
#include "nimble_net_util.hpp"

class IOSReferenceDataVariable : public FrontendDataVariable {
  IosObject _iosObj;

  std::string common_print() const;

  std::string print() override { return common_print(); }

  nlohmann::json to_json() const override { return common_print(); }

  bool get_bool() override { return true; }

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

 public:
  IOSReferenceDataVariable(IosObject&& obj);
  ~IOSReferenceDataVariable();

  OpReturnType get_string_subscript(const std::string& key) override;
  OpReturnType get_int_subscript(int idx) override;
  void* get_raw_ptr() override;
  int get_size() override;
  void set_subscript(const OpReturnType& subscript, const OpReturnType& val) override;
  OpReturnType arrange(const OpReturnType argument) override;
  bool in(const OpReturnType& elem) override;
  OpReturnType call_function(int memberFuncIndex, const std::vector<OpReturnType>& arguments,
                             CallStack& stack) override;
};
