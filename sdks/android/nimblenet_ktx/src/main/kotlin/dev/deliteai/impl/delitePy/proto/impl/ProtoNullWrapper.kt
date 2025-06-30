/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.delitePy.proto.impl

import dev.deliteai.impl.common.DATATYPE
import dev.deliteai.impl.delitePy.proto.ProtoMemberExtender
import dev.deliteai.impl.delitePy.proto.ProtoObject

class ProtoNullWrapper : ProtoMemberExtender {
    override var modified: Boolean = false
    override var message: Any? = null

    override fun getCoreType(): Int {
        return DATATYPE.NONE.value
    }

    override fun print(): String {
        return "null"
    }

    override fun getType(): String {
        return "null"
    }

    override fun getProtoObject(): ProtoObject {
        return ProtoObject(null, true)
    }
}
