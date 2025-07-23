/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import com.google.protobuf.Descriptors
import com.google.protobuf.Descriptors.FieldDescriptor
import com.google.protobuf.DynamicMessage
import com.google.protobuf.MapEntry
import com.google.protobuf.MessageOrBuilder
import com.google.protobuf.util.JsonFormat
import com.google.protobuf.util.JsonFormat.TypeRegistry
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject
import org.json.JSONObject

class ProtoMapWrapper(
    private val listOfKeyValuePairs: List<*>,
    private val typeRegistry: TypeRegistry,
    private val mapFieldDescriptor: FieldDescriptor,
) : ProtoMemberExtender {
    override var modified = false
    override var message: Any? = null
    private var mapObject: Map<String, ProtoMemberExtender> =
        listOfKeyValuePairs.associate { entry ->
            val ret = getKeyValuePairFromEntry(entry!!)
            ret.first to wrapValue(ret.second, typeRegistry, mapFieldDescriptor)
        }

    private fun getKeyValuePairFromEntry(entry: Any): Pair<String, Any> {
        return when (entry) {
            is MapEntry<*, *> -> {
                if (entry.key is String) {
                    entry.key as String to entry.value
                } else {
                    throw Exception("Currently only Map with String Keys supported")
                }
            }
            is DynamicMessage -> {
                val descriptor = entry.descriptorForType
                val key: String = entry.getField(descriptor.fields[0]).toString()
                val value = entry.getField(descriptor.fields[1])
                key to value
            }
            else -> throw Exception("Map has unexpected types, expected MapEntry or DynamicMessage")
        }
    }

    private val originalMapObject: Map<String, Any> =
        listOfKeyValuePairs.associate { entry ->
            val ret = getKeyValuePairFromEntry(entry!!)
            ret.first to entry
        }

    override fun get(): Any = mapObject

    override fun getValue(key: String): Any? =
        mapObject[key] ?: throw NoSuchFieldException("Field not found: $key")

    override fun setValue(key: String, value: Any?) {
        val mutableMap =
            when (mapObject) {
                is MutableMap<String, ProtoMemberExtender> ->
                    mapObject as MutableMap<String, ProtoMemberExtender>
                else -> {
                    val ret: MutableMap<String, ProtoMemberExtender> = mapObject.toMutableMap()
                    mapObject = ret
                    ret
                }
            }
        val valueField = mapFieldDescriptor.messageType.findFieldByName("value")
        val valToSet = getProtoMemberExtenderForSet(value, typeRegistry, valueField)
        mutableMap[key] = valToSet
    }

    override fun getKeys(): Array<String> = mapObject.keys.toTypedArray()

    override fun contains(key: String): Boolean = mapObject.containsKey(key)

    override fun size(): Int = mapObject.size

    override fun print(): String {
        val jsonMap =
            mapObject.mapValues { (_, value) ->
                val data = value.getProtoObject().data
                if (data is MessageOrBuilder) {
                    JsonFormat.printer()
                        .usingTypeRegistry(typeRegistry)
                        .preservingProtoFieldNames()
                        .print(data)
                } else {
                    data
                }
            }
        return JSONObject(jsonMap).toString()
    }

    override fun isSameAs(field: Descriptors.FieldDescriptor): Boolean {
        return field.type == mapFieldDescriptor.type &&
            field.containingType == mapFieldDescriptor.containingType
    }

    override fun getType(): String {
        return mapFieldDescriptor.name
    }

    override fun getProtoObject(): ProtoObject {
        // Create a new MapEntry.Builder dynamically
        var isMapModified = modified
        val map: List<*> =
            mapObject.map() { (key, value) ->
                val protoValue = value.getProtoObject()
                if (!protoValue.modified) {
                    return@map originalMapObject[key]
                }
                isMapModified = true
                val builder = DynamicMessage.newBuilder(mapFieldDescriptor.messageType)
                val keyField = mapFieldDescriptor.messageType.findFieldByName("key")
                val valueField = mapFieldDescriptor.messageType.findFieldByName("value")
                var ret =
                    builder.setField(keyField, key).setField(valueField, protoValue.data).build()
                return@map ret
            }
        return ProtoObject(map, isMapModified)
    }

    override fun build() {
        val dynamicMessageList = getProtoObject().data as List<*>
        message =
            dynamicMessageList.associate { entry ->
                val ret = getKeyValuePairFromEntry(entry!!)
                ret.first to ret.second
            }
    }

    override fun pop(key: String): Any? {
        val mutableMap =
            when (mapObject) {
                is MutableMap<String, ProtoMemberExtender> ->
                    mapObject as MutableMap<String, ProtoMemberExtender>
                else -> {
                    val ret: MutableMap<String, ProtoMemberExtender> = mapObject.toMutableMap()
                    mapObject = ret
                    ret
                }
            }
        if (mutableMap.containsKey(key)) {
            val value = mutableMap.remove(key)
            modified = true
            return value
        }
        throw Exception("Trying to remove key=$key, not present in map.")
    }
}
