/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

struct DownloadItem: Codable {
    let requestId: Int64
    var prevStatus: Int
    var prevStatusTime: Int64
    var currStatus: Int
    
   static func convertToJSONString(from info: DownloadItem) -> String? {
        let wrapper = DownloadItem(
            requestId: Int64(info.requestId),
            prevStatus: Int(info.prevStatus),
            prevStatusTime: Constants.currentTimeInMicroseconds(),
            currStatus: Int(info.currStatus)
        )
        
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted
        
        if let jsonData = try? encoder.encode(wrapper),
           let jsonString = String(data: jsonData, encoding: .utf8) {
            return jsonString
        }
        return nil
    }
    
    static func convertJSONStringToDownloadItem(jsonString: String) -> DownloadItem? {
        let decoder = JSONDecoder()
        guard let jsonData = jsonString.data(using: .utf8),
              let wrapper = try? decoder.decode(DownloadItem.self, from: jsonData) else {
            return nil
        }
        return wrapper
    }

}
