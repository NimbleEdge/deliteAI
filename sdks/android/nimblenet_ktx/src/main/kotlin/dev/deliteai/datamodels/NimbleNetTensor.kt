/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

import dev.deliteai.impl.common.DATATYPE

/**
 * Multi-type tensor container for machine learning data in the NimbleNet SDK.
 * 
 * [NimbleNetTensor] is the fundamental data structure for representing inputs and outputs
 * in machine learning operations. It provides a type-safe, flexible way to handle diverse
 * data types including primitives, arrays, JSON objects, Protocol Buffers, and even
 * Kotlin functions for dynamic computation.
 * 
 * ## Supported Data Types
 * 
 * ### Primitive Types
 * - **Int** (32-bit integers): DATATYPE.INT32
 * - **Long** (64-bit integers): DATATYPE.INT64  
 * - **Float** (32-bit floating point): DATATYPE.FLOAT
 * - **Double** (64-bit floating point): DATATYPE.DOUBLE
 * - **Boolean**: DATATYPE.BOOL
 * - **String**: DATATYPE.STRING
 * 
 * ### Array Types  
 * - **IntArray**: Efficient primitive arrays for numerical data
 * - **LongArray**: 64-bit integer arrays
 * - **FloatArray**: Single-precision floating point arrays
 * - **DoubleArray**: Double-precision floating point arrays  
 * - **BooleanArray**: Boolean flag arrays
 * 
 * ### Complex Types
 * - **JSONObject**: DATATYPE.JSON for structured data
 * - **JSONArray**: DATATYPE.JSON_ARRAY for lists and sequences
 * - **Protocol Buffers**: DATATYPE.FE_OBJ for type-safe structured data
 * - **Functions**: DATATYPE.FUNCTION for dynamic computation
 * 
 * ## Shape Information
 * 
 * Tensors can include optional shape information to describe multi-dimensional data:
 * - **Scalars**: shape = null or empty array
 * - **Vectors**: shape = [length] (e.g., [100] for 100-element vector)  
 * - **Matrices**: shape = [rows, cols] (e.g., [28, 28] for 28x28 image)
 * - **Higher dimensions**: shape = [dim1, dim2, dim3, ...] (e.g., [1, 224, 224, 3] for batch of RGB images)
 * 
 * ## Usage Examples
 * 
 * ### Scalar Values
 * ```kotlin
 * // Simple scalars (shape is automatically null)
 * val intTensor = NimbleNetTensor(42)
 * val floatTensor = NimbleNetTensor(3.14f)
 * val boolTensor = NimbleNetTensor(true)
 * val stringTensor = NimbleNetTensor("hello world")
 * 
 * // Scalars with explicit shape (usually not needed)
 * val explicitScalar = NimbleNetTensor(42, shape = intArrayOf())
 * ```
 * 
 * ### Array Data
 * ```kotlin
 * // 1D arrays (vectors)
 * val vector = NimbleNetTensor(
 *     data = floatArrayOf(1.0f, 2.0f, 3.0f, 4.0f),
 *     shape = intArrayOf(4)
 * )
 * 
 * // 2D data represented as 1D array with shape
 * val matrix = NimbleNetTensor(
 *     data = floatArrayOf(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f),
 *     shape = intArrayOf(2, 3) // 2 rows, 3 columns
 * )
 * 
 * // 3D image data (height, width, channels)  
 * val imageData = FloatArray(224 * 224 * 3) // Initialize with image pixels
 * val imageTensor = NimbleNetTensor(
 *     data = imageData,
 *     shape = intArrayOf(224, 224, 3)
 * )
 * 
 * // Batch of images (batch_size, height, width, channels)
 * val batchImageData = FloatArray(32 * 224 * 224 * 3)
 * val batchTensor = NimbleNetTensor(
 *     data = batchImageData,  
 *     shape = intArrayOf(32, 224, 224, 3)
 * )
 * ```
 * 
 * ### JSON Data
 * ```kotlin
 * import org.json.JSONObject
 * import org.json.JSONArray
 * 
 * // Structured configuration data
 * val configData = JSONObject(mapOf(
 *     "model_version" to "1.2.3",
 *     "preprocessing" to mapOf(
 *         "normalize" to true,
 *         "mean" to listOf(0.485, 0.456, 0.406),
 *         "std" to listOf(0.229, 0.224, 0.225)
 *     ),
 *     "inference_params" to mapOf(
 *         "batch_size" to 1,
 *         "temperature" to 0.7
 *     )
 * ))
 * 
 * val configTensor = NimbleNetTensor(
 *     data = configData,
 *     datatype = DATATYPE.JSON
 * )
 * 
 * // List/array data
 * val sequenceData = JSONArray(listOf(
 *     mapOf("token" to "hello", "id" to 101),
 *     mapOf("token" to "world", "id" to 102),
 *     mapOf("token" to "[SEP]", "id" to 103)
 * ))
 * 
 * val sequenceTensor = NimbleNetTensor(
 *     data = sequenceData,
 *     datatype = DATATYPE.JSON_ARRAY
 * )
 * ```
 * 
 * ### Function-Based Tensors
 * ```kotlin
 * // Dynamic computation function
 * val computeFunction: (HashMap<String, NimbleNetTensor>?) -> HashMap<String, NimbleNetTensor>? = { inputs ->
 *     // Custom processing logic
 *     val inputValue = inputs?.get("x")?.data as? Float ?: 0.0f
 *     val result = inputValue * 2.0f + 1.0f
 *     
 *     hashMapOf("output" to NimbleNetTensor(result))
 * }
 * 
 * val functionTensor = NimbleNetTensor(
 *     data = computeFunction,
 *     datatype = DATATYPE.FUNCTION
 * )
 * ```
 * 
 * ### Model Input/Output Example
 * ```kotlin
 * // Prepare inputs for a computer vision model
 * val inputs = hashMapOf(
 *     // Main image input
 *     "image" to NimbleNetTensor(
 *         data = preprocessedImageData, // FloatArray
 *         shape = intArrayOf(1, 224, 224, 3) // NHWC format
 *     ),
 *     
 *     // Model configuration
 *     "config" to NimbleNetTensor(
 *         data = JSONObject(mapOf("threshold" to 0.5, "nms_iou" to 0.3)),
 *         datatype = DATATYPE.JSON
 *     ),
 *     
 *     // Processing flags
 *     "enable_postprocessing" to NimbleNetTensor(true),
 *     "batch_size" to NimbleNetTensor(1)
 * )
 * 
 * // Execute model
 * val result = NimbleNet.runMethod("object_detection", inputs)
 * 
 * // Process outputs
 * if (result.status) {
 *     val outputs = result.payload
 *     val bboxes = outputs?.get("bounding_boxes")?.data as? FloatArray
 *     val scores = outputs?.get("confidence_scores")?.data as? FloatArray
 *     val classes = outputs?.get("class_ids")?.data as? IntArray
 *     
 *     // Use detection results...
 * }
 * ```
 * 
 * ## Memory and Performance
 * 
 * - **Primitive Arrays**: Use native arrays (FloatArray, IntArray) for best performance
 * - **Large Data**: Consider shape information for proper memory layout
 * - **JSON Objects**: Suitable for configuration and metadata, less efficient for large numerical data
 * - **Functions**: Enable dynamic computation but add execution overhead
 * 
 * ## Input vs Output Tensors
 * 
 * - **Input Tensors**: [data] must never be null, shapes should be well-defined
 * - **Output Tensors**: [data] may be null initially, populated by model execution
 * - **Validation**: Input tensors are validated for type consistency and shape compatibility
 * 
 * @property data The actual tensor data. Type varies based on [datatype].
 *              Must be non-null for input tensors, may be null for output tensors initially.
 * @property datatype The data type enum indicating how to interpret the [data] field.
 *                   Automatically set by type-specific constructors.
 * @property shape Optional array describing the dimensions of multi-dimensional data.
 *                Should be null for scalar values, non-null for arrays and matrices.
 * 
 * @see DATATYPE
 * @since 1.0.0
 */
class NimbleNetTensor {
    // NOTE: data is nullable only for outputs, inputs cannot be null
    var data: Any?
    var datatype: DATATYPE
    var shape: IntArray?

    /**
     * Creates a tensor containing a 32-bit integer value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.INT32].
     * Shape is optional and typically null for scalar values.
     * 
     * @param data The integer value to store in the tensor
     * @param shape Optional shape information. Usually null for scalars.
     *             Can be intArrayOf() to explicitly indicate scalar shape.
     * 
     * @since 1.0.0
     */
    constructor(data: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    /**
     * Creates a tensor containing a 64-bit integer (Long) value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.INT64].
     * Useful for large integer values, timestamps, or identifiers.
     * 
     * @param data The long value to store in the tensor
     * @param shape Optional shape information. Usually null for scalars.
     * 
     * @since 1.0.0
     */
    constructor(data: Long, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    /**
     * Creates a tensor containing a 32-bit floating-point value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.FLOAT].
     * Most common for machine learning applications requiring single precision.
     * 
     * @param data The float value to store in the tensor
     * @param shape Optional shape information. Usually null for scalars.
     * 
     * @since 1.0.0
     */
    constructor(data: Float, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    /**
     * Creates a tensor containing a 64-bit floating-point (Double) value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.DOUBLE].
     * Use when higher precision is required for numerical computations.
     * 
     * @param data The double value to store in the tensor
     * @param shape Optional shape information. Usually null for scalars.
     * 
     * @since 1.0.0
     */
    constructor(data: Double, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    /**
     * Creates a tensor containing a boolean value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.BOOL].
     * Useful for flags, conditions, and binary classification results.
     * 
     * @param data The boolean value to store in the tensor
     * @param shape Optional shape information. Usually null for scalars.
     * 
     * @since 1.0.0
     */
    constructor(data: Boolean, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    /**
     * Creates a tensor containing a string value.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.STRING].
     * Suitable for text data, labels, or identifiers.
     * 
     * @param data The string value to store in the tensor
     * @param shape Optional shape information. Usually null for single strings.
     * 
     * @since 1.0.0
     */
    constructor(data: String, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.STRING
        this.shape = shape
    }

    /**
     * Creates a tensor containing an array of 32-bit integers.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.INT32].
     * Efficient for large amounts of integer data such as indices, IDs, or discrete values.
     * 
     * ## Example
     * ```kotlin
     * // 1D array of 5 integers
     * val indices = NimbleNetTensor(
     *     data = intArrayOf(10, 20, 30, 40, 50),
     *     shape = intArrayOf(5)
     * )
     * 
     * // 2D matrix represented as flattened array
     * val matrix = NimbleNetTensor(
     *     data = intArrayOf(1, 2, 3, 4, 5, 6),
     *     shape = intArrayOf(2, 3) // 2 rows, 3 columns
     * )
     * ```
     * 
     * @param data The integer array to store. Must not be null for input tensors.
     * @param shape Optional shape describing the dimensions. 
     *             Should match the total array size (product of dimensions).
     * 
     * @since 1.0.0
     */
    constructor(data: IntArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT32
        this.shape = shape
    }

    /**
     * Creates a tensor containing an array of 64-bit integers (Longs).
     * 
     * This constructor automatically sets the datatype to [DATATYPE.INT64].
     * Use for large integer values, timestamps, or when 32-bit range is insufficient.
     * 
     * @param data The long array to store. Must not be null for input tensors.
     * @param shape Optional shape describing the dimensions.
     * 
     * @since 1.0.0
     */
    constructor(data: LongArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.INT64
        this.shape = shape
    }

    /**
     * Creates a tensor containing an array of 32-bit floating-point values.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.FLOAT].
     * Most commonly used for neural network inputs, weights, and numerical computations.
     * 
     * ## Example
     * ```kotlin
     * // Normalized image data (224x224 grayscale)
     * val imageData = FloatArray(224 * 224) { i -> 
     *     // Initialize with normalized pixel values [0.0, 1.0]
     *     pixelData[i] / 255.0f
     * }
     * val imageTensor = NimbleNetTensor(
     *     data = imageData,
     *     shape = intArrayOf(224, 224)
     * )
     * 
     * // Feature vector for classification
     * val features = NimbleNetTensor(
     *     data = floatArrayOf(0.1f, 0.2f, 0.3f, 0.8f, 0.9f),
     *     shape = intArrayOf(5)
     * )
     * ```
     * 
     * @param data The float array to store. Must not be null for input tensors.
     * @param shape Optional shape describing the dimensions.
     * 
     * @since 1.0.0
     */
    constructor(data: FloatArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.FLOAT
        this.shape = shape
    }

    /**
     * Creates a tensor containing an array of 64-bit floating-point (Double) values.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.DOUBLE].
     * Use when higher precision is required for scientific computations.
     * 
     * @param data The double array to store. Must not be null for input tensors.
     * @param shape Optional shape describing the dimensions.
     * 
     * @since 1.0.0
     */
    constructor(data: DoubleArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.DOUBLE
        this.shape = shape
    }

    /**
     * Creates a tensor containing an array of boolean values.
     * 
     * This constructor automatically sets the datatype to [DATATYPE.BOOL].
     * Useful for masks, binary conditions, or boolean feature vectors.
     * 
     * ## Example
     * ```kotlin
     * // Attention mask for sequence processing
     * val attentionMask = NimbleNetTensor(
     *     data = booleanArrayOf(true, true, true, false, false),
     *     shape = intArrayOf(5) // 5 tokens, first 3 are valid
     * )
     * 
     * // Binary feature flags
     * val featureFlags = NimbleNetTensor(
     *     data = booleanArrayOf(true, false, true, true),
     *     shape = intArrayOf(4)
     * )
     * ```
     * 
     * @param data The boolean array to store. Must not be null for input tensors.
     * @param shape Optional shape describing the dimensions.
     * 
     * @since 1.0.0
     */
    constructor(data: BooleanArray, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.BOOL
        this.shape = shape
    }

    /**
     * Creates a tensor with arbitrary data type specified by integer value.
     * 
     * This is the most flexible constructor, allowing any supported data type.
     * The datatype parameter is converted from integer to [DATATYPE] enum.
     * 
     * ## Supported Integer Values
     * 
     * See [DATATYPE] enum for complete mapping of integers to types:
     * - 1: FLOAT (32-bit floating point)
     * - 6: INT32 (32-bit integers)  
     * - 7: INT64 (64-bit integers)
     * - 8: STRING (text data)
     * - 9: BOOL (boolean values)
     * - 11: DOUBLE (64-bit floating point)
     * - 670: JSON (JSON objects)
     * - 681: JSON_ARRAY (JSON arrays)
     * - 682: FUNCTION (Kotlin functions)
     * - 700: FE_OBJ (Protocol Buffers)
     * 
     * @param data The data to store. Type should match the specified datatype.
     * @param datatypeInt Integer value corresponding to desired [DATATYPE] enum value.
     * @param shape Optional shape describing the dimensions.
     * 
     * @see DATATYPE.fromInt
     * @since 1.0.0
     */
    constructor(data: Any?, datatypeInt: Int, shape: IntArray? = null) {
        this.data = data
        this.datatype = DATATYPE.fromInt(datatypeInt)
        this.shape = shape
    }

    /**
     * Creates a tensor with arbitrary data type specified by [DATATYPE] enum.
     * 
     * This constructor provides explicit type control while maintaining type safety.
     * Recommended over the integer-based constructor for better code clarity.
     * 
     * ## Example Usage
     * ```kotlin
     * import org.json.JSONObject
     * 
     * // JSON configuration data
     * val config = JSONObject(mapOf(
     *     "threshold" to 0.5,
     *     "algorithm" to "fast_rcnn"
     * ))
     * val configTensor = NimbleNetTensor(
     *     data = config,
     *     datatype = DATATYPE.JSON
     * )
     * 
     * // Function for dynamic computation
     * val processingFunction: (Map<String, Any>?) -> Map<String, Any> = { input ->
     *     // Custom processing logic
     *     mapOf("processed" to true)
     * }
     * val functionTensor = NimbleNetTensor(
     *     data = processingFunction,
     *     datatype = DATATYPE.FUNCTION
     * )
     * ```
     * 
     * @param data The data to store. Type should match the specified datatype.
     * @param datatype The [DATATYPE] enum value specifying how to interpret the data.
     * @param shape Optional shape describing the dimensions.
     * 
     * @since 1.0.0
     */
    constructor(data: Any?, datatype: DATATYPE, shape: IntArray? = null) {
        this.data = data
        this.datatype = datatype
        this.shape = shape
    }

    /**
     * Returns the integer representation of the tensor's data type.
     * 
     * This method provides the numeric value corresponding to the [datatype] enum,
     * which is useful for interoperability with native code or serialization.
     * 
     * @return Integer value corresponding to [datatype] enum
     * @see DATATYPE.value
     * @since 1.0.0
     */
    fun getDatatypeInt(): Int {
        return datatype.value
    }

    /**
     * Returns the number of dimensions in the tensor's shape.
     * 
     * This method provides a convenient way to check tensor dimensionality:
     * - 0: Scalar (no shape or empty shape)
     * - 1: Vector (1D array)
     * - 2: Matrix (2D array)  
     * - 3+: Higher-dimensional tensor
     * 
     * ## Example
     * ```kotlin
     * val scalar = NimbleNetTensor(42)
     * println(scalar.getShapeArrayLength()) // 0
     * 
     * val vector = NimbleNetTensor(floatArrayOf(1f, 2f, 3f), intArrayOf(3))
     * println(vector.getShapeArrayLength()) // 1
     * 
     * val matrix = NimbleNetTensor(floatArrayOf(1f, 2f, 3f, 4f), intArrayOf(2, 2))
     * println(matrix.getShapeArrayLength()) // 2
     * ```
     * 
     * @return Number of dimensions, or 0 if shape is null/empty
     * @since 1.0.0
     */
    fun getShapeArrayLength(): Int {
        return shape?.size ?: 0
    }
}
