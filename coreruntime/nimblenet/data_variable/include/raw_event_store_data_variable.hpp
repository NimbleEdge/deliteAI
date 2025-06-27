/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "command_center.hpp"
#include "data_variable.hpp"
#include "user_events_manager.hpp"

/**
 * @brief Data variable for managing raw event stores in the NimbleNet system
 *
 * RawEventStoreDataVariable provides a data variable interface for raw event stores,
 * which are used to collect and manage events of a specific type with configurable
 * expiry policies. This class acts as a wrapper around RawStore instances, providing
 * the standard DataVariable interface while delegating actual event storage to the
 * underlying store.
 */
class RawEventStoreDataVariable final : public DataVariable {
  std::string _eventType; /**< The type of events this store manages */
  std::shared_ptr<RawStore> _rawStore = nullptr; /**< The underlying raw event store instance */

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::RAW_EVENTS_STORE; }

  nlohmann::json to_json() const override { return "[RawEventsStore]"; }

 public:
  /**
   * @brief Constructs a new raw event store data variable
   * @param commandCenter Pointer to the command center for accessing user events manager
   * @param eventType The type of events this store will manage
   * @param expiryType The type of expiry policy (e.g., "time", "count")
   * @param expiryValue The value for the expiry policy
   */
  RawEventStoreDataVariable(CommandCenter* commandCenter, const std::string& eventType,
                            const std::string& expiryType, int expiryValue);

  /**
   * @brief Sets a hook function to be called when events are added to the store
   * @param functionDataVariable The function data variable to be called on event addition
   */
  void set_add_event_hook(OpReturnType functionDataVariable) {
    _rawStore->set_add_event_hook(functionDataVariable);
  }

  std::string print() override { return fallback_print(); }
};
