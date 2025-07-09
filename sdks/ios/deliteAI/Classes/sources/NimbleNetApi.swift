/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import SwiftProtobuf

public class NimbleNetApi{
    
    private static let nimbleNetController: NimbleNetController = NimbleNetController();
    
    public static func restartSession(){
        nimbleNetController.restartSession()
    }
    public static func restartSessionWithId(sessionId: String){
        nimbleNetController.restartSession(withId: sessionId)
    }
    public static func initialize(config:NimbleNetConfig, assetsJson: [[String: Any]]? = nil)->NimbleNetResult<Void>{
        
        var config = config
        var res:NSDictionary
        do {
            let encoder = JSONEncoder()
            let jsonData = try encoder.encode(config)
            if let jsonString = String(data: jsonData, encoding: .utf8) {
                if (config.online) {
                    res = nimbleNetController.initialize_nimblenet_controller(jsonString, assetsJson: nil)! as NSDictionary
                    return NimbleNetResult<Void>(data: NSDictionary(dictionary: res))
                } else {
                    if (assetsJson == nil) {
                        return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "assetsJson cannot be nil in case online flag is set to false.");
                    }
                    // Also copy over assets logic will come here and the modified json string passed to nimblenetcontroller
                    var modifiedAssetsJson = assetsJson
                    do {
                        try modifiedAssetsJson = FileIO.shared.processModules(assetsJson: &modifiedAssetsJson!)
                    } catch let error as NSError {
                        return populateNimbleNetResultWithError(
                            errorCode: error.code != 0 ? error.code : 5000,
                            errorMessage: error.localizedDescription
                        )
                    }
                    // Encode modifiedAssetsJson into a JSON string
                    let modifiedAssetsJsonData = try JSONSerialization.data(withJSONObject: modifiedAssetsJson, options: [])
                    guard let modifiedAssetsJsonString = String(data: modifiedAssetsJsonData, encoding: .utf8) else {
                        return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "Failed to convert modified assetsJson to String.")
                    }

                    // Call the controller with both JSON strings
                    res = nimbleNetController.initialize_nimblenet_controller(jsonString, assetsJson: modifiedAssetsJsonString)! as NSDictionary
                    return NimbleNetResult<Void>(data: NSDictionary(dictionary: res))
                }
                
            }
            else{
                return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "invalid nimblenet config")
                
            }
        } catch {
            return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "exception: initNimbleNet \(error)")
        }
        
    }
    
    public static func initialize(config:String, assetsJson: [[String: Any]]?) -> NimbleNetResult<Void>{
        
        if let jsonData = config.data(using: .utf8) {
            do {
                
                let nimbleNetConfig = try JSONDecoder().decode(NimbleNetConfig.self, from: jsonData)
                
                return initialize(config: nimbleNetConfig, assetsJson: assetsJson)
                
            } catch {
                return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "exception: initNimbleNet \(error)")
            }
        }
        else{
            return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "invalid nimblenet config json")
            
        }
    }
        
    public static func addEvent(events: [String: Any], eventType: String) -> NimbleNetResult<UserEventdata> {
        
        do {
            let jsonData = try JSONSerialization.data(withJSONObject: events, options: .prettyPrinted)
            if let jsonString = String(data: jsonData, encoding: .utf8) {
                let res = nimbleNetController.add_event_controller(jsonString, eventType:eventType)!
                return NimbleNetResult<UserEventdata>(data: NSDictionary(dictionary:res))
            }
            else{
                return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "invalid events")
            }
        } catch {
            return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "exception: addEvent \(error)")
            
        }
    }
    
    public static func addEvent(eventString: String, eventType: String) -> NimbleNetResult<UserEventdata> {
        let res = nimbleNetController.add_event_controller(eventString, eventType:eventType)!
        return NimbleNetResult<UserEventdata>(data: NSDictionary(dictionary:res))

    }
  
    public static func isReady() -> NimbleNetResult<Unit>
    {
        do{
            let res = nimbleNetController.is_ready_controller()
            return NimbleNetResult<Unit>(data: res as! NSDictionary)
        }
        catch{
            return populateNimbleNetResultWithError(errorCode: 5000, errorMessage: "exception: isReady \(error)")
            
        }
    }
    
    public static func runMethod(methodName: String, inputs: [String: NimbleNetTensor]) -> NimbleNetResult<NimbleNetOutput> {
        
        do {
            try verifyUserInputs(inputs: inputs)
        } catch let error as DataTypeMismatchError {
            return NimbleNetResult<NimbleNetOutput>(
                data: [
                    "status": false,
                    "error": [
                        "code": InvalidInputError.inputDataMismatch.rawValue,
                        "message": error.description
                    ]
                ] as NSDictionary
            )
        } catch {
            return NimbleNetResult<NimbleNetOutput>(
                data: [
                    "status": false,
                    "error": [
                        "code": InvalidInputError.inputDataMismatch.rawValue,
                        "message": "error transform error"
                    ]
                ] as NSDictionary
            )
        }


        var inputDict:[String: [String: Any]] = [:]
        for (key, value) in inputs {
            var shape:[Int] = []
            inputDict[key] = convertToDictionary(value)
        }
        
        var res = nimbleNetController.run_task_controller(methodName,modelInputsWithShape: inputDict)!

        if var dataDict = res["data"] as? [String: Any],
           var outputsDict = dataDict["outputs"] as? [String: Any] {
            for (tensorName, tensorData) in outputsDict {
                if var tensorDetails = tensorData as? [String: Any]{
                   if var data = tensorDetails["data"] {
                       if let wrapper = data as? ProtoObjectWrapper{
                           do {
                               try wrapper.populateMessage()
                           }
                           catch{
                               return populateNimbleNetResultWithError(errorCode: ProtoError.failure.rawValue, errorMessage: "proto decoding failed \(error)")
                           }
                           tensorDetails["data"] = wrapper.message
                       }
                        outputsDict[tensorName] = tensorDetails
                    }
                }
            }
            dataDict["outputs"] = outputsDict
            res["data"] = dataDict
        }

        return NimbleNetResult<NimbleNetOutput>(data: NSDictionary(dictionary: res))
        
        func convertToDictionary(_ input: NimbleNetTensor) -> [String: Any] {
            if(input.datatype.rawValue == DataType.FE_OBJ.rawValue){
                var message = input.data as! (Message & _ProtoNameProviding)
                 var wrapper = ProtoObjectWrapper(message: message)
                return [
                    "data": wrapper,
                    "type": input.datatype.value,
                    "shape": input.shape
                ]
            }
            return [
                "data": input.data,
                "type": input.datatype.value,
                "shape": input.shape
            ]
        }
        
    }

    //utils
    private static func createNimbleNetDirectory() -> String {
        guard let documentsDirectory = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first else {
            fatalError("Failed to get documents directory.")
        }
        
        let folderName = "nimbleSDK"
        let folderURL = documentsDirectory.appendingPathComponent(folderName)
        
        do {
            try FileManager.default.createDirectory(at: folderURL, withIntermediateDirectories: true, attributes: nil)
            print("Folder created successfully at path: \(folderURL.path)")
        } catch {
            print("Error creating folder: \(error.localizedDescription)")
        }
        
        return folderURL.path
    }
    
    private static func convertToVoidPointer<T>(_ value: T) -> UnsafeMutableRawPointer {
        let pointer = UnsafeMutablePointer<T>.allocate(capacity: 1)
        pointer.initialize(to: value)
        return UnsafeMutableRawPointer(pointer)
    }
    
    
    private static func populateNimbleNetResultWithError<T>(errorCode:Int,errorMessage:String) -> NimbleNetResult<T>{
        let dict: NSDictionary = [
            "status": false,
            "data": NSNull(),
            "error": [
                "code":errorCode,
                "message":errorMessage
            ] as [String : Any]
        ]
        return NimbleNetResult<T>(data: dict)
    }
    
}


func verifyUserInputs(inputs: [String: NimbleNetTensor]) throws {
    for (_, modelInput) in inputs {
        let data = modelInput.data
        let shape = modelInput.shape

        func validateArray(expectedType: DataType,expectedShape: Int) throws{
            guard modelInput.datatype == expectedType else {
                throw DataTypeMismatchError.arrayTypeMismatch(expected: expectedType)
            }
            guard let shape = shape else {
                throw DataTypeMismatchError.invalidShapeArray
            }
            if (shape.isEmpty) {
                throw DataTypeMismatchError.invalidShapeArray
            }
            
            var shapeLength = 1
            for value in shape {
                shapeLength *= value
                if shapeLength > expectedShape { break }
            }

            guard shapeLength == expectedShape else {
                throw DataTypeMismatchError.invalidShapeArray
            }

        }        

        func validateSingular(expectedType: DataType) throws {
            if modelInput.datatype != expectedType {
                throw DataTypeMismatchError.singularTypeMismatch(expected: expectedType)
            }

            if let shape = shape {
                throw DataTypeMismatchError.invalidShapeSingular
            }
        }

        switch data {
        case let data as [Int32]:
            try validateArray(expectedType: .int32,expectedShape: data.count)
        case let data as [Int64]:
            try validateArray(expectedType: .int64,expectedShape: data.count)
        case let data as [Float]:
            try validateArray(expectedType: .float,expectedShape: data.count)
        case let data as [Double]:
            try validateArray(expectedType: .double,expectedShape: data.count)
        case let data as [Bool]:
            try validateArray(expectedType: .bool,expectedShape: data.count)
        case let data as [String]:
            try validateArray(expectedType: .string,expectedShape: data.count)
        case let data as [Any]:
            try validateArray(expectedType: .jsonArray,expectedShape: data.count)

        case let value as Int32:
            try validateSingular(expectedType: .int32)
        case let value as Int64:
            try validateSingular(expectedType: .int64)
        case let value as Float:
            try validateSingular(expectedType: .float)
        case let value as Double:
            try validateSingular(expectedType: .double)
        case let value as Bool:
            try validateSingular(expectedType: .bool)
        case let value as String:
            try validateSingular(expectedType: .string)
        case let value as [String: Any]:
            try validateSingular(expectedType: .json)
        case let value as Message:
            try validateSingular(expectedType: .FE_OBJ)

        default:
            throw DataTypeMismatchError.unsupportedDataType(type: String(describing: type(of: data)))
        }
    }
}
