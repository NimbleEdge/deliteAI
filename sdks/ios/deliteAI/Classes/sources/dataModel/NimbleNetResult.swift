/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

public class TensorInternal{
    public var name:String
    public var shape:[Int]
    public var type:DataType
    public var data:Any
    
    init(data: NSDictionary, name:String) {
        self.data = data["data"]!
        self.shape = data["shape"] as! [Int]
        self.type = DataType(value: data["type"] as! Int)
        self.name = name
    }
}

public class NimbleNetOutput {
    public var array: [TensorInternal] = []
    public var numOutputs: Int = 0
    public var map: [String: TensorInternal] = [:]
    
    init(data: NSDictionary) {
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
    
    public subscript(key: String) -> TensorInternal? {
        return map[key]
    }
}

public class ModelStatusData{
    public var isModelReady: Bool = false
    public var version:String = ""
    
    init(data:NSDictionary){
        self.isModelReady = data["isModelReady"] as! Bool
        self.version = data["version"] as! String
    }
}

public class NimbleNetError {
    public var code: Int = 0
    public var message: String = ""
    
    init?(errorDict: NSDictionary?) {
        guard let errorDict = errorDict else {
            return nil
        }
        
        self.code = errorDict["code"] as! Int
        self.message = errorDict["message"] as! String
        
    }
}

public class NimbleNetResult<T> {
    public var status: Bool
    public var payload: T?
    public var error: NimbleNetError?
    
    init(data: NSDictionary) {
        self.status = data["status"] as! Bool
        self.error = NimbleNetError(errorDict: data["error"] as? NSDictionary)
        
        if let dataDict = data["data"] as? NSDictionary {
            self.payload = parseData(dataDict)
        } else {
            self.payload = nil
        }
    }
    
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

//extensions to stringify

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

