/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

struct Constants {
    static func currentTimeInMicroseconds() -> Int64 {
        return Int64(Date().timeIntervalSince1970 * 1_000_000)
    }
    
    static func convertToJSONBody(from jsonStr: String) -> [String: Any]? {
        if let data = jsonStr.data(using: .utf8) {
            do {
                return try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]
            } catch {
                print(error.localizedDescription)
            }
        }
        return nil
    }
    
    static func convertToObject(from jsonStr: String) -> [Dictionary<String, String>]? {
        guard let data = jsonStr.data(using: .utf8) else {
            return nil
        }

        do {
            if let jsonArray = try JSONSerialization.jsonObject(with: data, options: []) as? [Dictionary<String, String>] {
                return jsonArray
            } else {
                print("bad json")
                return nil
            }
        } catch let error as NSError {
            print("JSONSerialization error: \(error.localizedDescription)")
            return nil
        }
    }
}


