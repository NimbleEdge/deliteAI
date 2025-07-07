/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation
import SwiftProtobuf

@objc public class ProtoMapWrapper: NSObject {
    private var protoMap: [String:AnyObject]
    
    private func getAllKeys() -> [String] {
        return Array(protoMap.keys)
    }

    @objc public func count() -> Int {
        return protoMap.count
    }
    
    init (map: [String:AnyObject]) {
        self.protoMap = map
    }
    
    init (map: [String:Message]) throws {
        self.protoMap = try map.mapValues { value in
            guard let value = value as? Message & _ProtoNameProviding else {
                throw SwiftError.runtimeError("Unable to find name from message in map value")
            }
            return ProtoObjectWrapper(message: value)
        }
    }
    
    
    @objc public func get_value(key: String, value: UnsafeMutablePointer<AnyObject>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        guard let mapVal = protoMap[key] else {
            return createNimbleNetStatus("Key \(key) not found in map")
        }
        value.pointee = mapVal
        return nil
    }
    
    @objc public func set_value(key: String, value: AnyObject) -> UnsafeMutablePointer<NimbleNetStatus>? {
        protoMap[key] = value
        return nil
    }
    
    @objc public func get_keys(value:UnsafeMutablePointer<AnyObject> ) -> UnsafeMutablePointer<NimbleNetStatus>? {
        value.pointee = getAllKeys() as AnyObject
        return nil
    }
    
    
    @objc public func in_ios_object(key: String, result: UnsafeMutablePointer<Bool>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        result.initialize(to: protoMap.keys.contains(key))
        return nil
    }
    
    @objc public func ios_object_to_string(str: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        let objectString = "\(self.protoMap)"
        objectString.withCString { swiftString in
            str.pointee = strdup(swiftString)
        }
        
        return nil
    }


    
    public func convert_to<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMap<KeyType, ValueType>.Type, value: inout SwiftProtobuf._ProtobufMap<KeyType, ValueType>.BaseType) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : SwiftProtobuf.MapValueType {
        guard let finalValue = protoMap as? SwiftProtobuf._ProtobufMap<KeyType, ValueType>.BaseType else {
            throw SwiftError.runtimeError("Cannot convert map from [String: Value] to desired [Key: Value]")
        }
        value = finalValue
    }
    
    public func convert_to<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.Type, value: inout SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.BaseType) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : Hashable, ValueType : SwiftProtobuf.Message  {
        let map = try protoMap.mapValues { objectWrapper in
            guard let objectWrapper = objectWrapper as? ProtoObjectWrapper else {
                throw SwiftError.runtimeError("Couldn't convert value in message map to ProtoObjectWrapper")
            }
            try objectWrapper.populateMessage()
            return objectWrapper.message
        }
        guard let finalValue = map as? SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.BaseType else {
            throw SwiftError.runtimeError("Cannot convert map from [String: Message] to desired [Key: Value]")
        }
        value = finalValue
    }
    
}
