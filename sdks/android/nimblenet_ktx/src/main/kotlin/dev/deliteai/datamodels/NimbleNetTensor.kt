/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

import dev.deliteai.impl.common.DATATYPE

/**
 * Multi-type tensor for ML data in NimbleNet SDK. Supports primitives, arrays, JSON, protobufs, and
 * functions.
 *
 * @property data actual tensor data (nullable for outputs)
 * @property datatype enum indicating data interpretation
 * @property shape optional dimensions (null or empty for scalars)
 */
class NimbleNetTensor {
    var data: Any?
    var datatype: DATATYPE
    var shape: IntArray?

    /** Scalar Int tensor (32-bit). */
    constructor(data: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    /** Scalar Long tensor (64-bit). */
    constructor(data: Long, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    /** Scalar Float tensor (32-bit). */
    constructor(data: Float, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    /** Scalar Double tensor (64-bit). */
    constructor(data: Double, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    /** Scalar Boolean tensor. */
    constructor(data: Boolean, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    /** Scalar String tensor. */
    constructor(data: String, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.STRING
        this.shape = shape
    }

    /** Array of Ints tensor. */
    constructor(data: IntArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    /** Array of Longs tensor. */
    constructor(data: LongArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    /** Array of Floats tensor. */
    constructor(data: FloatArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    /** Array of Doubles tensor. */
    constructor(data: DoubleArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    /** Array of Booleans tensor. */
    constructor(data: BooleanArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    /** Custom type by int code. */
    constructor(data: Any?, datatypeInt: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.fromInt(datatypeInt)
        this.shape = shape
    }

    /** Explicit datatype constructor. */
    constructor(data: Any?, datatype: DATATYPE, shape: IntArray? = null) {
        this.data = data
        this.datatype = datatype
        this.shape = shape
    }

    /** Numeric representation of datatype. */
    fun getDatatypeInt(): Int = datatype.value

    /** Number of dimensions (0 for scalars). */
    fun getShapeArrayLength(): Int = shape?.size ?: 0
}
