/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <map>
#include <string>
#include <vector>

#include "util.hpp"
#include "data_variable.hpp"

struct ModelInput {
  int length = 0;
  std::shared_ptr<void> data = nullptr;

  ModelInput(void* d, int l) : data(d, [](void* p) { operator delete[](p); }) { length = l; }
};

struct PreProcessorInfo {
  std::vector<float> rollingWindowsInSecs;
  std::vector<std::string> columnsToAggregate;
  std::vector<std::string> aggregateOperators;
  std::vector<std::string> groupColumns;
  std::vector<double> defaultVector;
  std::string tableName;
  DATATYPE dataType;
  bool valid = false;
};

void from_json(const json& j, PreProcessorInfo& preProcessorInfo);

struct TableInfo {
  bool valid = false;
  std::string name;
  std::map<std::string, int> schema;
  int64_t expiryTimeInMins;
};

struct TableEvent {
  std::vector<std::string> groups;
  int64_t timestamp;
  std::vector<OpReturnType> row;
};

struct TableRow {
  int64_t timestamp;
  std::map<std::string, OpReturnType> row;
};

struct TableData {
  std::vector<TableEvent> allEvents;
  std::map<std::string, int> columnToIdMap;
  std::vector<std::string> columns;
  std::map<std::string, int> schema;
};
