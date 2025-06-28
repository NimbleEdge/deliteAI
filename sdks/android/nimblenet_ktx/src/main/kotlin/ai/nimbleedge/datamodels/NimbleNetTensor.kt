package ai.nimbleedge.datamodels

import ai.nimbleedge.impl.common.DATATYPE

class NimbleNetTensor {
    // NOTE: data is nullable only for outputs, inputs cannot be null
    var data: Any?
    var datatype: DATATYPE
    var shape: IntArray?

    // Constructors for singular values
    constructor(data: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    constructor(data: Long, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    constructor(data: Float, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    constructor(data: Double, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    constructor(data: Boolean, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    constructor(data: String, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.STRING
        this.shape = shape
    }

    // Constructors for arrays
    constructor(data: IntArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    constructor(data: LongArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    constructor(data: FloatArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    constructor(data: DoubleArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    constructor(data: BooleanArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    // Original constructor for Any type
    constructor(data: Any?, datatypeInt: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.fromInt(datatypeInt)
        this.shape = shape
    }

    constructor(data: Any?, datatype: DATATYPE, shape: IntArray? = null) {
        this.data = data
        this.datatype = datatype
        this.shape = shape
    }

    fun getDatatypeInt(): Int {
        return datatype.value
    }

    fun getShapeArrayLength(): Int {
        return shape?.size ?: 0
    }
}
