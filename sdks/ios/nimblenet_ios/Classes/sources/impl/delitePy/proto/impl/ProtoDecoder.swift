/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation
import SwiftProtobuf

struct ProtoObjectDecoder: Decoder {
    var protoWrapper: ProtoObjectWrapper
    
    var numberToValueMap: [Int: Any]
    var keyToNumberMap: [String: Int]
    
    private var currentFieldNumber: Int
    private var fieldIterator: Dictionary<Int, Any>.Iterator
    
    
    init(protoWrapper: ProtoObjectWrapper) throws {
        if protoWrapper.numberToValueMap == nil {
            throw SwiftError.runtimeError("Create decoder only when object wrapper is modified")
        }
        self.protoWrapper = protoWrapper
        self.numberToValueMap = protoWrapper.numberToValueMap!
        self.keyToNumberMap = protoWrapper.keyToNumberMap!
        
        fieldIterator = numberToValueMap.makeIterator()
        currentFieldNumber = 0
    }
    
    mutating func nextFieldNumber() throws -> Int? {
        if let next = fieldIterator.next() {
            currentFieldNumber = next.key
            return next.key
        }
        return nil
    }
    
    private func convertToArray<T>(_ value: inout [T]) throws {
        if let listWrapper = numberToValueMap[currentFieldNumber] as? ProtoListWrapper {
            guard let convertedArray = listWrapper.protoList as? [T] else{
                throw SwiftError.runtimeError("could not cast array to desired type")
            }
            value = convertedArray
        }
        else{
            throw SwiftError.runtimeError("could not cast to expected [T] data type")
        }
    }
    
    private mutating func decodeMessage<M>(from value: Any) throws -> M? where M: SwiftProtobuf.Message {
        if let messageWrapper = value as? ProtoObjectWrapper {
            if messageWrapper.keyToNumberMap == nil {
                return messageWrapper.message as? M
            } else {
                var decoder = try ProtoObjectDecoder(protoWrapper: messageWrapper)
                var val = M.init()
                try val.decodeMessage(decoder: &decoder)
                return val
            }
        } else if let anyWrapper = value as? ProtoAnyWrapper {
            let anyVal = try anyWrapper.convert_to()
            guard let retMessage = anyVal as? M else {
                return nil
            }
            return retMessage
        }
        return nil
    }
    
    mutating func handleConflictingOneOf() throws {
        /// Called by a `oneof` when it already has a value and is being asked to
        /// accept a new value. Some formats require `oneof` decoding to fail in this
        /// case.
    }

    
    // MARK: - Map Fields
    
    mutating func decodeMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMap<KeyType, ValueType>.Type, value: inout SwiftProtobuf._ProtobufMap<KeyType, ValueType>.BaseType) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : SwiftProtobuf.MapValueType {
        guard let mapValue = value as? [String: Any] else {
            throw SwiftError.runtimeError("Expected map to be [String: Any]")
        }
        
        if let val = numberToValueMap[currentFieldNumber] {
            guard let mapWrapper = val as? ProtoMapWrapper else {
                throw SwiftError.runtimeError("Unable to cast to map wrapper")
            }
            try mapWrapper.convert_to(fieldType: fieldType, value: &value)
        }
        else{
            throw SwiftError.runtimeError("Should never happen")
        }
    }
    
    
    mutating func decodeMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufEnumMap<KeyType, ValueType>.Type, value: inout SwiftProtobuf._ProtobufEnumMap<KeyType, ValueType>.BaseType) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : SwiftProtobuf.Enum, ValueType.RawValue == Int {
        throw SwiftError.runtimeError("enum not supported")
    }
    
    mutating func decodeMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.Type, value: inout SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.BaseType) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : Hashable, ValueType : SwiftProtobuf.Message {
        guard let mapValue = value as? [String: Any] else {
            throw SwiftError.runtimeError("Expected map to be [String: Any]")
        }
        
        if let val = numberToValueMap[currentFieldNumber] {
            guard let mapWrapper = val as? ProtoMapWrapper else {
                throw SwiftError.runtimeError("Unable to cast to map wrapper")
            }
            try mapWrapper.convert_to(fieldType: fieldType, value: &value)
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }

    }
    
    mutating func decodeSingularFloatField(value: inout Float) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Float {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularFloatField(value: inout Float?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Float {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedFloatField(value: inout [Float]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Double Fields
    
    mutating func decodeSingularDoubleField(value: inout Double) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Double {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularDoubleField(value: inout Double?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Double {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedDoubleField(value: inout [Double]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Int32 Fields
    
    mutating func decodeSingularInt32Field(value: inout Int32) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularInt32Field(value: inout Int32?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    
    mutating func decodeRepeatedInt32Field(value: inout [Int32]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Int64 Fields
    
    mutating func decodeSingularInt64Field(value: inout Int64) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularInt64Field(value: inout Int64?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedInt64Field(value: inout [Int64]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - UInt32 Fields
    
    mutating func decodeSingularUInt32Field(value: inout UInt32) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularUInt32Field(value: inout UInt32?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedUInt32Field(value: inout [UInt32]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - UInt64 Fields
    
    mutating func decodeSingularUInt64Field(value: inout UInt64) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularUInt64Field(value: inout UInt64?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedUInt64Field(value: inout [UInt64]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - SInt32 Fields
    
    mutating func decodeSingularSInt32Field(value: inout Int32) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularSInt32Field(value: inout Int32?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedSInt32Field(value: inout [Int32]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - SInt64 Fields
    
    mutating func decodeSingularSInt64Field(value: inout Int64) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularSInt64Field(value: inout Int64?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedSInt64Field(value: inout [Int64]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Fixed32 Fields
    
    mutating func decodeSingularFixed32Field(value: inout UInt32) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularFixed32Field(value: inout UInt32?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedFixed32Field(value: inout [UInt32]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Fixed64 Fields
    
    mutating func decodeSingularFixed64Field(value: inout UInt64) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularFixed64Field(value: inout UInt64?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? UInt64 {
            value = fieldValue
            return
        }
        throw SwiftError.runtimeError("cant find number to value map/ field number")
    }
    
    mutating func decodeRepeatedFixed64Field(value: inout [UInt64]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - SFixed32 Fields
    
    mutating func decodeSingularSFixed32Field(value: inout Int32) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularSFixed32Field(value: inout Int32?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int32 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedSFixed32Field(value: inout [Int32]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - SFixed64 Fields
    
    mutating func decodeSingularSFixed64Field(value: inout Int64) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularSFixed64Field(value: inout Int64?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Int64 {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedSFixed64Field(value: inout [Int64]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Bool Fields
    
    mutating func decodeSingularBoolField(value: inout Bool) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Bool {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularBoolField(value: inout Bool?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Bool {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedBoolField(value: inout [Bool]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - String Fields
    
    mutating func decodeSingularStringField(value: inout String) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? String {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularStringField(value: inout String?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? String {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedStringField(value: inout [String]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Bytes Fields
    
    mutating func decodeSingularBytesField(value: inout Data) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Data {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeSingularBytesField(value: inout Data?) throws {
        if let fieldValue = numberToValueMap[currentFieldNumber] as? Data {
            value = fieldValue
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }
    }
    
    mutating func decodeRepeatedBytesField(value: inout [Data]) throws {
        try convertToArray(&value)
    }
    
    // MARK: - Enum Fields
    
    mutating func decodeSingularEnumField<E>(value: inout E) throws where E : SwiftProtobuf.Enum, E.RawValue == Int {
        // Enum can't be set, so decoding doesn't do anything
        return
    }
    
    mutating func decodeSingularEnumField<E>(value: inout E?) throws where E : SwiftProtobuf.Enum, E.RawValue == Int {
        return
    }
    
    mutating func decodeRepeatedEnumField<E>(value: inout [E]) throws where E : SwiftProtobuf.Enum, E.RawValue == Int {
        return
    }
    
    // MARK: - Message Fields
    
    mutating func decodeSingularMessageField<M>(value: inout M?) throws where M: SwiftProtobuf.Message {
        if let val = numberToValueMap[currentFieldNumber] {
            if let decodedMessage: M = try decodeMessage(from: val) {
                value = decodedMessage
            }
        }
        else{
            throw SwiftError.runtimeError("\(#function): could not cast to expected data type")
        }

    }
    
    mutating func decodeRepeatedMessageField<M>(value: inout [M]) throws where M: SwiftProtobuf.Message {
        if let fieldValues = numberToValueMap[currentFieldNumber] as? ProtoListWrapper {
            var messages: [M] = []
            var protoList: [Any] = fieldValues.protoList
            
            for proto in protoList {
                if let decodedMessage: M = try decodeMessage(from: proto) {
                    messages.append(decodedMessage)
                }
            }
            value = messages
        }
        else{
            throw SwiftError.runtimeError("could not cast to repeated message field")
        }
    }
    
    
    mutating func decodeSingularGroupField<G>(value: inout G?) throws where G : SwiftProtobuf.Message {
        try decodeSingularMessageField(value: &value)
    }
    
    mutating func decodeRepeatedGroupField<G>(value: inout [G]) throws where G : SwiftProtobuf.Message {
        try decodeRepeatedMessageField(value: &value)
    }
    
    // MARK: - Extension Fields
    
    mutating func decodeExtensionField(values: inout SwiftProtobuf.ExtensionFieldValueSet, messageType: any SwiftProtobuf.Message.Type, fieldNumber: Int) throws {
        throw SwiftError.runtimeError("ExtensionFieldValueSet not implimented")
    }
    
}
