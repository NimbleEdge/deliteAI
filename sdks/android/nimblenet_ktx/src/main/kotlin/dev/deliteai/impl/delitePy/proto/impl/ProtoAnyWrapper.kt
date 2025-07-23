/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import com.google.protobuf.ByteString
import com.google.protobuf.Descriptors
import com.google.protobuf.Descriptors.FieldDescriptor
import com.google.protobuf.DynamicMessage
import com.google.protobuf.Message
import com.google.protobuf.MessageOrBuilder
import com.google.protobuf.util.JsonFormat
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject

class ProtoAnyWrapper(
    private var protoOrBuilder: MessageOrBuilder,
    private var typeRegistry: JsonFormat.TypeRegistry =
        JsonFormat.TypeRegistry.newBuilder().build(),
    private var protoFieldDescriptor: FieldDescriptor? = null,
) : ProtoMemberExtender {
    override var modified = false
    override var message: Any? = null
    private var protoMemberExtender: ProtoMemberExtender? = null
    private var typeUrl: String

    init {
        assert(protoOrBuilder.descriptorForType == com.google.protobuf.Any.getDescriptor())
        // setting only typeUrl in init{}
        val typeField = protoOrBuilder.descriptorForType.findFieldByName("type_url")
        typeUrl = protoOrBuilder.allFields[typeField].toString()
    }

    private fun convertToDynamicMessageFromAny(): ProtoMemberExtender {
        // handling for Any Types
        val valueField = protoOrBuilder.descriptorForType.findFieldByName("value")
        val value = protoOrBuilder.allFields[valueField] as ByteString
        val classFullName = typeUrl.substringAfter("type.googleapis.com/")
        val descriptor =
            typeRegistry.find(classFullName)
                ?: throw IllegalArgumentException("Unknown type: $classFullName")

        val dynamicMessage = DynamicMessage.parseFrom(descriptor, value)
        return wrapValue(dynamicMessage, typeRegistry, protoFieldDescriptor!!)
    }

    override fun getProtoObject(): ProtoObject {
        if (protoMemberExtender == null) {
            // Any type was never casted
            return ProtoObject(protoOrBuilder, modified)
        }
        // Any type was casted
        val protoObject = protoMemberExtender!!.getProtoObject()
        val anyMessage = com.google.protobuf.Any.pack(protoObject.data as Message)
        return ProtoObject(anyMessage, true)
    }

    override fun getValue(key: String): Any? {
        if (key == "@type") {
            return ProtoPrimitiveWrapper(typeUrl)
        }
        if (protoMemberExtender == null) {
            protoMemberExtender = convertToDynamicMessageFromAny()
        }
        return protoMemberExtender!!.getValue(key)
    }

    override fun setValue(key: String, value: Any?) {
        if (protoMemberExtender == null) {
            protoMemberExtender = convertToDynamicMessageFromAny()
        }
        protoMemberExtender!!.setValue(key, value)
    }

    override fun getKeys(): Array<String> {
        if (protoMemberExtender == null) {
            protoMemberExtender = convertToDynamicMessageFromAny()
        }
        return protoMemberExtender!!.getKeys()
    }

    override fun contains(key: String): Boolean {
        if (key == "@type") {
            return true
        }
        if (protoMemberExtender == null) {
            protoMemberExtender = convertToDynamicMessageFromAny()
        }
        return protoMemberExtender!!.contains(key)
    }

    override fun size(): Int {
        val keys = getKeys()
        return keys.size
    }

    override fun print(): String {
        val protoObject = getProtoObject()
        return JsonFormat.printer()
            .usingTypeRegistry(typeRegistry)
            .preservingProtoFieldNames()
            .print(protoObject.data as Message)
    }

    override fun isSameAs(field: Descriptors.FieldDescriptor): Boolean {
        return field.type == Descriptors.FieldDescriptor.Type.MESSAGE
    }

    override fun getType(): String {
        return protoFieldDescriptor!!.name
    }
}
