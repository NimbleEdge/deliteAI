#include "raw_event_store_data_variable.hpp"

RawEventStoreDataVariable::RawEventStoreDataVariable(CommandCenter* commandCenter,
                                                     const std::string& eventType,
                                                     const std::string& expiryType,
                                                     int expiryValue) {
  _eventType = eventType;
  _rawStore =
      commandCenter->get_userEventsManager().create_raw_store(eventType, expiryType, expiryValue);
}
