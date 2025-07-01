/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "raw_event_store_data_variable.hpp"

RawEventStoreDataVariable::RawEventStoreDataVariable(CommandCenter* commandCenter,
                                                     const std::string& eventType,
                                                     const std::string& expiryType,
                                                     int expiryValue) {
  _eventType = eventType;
  _rawStore =
      commandCenter->get_userEventsManager().create_raw_store(eventType, expiryType, expiryValue);
}
