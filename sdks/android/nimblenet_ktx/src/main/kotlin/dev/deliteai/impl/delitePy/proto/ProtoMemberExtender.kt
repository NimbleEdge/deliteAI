/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto

import com.google.protobuf.Descriptors
import com.google.protobuf.MessageOrBuilder
import com.google.protobuf.util.JsonFormat.TypeRegistry
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.delitePy.proto.impl.ProtoAnyWrapper
import dev.deliteai.impl.delitePy.proto.impl.ProtoListWrapper
import dev.deliteai.impl.delitePy.proto.impl.ProtoMapWrapper
import dev.deliteai.impl.delitePy.proto.impl.ProtoNullWrapper
import dev.deliteai.impl.delitePy.proto.impl.ProtoObjectWrapper
import dev.deliteai.impl.delitePy.proto.impl.ProtoPrimitiveWrapper
import org.json.JSONArray
import org.json.JSONObject

data class ProtoObject(val data: Any?, val modified: Boolean)

// NOTE: Every proto object passed to the core must be wrapped inside an implementation of this
// interface
interface ProtoMemberExtender {
    var modified: Boolean
    var message: Any?

    fun get(): Any? = throw UnsupportedOperationException("get is not supported")

    fun getCoreType(): Int = DATATYPE.FE_OBJ.value

    fun getValue(index: Int): Any? =
        throw UnsupportedOperationException("index-based access is not supported")

    fun getValue(key: String): Any? =
        throw UnsupportedOperationException("key-based access is not supported")

    fun setValue(index: Int, value: Any?): Unit =
        throw UnsupportedOperationException("index-based modification is not supported")

    fun setValue(key: String, value: Any?): Unit =
        throw UnsupportedOperationException("key-based modification is not supported")

    fun getKeys(): Array<String> = throw UnsupportedOperationException("keys is not supported")

    fun contains(key: String): Boolean =
        throw UnsupportedOperationException("contains is not supported")

    fun size(): Int = throw UnsupportedOperationException("size is not supported")

    fun arrange(order: IntArray): ProtoListWrapper =
        throw UnsupportedOperationException("arrange is not supported")

    fun pop(index: Int): Any? = throw UnsupportedOperationException("pop by index not supported")

    fun pop(key: String): Any? = throw UnsupportedOperationException("pop by key not supported")

    fun append(value: Any?): Unit = throw UnsupportedOperationException("append not supported")

    fun print(): String

    fun isSameAs(field: Descriptors.FieldDescriptor): Boolean = false

    fun getType(): String

    fun getProtoObject(): ProtoObject

    fun build() {
        message = getProtoObject().data
    }

    fun setModified() {
        modified = true
    }

    fun wrapValue(
        value: Any?,
        typeRegistry: TypeRegistry,
        field: Descriptors.FieldDescriptor,
    ): ProtoMemberExtender =
        when (value) {
            null -> ProtoNullWrapper()
            is MessageOrBuilder -> {
                if (value.descriptorForType == com.google.protobuf.Any.getDescriptor()) {
                    ProtoAnyWrapper(value, typeRegistry, field)
                } else {
                    ProtoObjectWrapper(value, typeRegistry, field)
                }
            }
            is List<*> -> wrapListValue(value, typeRegistry, field)
            is Int,
            is Long,
            is Float,
            is Double,
            is Boolean,
            is String -> ProtoPrimitiveWrapper(value)

            else -> throw Exception("Unsupported proto type")
        }

    fun wrapListValue(
        value: List<*>,
        typeRegistry: TypeRegistry,
        field: Descriptors.FieldDescriptor,
    ): ProtoMemberExtender {
        return if (field.isMapField) {
            // this if needs to be first as Map is also repeated field
            ProtoMapWrapper(value, typeRegistry, field)
        } else if (field.isRepeated) {
            // have to do something similar here
            val list = value.map { wrapValue(it, typeRegistry, field) }
            ProtoListWrapper(list, typeRegistry, field)
        } else {
            throw Exception("Unsupported proto type ")
        }
    }

    fun getProtoMemberExtenderForSet(
        value: Any?,
        typeRegistry: TypeRegistry,
        fieldDescriptor: Descriptors.FieldDescriptor,
    ): ProtoMemberExtender {
        val protoMemberExtender =
            when (value) {
                null -> {
                    require(!fieldDescriptor.isRepeated) { "Cannot set None to list/map field" }
                    ProtoNullWrapper()
                }
                is ProtoMemberExtender -> {
                    require(value.isSameAs(fieldDescriptor)) {
                        "Cannot set ${value.getType()} to ${fieldDescriptor.type}"
                    }
                    value
                }
                // Sets empty map if value is "{}"
                is JSONObject -> {
                    require(value.length() == 0 && fieldDescriptor.isMapField) {
                        "Non-empty JsonObject is not supported"
                    }
                    ProtoMapWrapper(listOf<Any>(), typeRegistry, fieldDescriptor)
                }
                // Sets empty list if value is "[]"
                is JSONArray -> {
                    require(value.length() == 0 && fieldDescriptor.isRepeated) {
                        "Non-empty JsonArray is not supported"
                    }
                    ProtoListWrapper(emptyList(), typeRegistry, fieldDescriptor)
                }
                else -> ProtoPrimitiveWrapper.createForSet(value, fieldDescriptor)
            }
        protoMemberExtender.setModified()
        return protoMemberExtender
    }
}
