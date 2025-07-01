/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "raw_store.hpp"

#include <mutex>

#include "statements.hpp"

void RawStore::set_add_event_hook(OpReturnType functionDataVariable) {
  if (_eventHookSet) {
    THROW("add_event hook for eventType=%s is already set, cannot set again", _eventType.c_str());
  }
  _eventHookSet = true;
  _functionDataVariable = functionDataVariable;
  auto events = _database->get_events_from_db(_eventType);
  for (auto& event : events) {
    _functionDataVariable->execute_function(
        {OpReturnType(new SingleVariable<std::string>(_eventType)),
         OpReturnType(DataVariable::get_map_from_json_object(std::move(event)))});
  }
}

bool RawStore::add_event(OpReturnType eventMapTable) {
  std::map<std::string, OpReturnType> map = eventMapTable->get_map();
  if (_tableStore) {
    // case where schema from frontend
    TableRow r;
    for (const auto& [key, value] : map) {
      r.row[key] = value;
    }
    r.timestamp = Time::get_time();
    _tableStore->add_row(r);
  } else {
    // case where schema less RawStore created in script
    map[usereventconstants::TimestampField] =
        OpReturnType(new SingleVariable<int64_t>(Time::get_time()));
    if (_functionDataVariable)
      _functionDataVariable->execute_function(
          {OpReturnType(new SingleVariable<std::string>(_eventType)),
           OpReturnType(new MapDataVariable(std::move(map)))});
  }
  return true;
}
