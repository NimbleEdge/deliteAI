# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

eventType1 = "eventType1"
eventType2 = "eventType2"
eventType3 = "eventType3"
eventType4 = "eventType4"
updatedEventType = "updatedEventType"
eventSchema = {
    "id": "int32",
    "floatData": "float",
    "stringData": "string"
}
eventExpiryCount = 5
rawStore = nm.RawEventStore(eventType1, "count", eventExpiryCount)
updatedEventTypeRawStore = nm.RawEventStore(updatedEventType, "count", eventExpiryCount)

eventDf = nm.Dataframe(eventSchema)

@concurrent
@pre_add_event([eventType1, eventType2, eventType4])
def pre_add_event_func(type, event):
    print(type, event)
    if type == eventType2:
        print("Updating event type of ", type, " to ", updatedEventType)
        return updatedEventType, event
    if type == eventType4:
        print("Returning None for type", type)
        return None
    return type, event

@add_event(rawStore, updatedEventTypeRawStore)
def add_event_func(type, event):
    print("add_event for type: ", type)
    nm.log("event", {"id": event["id"], "floatData": event["floatData"], "stringData": event["stringData"]})
    if type != updatedEventType:
        eventDf.append(event)
