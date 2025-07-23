/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import com.google.protobuf.Descriptors
import com.google.protobuf.Descriptors.FieldDescriptor
import com.google.protobuf.MessageOrBuilder
import com.google.protobuf.util.JsonFormat
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject
import org.json.JSONArray

class ProtoListWrapper(
    private var listObject: List<ProtoMemberExtender>,
    private var typeRegistry: JsonFormat.TypeRegistry,
    private var listFieldDescriptor: FieldDescriptor,
) : ProtoMemberExtender {
    override var modified = false
    override var message: Any? = null

    override fun get(): Any = listObject

    override fun getValue(index: Int): Any? {
        validateIndex(index)
        return listObject[index]
    }

    override fun setValue(index: Int, value: Any?) {
        validateIndex(index)
        val mutableList =
            when (listObject) {
                is MutableList<*> -> listObject as MutableList<ProtoMemberExtender>
                else -> listObject.toMutableList()
            }
        listObject = mutableList
        val valToSet = getProtoMemberExtenderForSet(value, typeRegistry, listFieldDescriptor)
        mutableList[index] = valToSet
    }

    override fun size(): Int = listObject.size

    override fun arrange(order: IntArray): ProtoListWrapper {
        val wrappedList =
            order
                .map { index ->
                    val item = listObject[index]
                    item
                }
                .toMutableList()
        return ProtoListWrapper(wrappedList, typeRegistry, listFieldDescriptor)
    }

    override fun print(): String {
        val list =
            listObject.map { item ->
                val data = item.getProtoObject().data
                if (data is MessageOrBuilder) {
                    JsonFormat.printer()
                        .usingTypeRegistry(typeRegistry)
                        .preservingProtoFieldNames()
                        .print(data)
                } else {
                    data
                }
            }
        return JSONArray(list).toString()
    }

    override fun isSameAs(field: Descriptors.FieldDescriptor): Boolean {
        return field.type == listFieldDescriptor.type &&
            field.containingType == listFieldDescriptor.containingType
    }

    override fun getType(): String {
        return listFieldDescriptor.name
    }

    override fun getProtoObject(): ProtoObject {
        var isListModified = modified
        val retList: List<*> =
            listObject.map { item ->
                val obj = item.getProtoObject()
                if (obj.modified) {
                    isListModified = true
                }
                obj.data
            }

        return ProtoObject(retList, isListModified)
    }

    override fun pop(index: Int): Any? {
        validateIndex(index)
        val mutableList =
            when (listObject) {
                is MutableList<*> -> listObject as MutableList<ProtoMemberExtender>
                else -> listObject.toMutableList()
            }
        listObject = mutableList
        val value = mutableList.removeAt(index)
        modified = true
        return value
    }

    override fun append(value: Any?) {
        val mutableList =
            when (listObject) {
                is MutableList<*> -> listObject as MutableList<ProtoMemberExtender>
                else -> listObject.toMutableList()
            }
        listObject = mutableList
        val valToAppend = getProtoMemberExtenderForSet(value, typeRegistry, listFieldDescriptor)
        mutableList.add(valToAppend)
        modified = true
    }

    private fun validateIndex(index: Int) {
        if (index < 0 || index >= listObject.size) {
            throw IndexOutOfBoundsException(
                "Index $index out of bounds for length ${listObject.size}"
            )
        }
    }
}
