# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

widgetTable = nm.Dataframe(
    {"rowId": "int32", "widgetId": "int32", "restaurantId": "int32"}
)

restaurantsTable = nm.Dataframe({"id": "int32", "Name": "string"})


def add_initial_data(inp):
    dataStr = inp["data"]
    dataJson = nm.parse_json(dataStr)

    for row in dataJson["rows"]:
        rowId = row["id"]
        for widget in row["widgets"]:
            widget["rowId"] = rowId
            widgetTable.append(widget)

    for restaurant in dataJson["restaurants"]:
        restaurantsTable.append(restaurant)

    json = inp["nestedJson"]
    json["bigValue"] = json["bigValue"] + 1
    print(json["bigValue"])
    json["newKey"] = [100, 900]
    json["key2"].append("addValue")
    json["key2"].append(1)
    poppedValueFromList = json["key2"].pop(0)
    print(poppedValueFromList)
    json["key4"]["newKeyInKey4"] = "newString"
    for key in json.keys():
        print(key, json[key])
    json.pop("key1")
    poppedValue = json.pop("key5")
    print(json)

    array = inp["nestedArray"]
    array[0]["key1"] = array[0]["key1"]+1
    for i in range(len(array)):
        print(array[i])
    array[0].pop("key4")
    array.pop(1)
    print(array)

    return {
        "nestedJson": json,
        "nestedArray": array,
        "poppedValue": poppedValue,
        "poppedValueFromList": poppedValueFromList
        }
