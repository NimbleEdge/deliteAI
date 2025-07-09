/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

public struct NimbleNetTensor {
    public var data: Any
    public var datatype: DataType
    public var shape: [Int]? //shape as null means singular input

    public init(data: Any, datatype: DataType, shape: [Int]? = []) {
        self.data = data
        self.datatype = datatype
        self.shape = shape
    }
}
