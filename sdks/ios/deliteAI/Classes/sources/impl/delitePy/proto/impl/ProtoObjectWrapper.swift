/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation
import SwiftProtobuf

@objc public enum ProtoError: Int {
    case failure = 3576
}

@objc public class ProtoObjectWrapper: NSObject {
    var message: Message & _ProtoNameProviding
    let protobufNameMap: _NameMap
    var numberToValueMap: [Int: AnyObject]?
    var keyToNumberMap: [String: Int]?

    init <T: Message & _ProtoNameProviding>(message: T) {
        self.message = message
        self.protobufNameMap = T._protobuf_nameMap
    }
    
    private func getKeys(from map: [String: Int]) -> [String] {
        return Array(map.keys)
    }
    
    @objc public func count() -> Int {
        if (keyToNumberMap == nil) {
            do {
                try initialize()
            } catch {
               return -1
            }
        }
        return keyToNumberMap!.count
    }
    
    func initialize() throws {
        var visitor = ProtoVisitor()
        try message.traverse(visitor: &visitor)
        numberToValueMap = visitor.fieldMap

        let mirr = Mirror(reflecting: protobufNameMap)
        var anyJsonToNumberMap: Any?
        for child in mirr.children {
            if let key = child.label, key == "jsonToNumberMap" {
                anyJsonToNumberMap = child.value
                break
            }
        }
        
        guard let jsonToNumberMap = anyJsonToNumberMap as? [AnyHashable : Int] else {
            throw SwiftError.runtimeError("Unable to find jsonToNumberMap in proto")
        }
        
        keyToNumberMap = [:]
        for (key, val) in jsonToNumberMap {
            keyToNumberMap![key.description] = val
        }
    }
    
    @objc public func get_keys(value:UnsafeMutablePointer<AnyObject> ) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if (keyToNumberMap == nil) {
            do {
                try initialize()
            } catch {
                return createNimbleNetStatus("\(error)")
            }
        }
        
        value.pointee = getKeys(from: keyToNumberMap!) as AnyObject
        return nil
    }

    @objc public func get_value(key: String, value: UnsafeMutablePointer<AnyObject>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if (keyToNumberMap == nil) {
            do {
                try initialize()
            } catch {
                return createNimbleNetStatus("\(error)")
            }
        }
        
        guard let fieldNumber = keyToNumberMap![key] else {
            return createNimbleNetStatus("Field \(key) not recognised for proto")
        }
        
        guard let anyVal = numberToValueMap![fieldNumber] else {
            return createNimbleNetStatus("Key has no value")
        }
        
        value.pointee = anyVal
        return nil
    }
    
    
    @objc public func in_ios_object(key: String, result: UnsafeMutablePointer<Bool>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if (keyToNumberMap == nil) {
            do {
                try initialize()
            } catch {
                print("Error in initialize of ObjectWrapper")
                return createNimbleNetStatus("Error in initialize of ObjectWrapper")
            }
        }
        
        guard let fieldNumber = keyToNumberMap![key] else {
            result.initialize(to: false)
            return nil
        }
        
        result.initialize(to: numberToValueMap!.keys.contains(fieldNumber))
        return nil
    }
    
    public func populateMessage() throws {
        if self.numberToValueMap == nil {
            // No modifications have been made, just return
            return;
        }
        
        do {
            var decoder = try ProtoObjectDecoder(protoWrapper: self)
            try self.message.decodeMessage(decoder: &decoder)
        }
        catch{
            throw SwiftError.runtimeError("error decoding proto \(error)")
        }
    }
    
    
    @objc public func ios_object_to_string(str: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        do{
            try self.populateMessage()
            let objectString = "\(self.message)"
            objectString.withCString { swiftString in
                str.pointee = strdup(swiftString)
            }
        }
        catch{
            return createNimbleNetStatus("\(error)")
        }
        
        return nil
    }

    
    @objc public func set_value(key: String, value: AnyObject) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if (keyToNumberMap == nil) {
            do {
                try initialize()
            } catch {
                return createNimbleNetStatus("\(error)")
            }
        }
        guard let fieldNumber = keyToNumberMap![key] else {
            return createNimbleNetStatus("Field \(key) not recognised for proto")
        }
        // TODO: check datatype of value, return more get descriptive error message
        numberToValueMap![fieldNumber] = value
        
        return nil
    }
}
