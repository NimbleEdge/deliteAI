#pragma once

#include "command_center.hpp"
#include "data_variable.hpp"
#include "user_events_manager.hpp"

class RawEventStoreDataVariable final : public DataVariable {
  std::string _eventType;
  std::shared_ptr<RawStore> _rawStore = nullptr;

  int get_containerType() const override { return CONTAINERTYPE::SINGLE; }

  bool get_bool() override { return true; }

  int get_dataType_enum() const override { return DATATYPE::RAW_EVENTS_STORE; }

  nlohmann::json to_json() const override { return "[RawEventsStore]"; }

 public:
  RawEventStoreDataVariable(CommandCenter* commandCenter, const std::string& eventType,
                            const std::string& expiryType, int expiryValue);

  void set_add_event_hook(OpReturnType functionDataVariable) {
    _rawStore->set_add_event_hook(functionDataVariable);
  }

  std::string print() override { return fallback_print(); }
};
