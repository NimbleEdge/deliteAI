/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import SwiftProtobuf

/// Configuration class for initializing the NimbleNet SDK.
///
/// This class contains all the necessary parameters and settings required to initialize and
/// configure the NimbleNet SDK. It includes authentication credentials, server endpoints, resource
/// limits, and deployment options.
public struct NimbleNetConfig: Codable {
    
    /// Your unique client identifier from NimbleNet platform. Must be a non-empty
    /// string obtained from your NimbleNet account.
    public let clientId: String
    
    /// Authentication secret key for API access. Keep this value secure and never
    /// expose it in logs or client-side code.
    public let clientSecret: String
    
    /// The base URL of the NimbleNet platform API endpoint. Must be a valid HTTPS URL (e.g.,
    /// "https://api.nimblenet.ai").
    public let host: String
    
    /// Unique identifier for this device or application installation. Can be a UUID,
    /// device fingerprint, or custom identifier. Should remain consistent across app sessions for the
    /// same device.
    public let deviceId: String
    
    /// Enable debug mode for detailed logging and diagnostics. Set to `false` in production
    /// builds for optimal performance.
    public let debug: Bool
    
    /// Version identifier for API compatibility checking. Should match your
    /// application version or SDK compatibility version. Used to ensure client-server API
    /// compatibility.
    public var compatibilityTag: String
    
    /// Optional custom session identifier for session management. If empty, the SDK
    /// will generate a default session ID. Useful for multi-user applications or session persistence.
    public var sessionId: String
    
    /// Optional maximum database size limit in kilobytes. When reached, older data
    /// may be purged to stay within limits. Set based on your device storage constraints.
    public var maxDBSizeKBs: Float?
    
    /// Optional maximum event data storage limit in kilobytes. Controls how much
    /// event data can be cached before upload.
    public var maxEventsSizeKBs: Float?
    
    /// Array of cohort identifiers for A/B testing and experimentation. Used to assign
    /// users to specific test groups or feature variants.
    public var cohortIds: [String]
    
    /// Whether the assets will be downloaded from cloud or they are
    /// already bundled with the app.
    public var online: Bool
    
    /// Initializes a `NimbleNetConfig` instance.
    /// - Parameters:
    ///   - clientId: Your unique client identifier from NimbleNet platform. Defaults to an empty string.
    ///   - clientSecret: Authentication secret key for API access. Defaults to an empty string.
    ///   - host: The base URL of the SaaS Configuration platform API endpoint. Defaults to an empty string.
    ///   - deviceId: Unique identifier for this device or application installation. Defaults to an empty string.
    ///   - debug: Enable debug mode for detailed logging and diagnostics. Defaults to `false`.
    ///   - compatibilityTag: Version identifier for API compatibility checking. Defaults to an empty string.
    ///   - sessionId: Optional custom session identifier for session management. Defaults to an empty string.
    ///   - maxDBSizeKBs: Optional maximum database size limit in kilobytes. Defaults to `nil`.
    ///   - maxEventsSizeKBs: Optional maximum event data storage limit in kilobytes. Defaults to `nil`.
    ///   - cohortIds: Array of cohort identifiers for A/B testing and experimentation. Defaults to an empty array.
    ///   - online: Whether the assets will be downloaded from cloud or they are already bundled with the app. Defaults to `false`.
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
