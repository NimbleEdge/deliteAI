# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

from delitepy import nimblenet as nm

def main(inputs):
    #################### Nested Json objects/array
    # { "data" : [ 1, 2, 3 ], "data2" : [ 1, 2.08, 3 ], "data3" : "abcd" }
    print("Found json object")
    print(inputs["nestedJsonObject"])
    json_object = inputs["nestedJsonObject"]
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

    # [ {"data" : [ 1, 2, 3 ]}, {"data2" : [ 1.90, 2.123, 3.45 ], "data3" : "stringValue"}, 1 ]
    print(inputs["nestedJsonArray1"])
    json_array1 = inputs["nestedJsonArray1"]
    # Editing json_array1 will change the below 2 variables
    json_array1_1 = json_array1[1]
    json_array1_2 = json_array1[2]
    json_array1[2]["data3"] = "editedString"
    json_array1[1]["dataAdd"] = [1, 3.45, "fdnfsj"]
    print(json_array1)
    print(json_array1_1, json_array1_2)

    # [ {"data" : [ {"x": 1}, {"y": 2}, {"z": 3} ]}, {"data2" : [ 1.90, 2.123, 3.45 ],"data3" : "stringValue"}, "stringValue" ]
    print(inputs["nestedJsonArray2"])
    json_array2 = inputs["nestedJsonArray2"]
    json_array2_data = json_array2[0]["data"]
    json_array2_data.append({"t": 4})
    json_array2_size = len(json_array2) # 3
    json_array2_data_size = len(json_array2[0]["data"]) # 4
    json_array2_data2 = json_array2[1]["data2"] # Will be preserved even after json_array2 is assigned something else
    json_array2 = [1, 2]
    print(json_array2_data)
    print(json_array2_data2)
    print(json_array2_data_size)
    print(json_array2_size)

    return {
            "updated_json_object": json_object, # {"data":changedToString, "data2":[1,2,3], "data3":abcd, "data4":[3,4,5]}
            "json_object_data": json_object_data,   # [1, 2, 3]
            "json_object_data2": json_object_data2, # [1, 2, 3]
            "json_object_data3": json_object_data3,  # abcd
            "json_array1": json_array1, # [{"data":[1,2,3], "dataAdd":[1,3.45,fdnfsj]},{"data2":[1.9,2.123,3.45], "data3":editedString},1]
            "json_array1_1": json_array1_1, # {"data":[1,2,3], "dataAdd":[1,3.45,fdnfsj]}
            "json_array1_2": json_array1_2,  # {"data2":[1.9,2.123,3.45], "data3":editedString},
            "json_array2_data2": json_array2_data2, # [1.9,2.123,3.45]
            "json_array2_data_size": json_array2_data_size, # 4
            "json_array2_size": json_array2_size,    # 2
            "json_array2_data": json_array2_data    # [{"x":1},{"y":2},{"z":3},{"t":4}]
        }
