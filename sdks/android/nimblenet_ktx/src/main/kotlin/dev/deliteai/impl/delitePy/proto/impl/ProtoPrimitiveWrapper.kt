/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import com.google.protobuf.Descriptors
import com.google.protobuf.Descriptors.FieldDescriptor.JavaType
import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject

class ProtoPrimitiveWrapper(val value: Any?, override var modified: Boolean = false) :
    ProtoMemberExtender {
    override var message: Any? = null

    companion object {
        fun createForSet(
            value: Any,
            descriptor: Descriptors.FieldDescriptor,
        ): ProtoPrimitiveWrapper {
            val modifiedValue =
                when {
                    descriptor.javaType == JavaType.INT && value is Long -> value.toInt()
                    descriptor.javaType == JavaType.LONG && value is Int -> value.toLong()
                    descriptor.javaType == JavaType.FLOAT && value is Double -> value.toFloat()
                    descriptor.javaType == JavaType.DOUBLE && value is Float -> value.toDouble()
                    else -> value
                }
            val expectedClass =
                when (descriptor.javaType) {
                    JavaType.INT -> java.lang.Integer::class.java
                    JavaType.LONG -> java.lang.Long::class.java
                    JavaType.FLOAT -> java.lang.Float::class.java
                    JavaType.DOUBLE -> java.lang.Double::class.java
                    JavaType.BOOLEAN -> java.lang.Boolean::class.java
                    JavaType.STRING -> java.lang.String::class.java
                    else ->
                        throw IllegalArgumentException(
                            "Invalid type passed as primitive: ${descriptor.javaType}"
                        )
                }
            if (!expectedClass.isInstance(modifiedValue)) {
                throw IllegalArgumentException(
                    "Invalid type: Expected ${expectedClass.simpleName}, " +
                        "got ${modifiedValue::class.java.simpleName}"
                )
            }
            return ProtoPrimitiveWrapper(modifiedValue)
        }
    }

    override fun getCoreType(): Int {
        return DATATYPE.fromValue(value)
    }

    override fun getProtoObject(): ProtoObject {
        return ProtoObject(get()!!, modified)
    }

    override fun get(): Any? {
        return value
    }

    override fun print(): String {
        return value.toString()
    }

    override fun getType(): String {
        return value!!::class.toString()
    }
}
