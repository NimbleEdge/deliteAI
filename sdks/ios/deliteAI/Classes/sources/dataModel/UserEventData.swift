/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

public class UserEventdata{
    
    /// The serialized JSON representation of the event data.
    public var eventDataJSONString: String?
    
    /// The classification or category of the recorded event.
    public var eventType:String?

    /// - Parameters:
    ///   - eventDataJSONString: The serialized JSON string of the event data. Defaults to `nil`.
    ///   - eventType: The classification or category of the recorded event. Defaults to `nil`.
    public init(eventDataJSONString: String? = nil, eventType: String? = nil) {
        self.eventDataJSONString = eventDataJSONString
        self.eventType = eventType
    }
}
