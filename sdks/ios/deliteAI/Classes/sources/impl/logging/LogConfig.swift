/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 

import Foundation

struct LogConfig: Decodable {
    var APIKey: String = "abcd" // Set by calling InitLogger function
    var url: URL = URL(string: "https://http-intake.logs.datadoghq.com/api/v2/logs")!
    var source: String = "iOS"
    var tags: String = "env:oyopoc,version:1.0"
    var service: String = "mobilesdk"
    var dateFormat: String = "yyyy-MM-dd hh:mm:ss"
    var logFileName: String = "logs.txt"
}
