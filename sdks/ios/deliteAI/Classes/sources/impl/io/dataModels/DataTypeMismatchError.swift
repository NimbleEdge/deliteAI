/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

enum InvalidInputError: Int {
    case inputDataMismatch = 12001
    case errorTranformError = 12002
}

class DataTypeMismatchError: Error, CustomStringConvertible {
    let message: String

    init(message: String) {
        self.message = message
    }

    var description: String {
        return message
    }

    static func arrayTypeMismatch(expected: DataType) -> DataTypeMismatchError {
        return DataTypeMismatchError(message: "Array data type mismatch. Expected \(expected).")
    }

    static func singularTypeMismatch(expected: DataType) -> DataTypeMismatchError {
        return DataTypeMismatchError(message: "Singular data type mismatch. Expected \(expected).")
    }

    static func unsupportedDataType(type: String) -> DataTypeMismatchError {
        return DataTypeMismatchError(message: "Unsupported data type: \(type).")
    }

    static var invalidShapeArray: DataTypeMismatchError {
        return DataTypeMismatchError(message: "Invalid shape for array.")
    }

    static var invalidShapeSingular: DataTypeMismatchError {
        return DataTypeMismatchError(message: "Invalid shape for singular value.")
    }
}
