/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

public class UserEventdata{
    init(eventDataJSONString: String? = nil, eventType: String? = nil) {
        self.eventDataJSONString = eventDataJSONString
        self.eventType = eventType
    }
    public var eventDataJSONString: String?
    public var eventType:String?
}
