/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

public struct DatabaseTable: Codable {
    public let tableName: String
    public let schema: [String: String]
    public let expiryInMins: Int
    
    public init(tableName: String, schema: [String : String], expiryInMins: Int) {
        self.tableName = tableName
        self.schema = schema
        self.expiryInMins = expiryInMins
    }
}
