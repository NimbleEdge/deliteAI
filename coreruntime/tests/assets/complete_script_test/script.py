# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

bundleTableName = "ContestBundles"
bundleTableSchema = {
    "bundleId": "int32",
    "promotion_code": "int32",
    "productIds": "int32[]",
    "bundleStrings": "string[]",
    "bundleFloats": "float[]",
    "setStrings": "string[]",
    "setInts": "int32[]"
}
bundleTableExpiryTime = 1
rawStore = nm.RawEventStore(bundleTableName,"time",bundleTableExpiryTime)
df= nm.Dataframe(bundleTableSchema)

@concurrent
@pre_add_event(["ContestBundles"])
def pre_Add_event(type, event):
    return type, event

@add_event(rawStore)
def add_event_hook(type,bundle):
    for item in bundle["bundleStrings"]:
        if not item.is_string():
            return

    # Modify string array
    tensor = bundle["bundleStrings"]
    tensor[0] = "modifiedString"
    bundle["bundleStrings"] = tensor
    for item in bundle["bundleFloats"]:
        if not item.is_float() and not item.is_integer():
            return
    # Modify float array
    tensor = bundle["bundleFloats"]
    tensor[0] = 99.0
    bundle["bundleFloats"] = tensor
    # Create a tensor from list for int and string and assing to event
    tempIntList = [1, 2]
    tempStringList = ["a"]
    bundle["setInts"] = nm.tensor(tempIntList,"int32")
    bundle["setStrings"] = nm.tensor(tempStringList,"string")
    df.append(bundle)

# Event Store Filter function
def get_filtered_event_store(event):
    if event["bundleId"].is_integer():
        if event["bundleId"] == 1:
            return True
    return False

def get_filtered_event_store2(event):
    if event["bundleId"].is_integer():
        if event["bundleId"] == 2:
            return True
    return False

#################### Event Processor
bundleIdProcessor = df.processor("int32").rollingWindow([10]).groupBy(["bundleId"]).add_computation("bundleId", "Count", 0).create()
bundleIdPromotionCodeProcessor = df.processor("int32").rollingWindow([10]).groupBy(["bundleId", "promotion_code"]).add_computation("bundleId", "Count", 0).create()

def main(inputs):
    print(inputs["emptyJson"])
    print(inputs["emptyJsonArray"])

    #################### Nested Json objects/array
    # { "data" : [ 1, 2, 3 ], "data2" : [ 1, 2.08, 3 ], "data3" : "abcd", "nullValue": null }
    print("Found json object")
    print(inputs["nestedJsonObject"])
    json_object = inputs["nestedJsonObject"]
    for k in json_object.keys():
        print(k, json_object[k])

    # Even after changing json_object, the below three variables won't change
    json_object_data = json_object["data"]
    json_object_data2 = json_object["data2"]
    json_object_data3 = json_object["data3"]
    a = [3, 4, 5]
    json_object["data4"] = a
    json_object["data"] = "changedToString"
    print(json_object)
    print(a)
    print(json_object_data, json_object_data2, json_object_data3)

    # [ {"data" : [ 1, 2, 3 ]}, {"data2" : [ 1.90, 2.123, 3.45 ], "data3" : "stringValue"}, null ]
    print(inputs["nestedJsonArray1"])
    json_array1 = inputs["nestedJsonArray1"]
    # Editing json_array1 will change the below 2 variables
    json_array1_0 = json_array1[0]
    json_array1_1 = json_array1[1]
    json_array1[1]["data3"] = "editedString"
    json_array1[0]["dataAdd"] = [1, 3.45, "fdnfsj"]
    print(json_array1)
    print(json_array1_0, json_array1_1)

    # [ {"data" : [ {"x": 1}, {"y": 2}, {"z": 3} ]}, {"data2" : [ 1.90, 2.123, 3.45 ],"data3" : "stringValue"}, "stringValue" ]
    print(inputs["nestedJsonArray2"])
    json_array2 = inputs["nestedJsonArray2"]
    json_array2_data = json_array2[0]["data"]
    json_array2_data.append({"t": 4})
    json_array2_size = len(json_array2) # 3
    json_array2_data_size = len(json_array2[0]["data"]) # 4
    json_array2_data2 = json_array2[1]["data2"] # Will be preserved even after json_array2 is assigned something else
    json_array2 = [1, 2]

    ################### Custom functions (print, range, str)
    tensor = nm.zeros([4], "int64")
    tensor = nm.tensor([1, 2, 3, 4],"int32")
    stringCastedList = []
    rangeOutput = 0
    for elem in range(len(tensor)):
        rangeOutput = rangeOutput + 1
        stringCastedList.append(str(elem))
        print(elem)
    stringCastedTensor = nm.tensor(stringCastedList,"string")
    cohort = "wrongcohort"
    # in not available in ListDataVariable
    for i in range(len(nm.get_config()["cohortIds"])):
        if "cohort1" == nm.get_config()["cohortIds"][i]:
            cohort = "cohort1"

    #################### Math functions (exp, pow, to_float, to_int, ....)
    expPowValue = nm.pow(nm.exp(2), 4.5)
    to_float = float("100")
    tensor = nm.zeros([4], "int64")
    tensor = nm.tensor([1, 2, 3, 4],"int32")
    to_string = str(tensor)

    ################### Math Binary Operators
    numberAddition = 5 + 6.1
    stringAddition = "ab" + "cd"
    numberDivision = 57 / 5
    numberMultiplication = 5 * 6.1
    numberSubtraction = 5 - 6.1
    numberPow = 5 ** 6

    ################### Get config
    config = nm.get_config()

    ################### String util functions
    text = "    abcd 1234 $% ABCD     "
    lowerText = text.lower()
    upperText = text.upper()
    strippedText = text.strip()
    joinedString1 = "-".join(("a", "bc", "d"))  # join with tuple of strings
    joinedString2 = ", ".join(["a", "b", "c"])  # join with list of strings
    joinedString3 = "".join(stringCastedTensor) # join with tensor of strings
    joinedString4 = "+".join([])                # Empty list join

    ################### Raw event store
    # Get all events stored
    eventStore = df.filter_all()
    eventStoreNumKeys = eventStore.num_keys()
    eventStoreBundleIds = eventStore.fetch("bundleId", "int32")
    eventStoreStrings = eventStore.fetch("bundleStrings", "string[]")
    eventStoreStrings = nm.tensor(eventStoreStrings,"string")
    eventStoreFloats = eventStore.fetch("bundleFloats", "float[]")
    eventStoreFloats = nm.tensor(eventStoreFloats,"float")
    eventStoreInts = eventStore.fetch("setInts", "int32[]")
    eventStoreInts = nm.tensor(eventStoreInts,"int32")

    # Filter events
    eventStore = df.filter_by_function(get_filtered_event_store)
    filteredEventStoreNumKeys = eventStore.num_keys()
    filteredEventStoreBundleIds = eventStore.fetch("bundleId", "int32")
    filteredEventStoreStrings = eventStore.fetch("bundleStrings", "string[]")
    filteredEventStoreStrings = nm.tensor(filteredEventStoreStrings,"string")
    filteredEventStoreFloats = eventStore.fetch("bundleFloats", "float[]")
    filteredEventStoreFloats = nm.tensor(filteredEventStoreFloats,"float")
    filteredEventStoreInts = eventStore.fetch("setInts", "int32[]")
    filteredEventStoreInts = nm.tensor(filteredEventStoreInts,"int32")
    eventStore2 = eventStore.filter_by_function(get_filtered_event_store2)
    filteredAgainStoreNumKeys = eventStore2.num_keys()

    #################### Processor output
    bundleIdProcessorOutput = bundleIdProcessor.get_for_items(inputs["bundles"])
    bundleIdPromotionCodeProcessorOutput = bundleIdPromotionCodeProcessor.get([1, 1])

    #################### Create tensor functions, then their sort, argsort and arrange for float and strings, tensor.reshape()
    twoDimensionTensor = nm.zeros([2, 2], "int32")
    twoDimensionTensor = nm.tensor([10, 20, 30, 40],"int32")
    twoDimensionTensorLength = len(twoDimensionTensor)
    twoDimensionTensorShape = twoDimensionTensor.shape()
    twoDimensionTensorShape = nm.tensor(twoDimensionTensorShape,"int32")
    reshapedTwoDimensionTensor = twoDimensionTensor.reshape([4])

    doubleTensor = nm.zeros([4], "double")
    doubleTensor = nm.tensor([4.56, 90.78, 1.23, 0.0],"double")
    sortedDoubleTensor = doubleTensor.sort("asc") # Sort modifies the existing tensor, so creating a new one
    doubleTensor = nm.tensor([4.56, 90.78, 1.23, 0.0],"double")
    argsortedDoubleTensor = doubleTensor.argsort("asc")
    topkDoubleTensor = doubleTensor.topk(2, "asc")
    arrangedDoubleTensor = doubleTensor.arrange(argsortedDoubleTensor)

    stringTensor = nm.zeros([4], "string")
    stringTensor = nm.tensor(["D", "C", "A", "P"],"string")
    sortedStringTensor = stringTensor.sort("desc") # Sort modifies the existing tensor, so creating a new one
    stringTensor = nm.tensor(["D", "C", "A", "P"],"string")
    argsortedStringTensor = stringTensor.argsort("desc")
    topkStringTensor = stringTensor.topk(2, "desc")
    arrangedStringTensor = stringTensor.arrange(argsortedStringTensor)

    ################## Empty Tensor Variable
    emptyTensorOfFloats = nm.tensor([],"float")
    emptyTensorOfStrings = nm.zeros([0], "string")

    #################### List Data variable
    arrangedJsonList = [inputs["bundles"][0], inputs["bundles"][1]]
    scores = nm.tensor([0.4, 0.1],"float")
    arrangedJsonList = arrangedJsonList.arrange(scores.argsort("asc"))
    print(arrangedJsonList)
    singleJsonElement = inputs["bundles"][0]
    # Removing elements from list while iterating
    list = [1, 2, 3, 4]
    count = 0
    poppedList = []
    for x in list:
        poppedList.append(list.pop(count))
        count = count + 1

    return {
            "rangeOutput": rangeOutput,
            "stringCastedTensor": stringCastedTensor,
            "expPowValue": expPowValue,
            "to_float": to_float,
            "to_string": to_string,
            "lowerText": lowerText,
            "upperText": upperText,
            "strippedText": strippedText,
            "joinedString1": joinedString1,
            "joinedString2": joinedString2,
            "joinedString3": joinedString3,
            "joinedString4": joinedString4,
            "eventStoreNumKeys": eventStoreNumKeys,
            "eventStoreBundleIds": eventStoreBundleIds,
            "eventStoreStrings": eventStoreStrings,
            "eventStoreFloats": eventStoreFloats,
            "eventStoreInts": eventStoreInts,
            "filteredEventStoreNumKeys": filteredEventStoreNumKeys,
            "filteredAgainStoreNumKeys": filteredAgainStoreNumKeys,
            "filteredEventStoreBundleIds": filteredEventStoreBundleIds,
            "filteredEventStoreStrings": filteredEventStoreStrings,
            "filteredEventStoreFloats": filteredEventStoreFloats,
            "filteredEventStoreInts": filteredEventStoreInts,
            "bundleIdProcessorOutput": bundleIdProcessorOutput,
            "bundleIdPromotionCodeProcessorOutput": bundleIdPromotionCodeProcessorOutput,
            "twoDimensionTensorLength": twoDimensionTensorLength,
            "twoDimensionTensorShape": twoDimensionTensorShape,
            "reshapedTwoDimensionTensor": reshapedTwoDimensionTensor,
            "sortedDoubleTensor": sortedDoubleTensor,
            "argsortedDoubleTensor": argsortedDoubleTensor,
            "topkDoubleTensor": topkDoubleTensor,
            "arrangedDoubleTensor": arrangedDoubleTensor,
            "sortedStringTensor": sortedStringTensor,
            "argsortedStringTensor": argsortedStringTensor,
            "topkStringTensor": topkStringTensor,
            "arrangedStringTensor": arrangedStringTensor,
            "arrangedJsonList": arrangedJsonList,
            "singleJsonElement": singleJsonElement,
            "emptyTensorOfFloats": emptyTensorOfFloats,
            "emptyTensorOfStrings": emptyTensorOfStrings,
            "numberAddition": numberAddition,
            "stringAddition": stringAddition,
            "numberDivision": numberDivision,
            "numberMultiplication": numberMultiplication,
            "numberSubtraction": numberSubtraction,
            "numberPow": numberPow,
            "cohort": cohort,
            "updated_json_object": json_object, # {"data":changedToString, "data2":[1,2.08,3], "data3":abcd, "data4":[3,4,5], null}
            "json_object_data": json_object_data,   # [1, 2, 3]
            "json_object_data2": json_object_data2, # [1, 2.08, 3]
            "json_object_data3": json_object_data3,  # abcd
            "json_array1": json_array1, # [{"data":[1,2,3], "dataAdd":[1,3.45,fdnfsj]},{"data2":[1.9,2.123,3.45], "data3":editedString},1, null]
            "json_array1_0": json_array1_0, # {"data":[1,2,3], "dataAdd":[1,3.45,fdnfsj]}
            "json_array1_1": json_array1_1,  # {"data2":[1.9,2.123,3.45], "data3":editedString},
            "json_array2_data2": json_array2_data2, # [1.9,2.123,3.45]
            "json_array2_data_size": json_array2_data_size, # 3
            "json_array2_size": json_array2_size,    # 2
            "json_array2_data": json_array2_data ,   # [{"x":1},{"y":2},{"z":3},{"t":4}],
            "emptyJson": inputs["emptyJson"],
            "emptyJsonArray":inputs["emptyJsonArray"],
            "poppedList": nm.tensor(poppedList, "int32")
        }
