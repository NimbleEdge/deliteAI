/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "rolling_window.hpp"

using namespace std;

bool RollingWindow::create_aggregate_columns_for_group(const std::string& group,
                                                       const std::vector<int>& columnIds,
                                                       std::vector<T>& totalFeatureVector,
                                                       int rollingWindowFeatureStartIndex) {
  _groupWiseAggregatedColumnMap[group] = std::vector<AggregateColumn*>();
  for (int i = 0; i < _preprocessorInfo.columnsToAggregate.size(); i++) {
    if (_preprocessorInfo.aggregateOperators[i] == "Sum")
      _groupWiseAggregatedColumnMap[group].push_back(
          new SumColumn(_preprocessorId, columnIds[i], group,
                        &totalFeatureVector[rollingWindowFeatureStartIndex + i]));
    else if (_preprocessorInfo.aggregateOperators[i] == "Count")
      _groupWiseAggregatedColumnMap[group].push_back(
          new CountColumn(_preprocessorId, columnIds[i], group,
                          &totalFeatureVector[rollingWindowFeatureStartIndex + i]));
    else if (_preprocessorInfo.aggregateOperators[i] == "Min")
      _groupWiseAggregatedColumnMap[group].push_back(
          new MinColumn(_preprocessorId, columnIds[i], group,
                        &totalFeatureVector[rollingWindowFeatureStartIndex + i]));
    else if (_preprocessorInfo.aggregateOperators[i] == "Max")
      _groupWiseAggregatedColumnMap[group].push_back(
          new MaxColumn(_preprocessorId, columnIds[i], group,
                        &totalFeatureVector[rollingWindowFeatureStartIndex + i]));
    else if (_preprocessorInfo.aggregateOperators[i] == "Avg")
      _groupWiseAggregatedColumnMap[group].push_back(
          new AverageColumn(_preprocessorId, columnIds[i], group,
                            &totalFeatureVector[rollingWindowFeatureStartIndex + i]));
    else {
      LOG_TO_CLIENT_ERROR(
          "%s",
          "No Operators found for the following column, Operators can be Count, Min, Max, "
          "Sum, Avg");
      return false;
    }
  }
  return true;
}
