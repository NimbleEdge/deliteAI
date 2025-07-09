/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

public struct NimbleNetConfig: Codable {
    public let clientId: String
    public let clientSecret: String
    public let host: String
    public let deviceId: String
    public let debug: Bool
    public var compatibilityTag: String
    public var sessionId: String
    public var maxDBSizeKBs: Float?
    public var maxEventsSizeKBs: Float?
    public var cohortIds: [String]
    public var online: Bool
    public init(clientId: String = "",
                clientSecret: String = "",
                host: String = "",
                deviceId: String = "",
                debug: Bool = false,
                compatibilityTag:String = "",
                sessionId: String = "",
                maxDBSizeKBs: Float? = nil,
                maxEventsSizeKBs: Float? = nil,
                cohortIds: [String] = [],
                online: Bool = false) {
        self.clientId = clientId
        self.clientSecret = clientSecret
        self.host = host
        self.deviceId = deviceId
        self.debug = debug
        self.compatibilityTag = compatibilityTag
        self.sessionId = sessionId
        self.maxDBSizeKBs = maxDBSizeKBs
        self.maxEventsSizeKBs = maxEventsSizeKBs
        self.cohortIds = cohortIds
        self.online = online
    }
}
