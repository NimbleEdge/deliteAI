/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
import Foundation
import SwiftProtobuf

struct ProtoVisitor: Visitor {
    var fieldMap: [Int: AnyObject] = [:]
    
    private func convertMessageToWrapper<M: Message>(message: M) throws -> AnyObject {
        guard let value = message as? (Message & _ProtoNameProviding) else {
            throw SwiftError.runtimeError("Can't find name in proto message")
        }
        
        if let anyVal = value as? Google_Protobuf_Any {
            return try ProtoAnyWrapper(anyVal: anyVal)
        } else {
            return ProtoObjectWrapper(message: value)
        }
    }
    
    
    mutating func visitMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMap<KeyType, ValueType>.Type, value: SwiftProtobuf._ProtobufMap<KeyType, ValueType>.BaseType, fieldNumber: Int) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : SwiftProtobuf.MapValueType {
        if (fieldType.Key != String.self) {
            throw SwiftError.runtimeError("Map has to have string as key")
        }
        guard let mapValue = value as? [String: AnyObject] else {
            throw SwiftError.runtimeError("Unable to cast map to [String: Message]")
        }
        fieldMap[fieldNumber] = ProtoMapWrapper(map: mapValue)
    }
    
    mutating func visitMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufEnumMap<KeyType, ValueType>.Type, value: SwiftProtobuf._ProtobufEnumMap<KeyType, ValueType>.BaseType, fieldNumber: Int) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : SwiftProtobuf.Enum, ValueType.RawValue == Int {
        throw SwiftError.runtimeError("Enum is not supported")
    }
    
    mutating func visitMapField<KeyType, ValueType>(fieldType: SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.Type, value: SwiftProtobuf._ProtobufMessageMap<KeyType, ValueType>.BaseType, fieldNumber: Int) throws where KeyType : SwiftProtobuf.MapKeyType, ValueType : Hashable, ValueType : SwiftProtobuf.Message {
        if (fieldType.Key != String.self) {
            throw SwiftError.runtimeError("Map has to have string as key")
        }
        guard let mapValue = value as? [String: Message] else {
            throw SwiftError.runtimeError("Unable to cast map to [String: Message]")
        }
        
        fieldMap[fieldNumber] = try ProtoMapWrapper(map: mapValue)
    }
    
    mutating func visitUnknown(bytes: Data) throws {
        
    }
    
    
    mutating func visitSingularFloatField(value: Float, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularDoubleField(value: Double, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularInt32Field(value: Int32, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularInt64Field(value: Int64, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularUInt32Field(value: UInt32, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularUInt64Field(value: UInt64, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularBoolField(value: Bool, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularStringField(value: String, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularBytesField(value: Data, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = value as AnyObject
    }
    
    mutating func visitSingularEnumField<E: Enum>(value: E, fieldNumber: Int) throws {
        
    }
    
    mutating func visitSingularMessageField<M: Message>(value: M, fieldNumber: Int) throws {
        fieldMap[fieldNumber] = try convertMessageToWrapper(message: value)
    }
    
    mutating func visitRepeatedInt32Field(value: [Int32], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject])
    }
    
    mutating func visitRepeatedStringField(value: [String], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    public mutating func visitRepeatedFloatField(value: [Float], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedDoubleField(value: [Double], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    
    public mutating func visitRepeatedInt64Field(value: [Int64], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedUInt32Field(value: [UInt32], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedUInt64Field(value: [UInt64], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedSInt32Field(value: [Int32], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedSInt64Field(value: [Int64], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedFixed32Field(value: [UInt32], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedFixed64Field(value: [UInt64], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedSFixed32Field(value: [Int32], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedSFixed64Field(value: [Int64], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedBoolField(value: [Bool], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedBytesField(value: [Data], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
    public mutating func visitRepeatedEnumField<E: Enum>(value: [E], fieldNumber: Int) throws {

    }
    
    public mutating func visitRepeatedMessageField<M: Message>(value: [M], fieldNumber: Int) throws {
        var messageArray:[AnyObject] = []
        
        for v in value {
            messageArray.append(try convertMessageToWrapper(message: v))
        }
        
        fieldMap[fieldNumber] = ProtoListWrapper(array: messageArray)
        
    }
    
    public mutating func visitRepeatedGroupField<G: Message>(value: [G], fieldNumber: Int) throws {
        fieldMap[fieldNumber] = ProtoListWrapper(array: value as [AnyObject]) 
    }
    
}
