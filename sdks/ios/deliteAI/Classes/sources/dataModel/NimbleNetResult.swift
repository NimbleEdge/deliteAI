/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

public class NimbleNetResult<T> {
    /// Indicates whether the operation succeeded (`true`) or failed (`false`).
    public var status: Bool
    /// The actual result data when the operation succeeds.
    /// Will be `nil` for failed operations.
    public var payload: T?
    /// Detailed error information when the operation fails.
    /// Will be `nil` for successful operations.
    public var error: NimbleNetError?
    
    /// Initializes a `NimbleNetResult` instance from a dictionary.
    /// 
    /// This initializer parses a dictionary, typically received from the native controller,
    /// to populate the `status`, `error`, and `payload` properties.
    /// 
    /// ## Result States
    /// 
    /// ### Success State
    /// - `status = true`
    /// - `payload` contains the actual result data
    /// - `error = nil`
    /// 
    /// ### Failure State
    /// - `status = false`
    /// - `payload = nil`
    /// - `error` contains detailed failure information
    /// 
    /// - Parameter data: A dictionary containing the result information, including "status",
    ///                   "data" (for payload), and "error".
    public init(data: NSDictionary) {
        self.status = data["status"] as! Bool
        self.error = NimbleNetError(errorDict: data["error"] as? NSDictionary)
        
        if let dataDict = data["data"] as? NSDictionary {
            self.payload = parseData(dataDict)
        } else {
            self.payload = nil
        }
    }
    
    /// Parses the raw data dictionary into the appropriate payload type.
    /// - Parameter data: The raw data dictionary from the native controller.
    /// - Returns: An optional payload of type `T` if parsing is successful, otherwise `nil`.
    private func parseData(_ data: Any) -> T? {
        if T.self == NimbleNetOutput.self {
            return NimbleNetOutput(data: data as! NSDictionary) as? T
        } else if T.self == ModelStatusData.self {
            return ModelStatusData(data: data as! NSDictionary) as? T
        }  else if T.self == Int.self {
            return data as? T
        }
        else if T.self == UserEventdata.self{
            let dataDict = data as? [String: Any]
            let userEventdata = UserEventdata(eventDataJSONString: dataDict?["eventJsonString"] as? String, eventType: dataDict?["eventType"] as? String)
            return userEventdata as? T
        }
        
        return nil
    }
}

public class TensorInternal{
    
    /// The name of the tensor. This typically corresponds to the output name
    /// defined in the machine learning model.
    public var name:String
    
    /// An array of integers representing the dimensions of the tensor (its shape).
    /// For example, `[1, 224, 224, 3]` might represent a batch of 1 image
    /// with 224x224 pixels and 3 color channels.
    public var shape:[Int]
    
    /// The data type of the elements within the tensor.
    public var type:DataType
    
    /// The actual data contained within the tensor. Its type will vary based on `self.type`.
    public var data:Any
    
    /// Initializes a `TensorInternal` instance from a dictionary and a name.
    ///
    /// This initializer is used to reconstruct a tensor from a dictionary
    /// representation, typically received from the native controller or a serialized source.
    ///
    /// - Parameters:
    ///   - data: A dictionary containing the tensor's "data", "shape", and "type".
    ///   - name: The name to assign to this tensor.
    public init(data: NSDictionary, name:String) {
        self.data = data["data"]!
        self.shape = data["shape"] as! [Int]
        self.type = DataType(value: data["type"] as! Int)
        self.name = name
    }
}

public class NimbleNetOutput {
    
    /// An array of `TensorInternal` objects representing the output tensors,
    /// ordered as they were received.
    public var array: [TensorInternal] = []
    
    /// The total number of output tensors.
    public var numOutputs: Int = 0
    
    /// A dictionary (map) of `TensorInternal` objects, where keys are the
    /// names of the output tensors and values are the `TensorInternal` instances.
    public var map: [String: TensorInternal] = [:]
    
    /// Initializes a `NimbleNetOutput` instance from a dictionary.
    ///
    /// This initializer parses a dictionary, typically received from the native controller,
    /// to populate the `map`, `array`, and `numOutputs` properties.
    ///
    /// - Parameter data: A dictionary containing the output information, expected to have
    ///                   an "outputs" dictionary and a "size" integer.
    public init(data: NSDictionary) {
        if let outputsDict = data["outputs"] as? [String: NSDictionary] {
            for (key, value) in outputsDict {
                let singleOutputData = TensorInternal(data: value, name: key)
                map[key] = singleOutputData
            }
            
            array = Array(map.values)
        }
        if let sizeValue = data["size"] as? Int {
            self.numOutputs = sizeValue
        }
    }
    
    /// Provides subscript access to output tensors by their name.
    /// - Parameter key: The string name of the desired output tensor.
    /// - Returns: The `TensorInternal` object associated with the given key, or `nil` if not found.
    public subscript(key: String) -> TensorInternal? {
        return map[key]
    }
}

public class ModelStatusData{
    /// A boolean indicating whether the model is ready for inference (`true`) or not (`false`).
    public var isModelReady: Bool = false
    /// The version string of the model.
    public var version:String = ""
    
    /// Initializes a `ModelStatusData` instance from a dictionary.
    ///
    /// This initializer parses a dictionary, typically received from the native controller,
    /// to populate the `isModelReady` and `version` properties.
    ///
    /// - Parameter data: A dictionary containing the model status information,
    ///                   expected to have "isModelReady" (Bool) and "version" (String) keys.
    public init(data:NSDictionary){
        self.isModelReady = data["isModelReady"] as! Bool
        self.version = data["version"] as! String
    }
}

public class NimbleNetError {
    
    /// Numeric error code identifying the specific type of error.
    public var code: Int = 0
    
    /// Human-readable description of the error.
    /// May be empty if not specified.
    public var message: String = ""
    
    /// Initializes a `NimbleNetError` instance from an optional dictionary.
    ///
    /// This failable initializer attempts to create an error object from a dictionary,
    /// typically received from the native controller. If the `errorDict` is `nil`,
    /// the initialization fails, returning `nil`.
    ///
    /// - Parameter errorDict: An optional dictionary containing error information,
    ///                        expected to have "code" (Int) and "message" (String) keys.
    public init?(errorDict: NSDictionary?) {
        guard let errorDict = errorDict else {
            return nil
        }
        
        self.code = errorDict["code"] as! Int
        self.message = errorDict["message"] as! String
    }
}

//MARK: extensions to stringify
extension NimbleNetResult: CustomStringConvertible {
    public var description: String {
        var description = "NimbleNetResult - Status: \(status)"
        
        if let error = error {
            description += ", Error: \(error.code), Message: \(error.message)"
        }
        
        if let payload = payload {
            description += ", Data: \(payload)"
        }
        
        
        return description
    }
}

extension CustomStringConvertible {
    func indentedDescription(indentationLevel: Int) -> String {
        let indentation = String(repeating: " ", count: indentationLevel * 4)
        return "\(indentation)\(description.replacingOccurrences(of: "\n", with: "\n\(indentation)"))"
    }
}

extension TensorInternal: CustomStringConvertible {
    public var description: String {
        return "TensorInternal:\n  Data: \(data)\n  Shape: \(shape)\n  Type: \(type)"
    }
}

extension NimbleNetOutput: CustomStringConvertible {
    public var description: String {
        var outputDescription = "NimbleNetOutput:\n  Size: \(numOutputs)\n  Outputs:\n"
        for (key, value) in map {
            outputDescription += key.indentedDescription(indentationLevel: 2) + "\n"
            outputDescription += value.indentedDescription(indentationLevel: 3) + "\n"
        }
        return outputDescription
    }
}

extension ModelStatusData: CustomStringConvertible {
    public var description: String {
        return "ModelStatusData:\n  Model Ready: \(isModelReady)\n  Version: \(version)"
    }
}

extension NimbleNetError: CustomStringConvertible {
    public var description: String {
        return "ErrorModel:\n  Code: \(code)\n  Message: \(message)"
    }
}

