/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import com.google.protobuf.Descriptors
import com.google.protobuf.Descriptors.FieldDescriptor
import com.google.protobuf.Message
import com.google.protobuf.MessageOrBuilder
import com.google.protobuf.util.JsonFormat
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject
import java.util.concurrent.ConcurrentHashMap

class ProtoObjectWrapper(
    private var protoOrBuilder: MessageOrBuilder,
    private var typeRegistry: JsonFormat.TypeRegistry =
        JsonFormat.TypeRegistry.newBuilder().build(),
    private var protoFieldDescriptor: FieldDescriptor? = null,
) : ProtoMemberExtender {
    private val valueCache = ConcurrentHashMap<String, ProtoMemberExtender>()
    override var modified = false
    override var message: Any? = null

    override fun getProtoObject(): ProtoObject {
        var builder: Message.Builder? = null
        valueCache.forEach { (key, _) ->
            var result = valueCache[key]!!.getProtoObject()
            if (result.modified) {
                builder = getBuilder()
                var toSet = result.data
                val descriptor: Descriptors.Descriptor = protoOrBuilder.descriptorForType
                val field =
                    descriptor.fields.find { it.jsonName == key }
                        ?: throw NoSuchFieldException("Field not found: $key")
                when (toSet) {
                    null -> builder!!.clearField(field)
                    else -> builder!!.setField(field, toSet)
                }
            }
        }

        if (builder == null) {
            // unmodified
            return ProtoObject(protoOrBuilder as Message, modified)
        } else {
            return ProtoObject(builder!!.build(), true)
        }
    }

    override fun getValue(key: String): Any? {
        return valueCache.getOrPut(key) {
            val descriptor: Descriptors.Descriptor = protoOrBuilder.descriptorForType
            val field =
                descriptor.fields.find { it.jsonName == key }
                    ?: throw NoSuchFieldException("Field not found: $key")
            wrapValue(protoOrBuilder.getField(field), typeRegistry, field)
        }
    }

    override fun setValue(key: String, value: Any?) {
        val descriptor: Descriptors.Descriptor = protoOrBuilder.descriptorForType
        val field =
            descriptor.fields.find { it.jsonName == key }
                ?: throw NoSuchFieldException("Field not found: $key")

        val valToSet = getProtoMemberExtenderForSet(value, typeRegistry, field)
        valueCache[key] = valToSet
    }

    override fun getKeys(): Array<String> {
        val descriptor: Descriptors.Descriptor = protoOrBuilder.descriptorForType
        return descriptor.fields.map { it.jsonName }.toTypedArray()
    }

    override fun contains(key: String): Boolean {
        if (valueCache.containsKey(key)) {
            return true
        }
        val descriptor: Descriptors.Descriptor = protoOrBuilder.descriptorForType
        val fieldDescriptor = descriptor.fields.find { it.jsonName == key }
        return fieldDescriptor != null && protoOrBuilder.hasField(fieldDescriptor)
    }

    override fun size(): Int {
        return protoOrBuilder.descriptorForType.fields.size
    }

    override fun print(): String {
        val protoObject = getProtoObject()
        return JsonFormat.printer()
            .usingTypeRegistry(typeRegistry)
            .preservingProtoFieldNames()
            .print(protoObject.data as Message)
    }

    private fun getBuilder(): Message.Builder {
        if (protoOrBuilder is Message) {
            protoOrBuilder = (protoOrBuilder as Message).toBuilder()
        }
        return protoOrBuilder as Message.Builder
    }

    override fun isSameAs(field: Descriptors.FieldDescriptor): Boolean {
        return field.type == protoFieldDescriptor!!.type &&
            field.containingType == protoFieldDescriptor!!.containingType
    }

    override fun getType(): String {
        return protoFieldDescriptor!!.name
    }
}
