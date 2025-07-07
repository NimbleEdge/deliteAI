# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

test_event = "test_event"
eventSchema = {
    "companyId": "string",
    "departmentId": "int32",
    "departmentRevenue": "float",
    "pincodes": "int64[]"
}
eventExpiryCount = 5
rawStore = nm.RawEventStore(test_event, "count", eventExpiryCount)
eventDf = nm.Dataframe(eventSchema)

@concurrent
@pre_add_event([test_event])
def pre_add_event_func(type, event):
    print(type, event)
    dep0 = event["departments"][0]
    office_pincodes = []
    for pincode in event["officePincodes"]:
        office_pincodes.append(pincode)
    updated_event = {
        "companyId": event["companyId"],
        "departmentId": dep0["departmentId"],
        "departmentRevenue": dep0["revenue"],
        "pincodes": office_pincodes
    }
    print(updated_event)
    return type, updated_event

@add_event(rawStore)
def add_event_func(type, event):
    eventDf.append(event)

# Processor to create a departmentRevenue sum grouping by departmentIds
companyProcessor = eventDf.processor("float").rollingWindow([60*60]).groupBy(["departmentId"]).add_computation("departmentRevenue", "Sum", 0.0).create()

def run_method(input):
    companyProcessorOutput = companyProcessor.get([0])

    eventStore = eventDf.filter_all()
    officePincodes = eventStore.fetch("pincodes", "int64[]")

    # TODO: Should tensor inside a list be allowed in output
    return {"companyProcessorOutput": companyProcessorOutput, "officePincodes": nm.tensor(officePincodes, "int64")}
