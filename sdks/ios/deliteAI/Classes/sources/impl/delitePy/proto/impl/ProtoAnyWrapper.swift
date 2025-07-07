/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation
import SwiftProtobuf

@objc public class ProtoAnyWrapper: NSObject {
    let anyVal: Google_Protobuf_Any
    let messageType: any SwiftProtobuf.Message.Type
    var messageWrapper: ProtoObjectWrapper?
    
    init(anyVal: Google_Protobuf_Any) throws {
        self.anyVal = anyVal
        guard let messageType = Google_Protobuf_Any.messageType(forTypeURL: anyVal.typeURL) else {
            throw SwiftError.runtimeError("Can't find typeURL for any type, it should be registered")
        }
        self.messageType = messageType
    }
    
    @objc public func count() -> Int {
        do {
            try unpack_if_needed()
            return  messageWrapper!.count()
        } catch {
            return -1
        }
    }
    
    @objc public func get_keys(value:UnsafeMutablePointer<AnyObject> ) -> UnsafeMutablePointer<NimbleNetStatus>? {
        do {
            try unpack_if_needed()
            return messageWrapper!.get_keys(value: value)
        } catch {
            return createNimbleNetStatus("\(error)")
        }
    }
    
    @objc public func in_ios_object(key: String, result: UnsafeMutablePointer<Bool>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if key == "@type" {
            result.pointee = Bool(true)
            return nil
        }
        
        do {
            try unpack_if_needed()
            return messageWrapper!.in_ios_object(key: key, result: result)
        } catch {
            return createNimbleNetStatus("\(error)")
        }
    }
    
    private func unpack_if_needed() throws {
        if messageWrapper == nil {
            let unpackedMessage: Message? = try messageType.init(unpackingAny: anyVal)
            if let unpackedMessage = unpackedMessage as? (Message & _ProtoNameProviding) {
                messageWrapper = ProtoObjectWrapper(message: unpackedMessage)
            } else {
                throw SwiftError.runtimeError("Unable to find name in Any proto message")
            }
        }
    }

    @objc public func get_value(key: String, value: UnsafeMutablePointer<AnyObject>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        if key == "@type" {
            value.pointee = anyVal.typeURL as AnyObject
            return nil
        }
        
        do {
            try unpack_if_needed()
            return messageWrapper!.get_value(key: key, value: value)
        } catch {
            return createNimbleNetStatus("\(error)")
        }
    }
    
    @objc public func set_value(key: String, value: AnyObject) -> UnsafeMutablePointer<NimbleNetStatus>? {
        do {
            try unpack_if_needed()
            return messageWrapper!.set_value(key: key, value: value)
        } catch {
            return createNimbleNetStatus("\(error)")
        }
    }
    

    public func convert_to() throws -> Google_Protobuf_Any {
        guard let messageWrapper = messageWrapper else {
            return anyVal
        }
        try messageWrapper.populateMessage()
        return try Google_Protobuf_Any(message: messageWrapper.message)
    }
    
    @objc public func ios_object_to_string(str: UnsafeMutablePointer<UnsafeMutablePointer<CChar>>) -> UnsafeMutablePointer<NimbleNetStatus>? {
        do {
            try unpack_if_needed()
            try messageWrapper!.populateMessage()
            return messageWrapper!.ios_object_to_string(str: str)
        } catch {
            return createNimbleNetStatus("\(error)")
        }
    }
}
