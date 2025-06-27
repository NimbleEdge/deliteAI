/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "data_variable.hpp"
#include "util.hpp"
#include "rolling_window.hpp"
#include "user_events_struct.hpp"

class BasePreProcessor {
 public:
  int _id;
  virtual void add_event(int newEventIndex) = 0;
  virtual std::string get_group_from_event(const TableEvent& e) = 0;
  virtual bool get_group_from_row(const std::vector<std::string>& row,
                                  const std::vector<bool>& columnsFilled,
                                  std::string& retGroup) = 0;
  virtual std::vector<std::string> get_groups_from_json(
      const nlohmann::json& preprocessorInput) = 0;
  virtual std::shared_ptr<ModelInput> get_model_input(const nlohmann::json& preprocessorInput) = 0;
  virtual OpReturnType get_model_input_data_variable(const std::vector<std::string>& groups) = 0;
  virtual OpReturnType get_model_input_data_variable(const nlohmann::json& json) = 0;
  virtual OpReturnType get_model_input_data_variable(
      const std::vector<std::vector<std::string>>& allGroups) = 0;
  virtual int get_num_of_groupBys() = 0;
  virtual std::string get_group_from_group_vector(const std::vector<std::string>& groupVec) = 0;

  BasePreProcessor(int id) { _id = id; }

  virtual ~BasePreProcessor() = default;
};

class PreProcessor : public BasePreProcessor {
  using T = double;

  std::vector<int> _groupIds;
  std::vector<int> _columnIds;
  bool _isUseless = false;
  PreProcessorInfo _info;
  std::vector<double> _defaultFeature;
  std::vector<RollingWindow*> _rollingWindows;
  std::map<std::string, std::vector<double>> _groupWiseFeatureMap;
  std::shared_ptr<TableData> _tableData = nullptr;

 public:
  int get_num_of_groupBys() override { return _groupIds.size(); }

  bool get_group_from_row(const std::vector<std::string>& row,
                          const std::vector<bool>& columnsFilled, std::string& retGroup) override;
  std::string get_group_from_group_vector(const std::vector<std::string>& groupVec) override;
  std::string get_group_from_event(const TableEvent& e) override;
  std::vector<std::string> get_groups_from_json(const nlohmann::json& preprocessorInput) override;
  std::shared_ptr<ModelInput> get_model_input(const nlohmann::json& preprocessorInput) override;
  OpReturnType get_model_input_data_variable(const std::vector<std::string>& groups) override;
  OpReturnType get_model_input_data_variable(const nlohmann::json& json) override;
  OpReturnType get_model_input_data_variable(
      const std::vector<std::vector<std::string>>& allGroups) override;
  PreProcessor(int id, const PreProcessorInfo& info, const std::vector<int>& groupIds,
               const std::vector<int>& columnIds, std::shared_ptr<TableData> tableData);
  void add_event(int newEventIndex) override;

  virtual ~PreProcessor() {
    for (auto rollWindow : _rollingWindows) {
      delete rollWindow;
    }
  }
};
