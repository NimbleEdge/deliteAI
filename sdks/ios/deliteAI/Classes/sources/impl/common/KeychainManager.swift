/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Security

class KeychainManager {
    static func save(appBundleIdentifier: String, appName: String, password: String)-> Bool {
        let data = password.data(using: .utf8)!
        
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: appBundleIdentifier,
            kSecAttrAccount as String: appName,
            kSecValueData as String: data
        ]
        
        let status = SecItemAdd(query as CFDictionary, nil)
        if(status != errSecSuccess){
            return false
        }
        return true
    }
    
    static func retrieve(appBundleIdentifier: String, appName: String) throws -> String? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrService as String: appBundleIdentifier,
            kSecAttrAccount as String: appName,
            kSecReturnData as String: kCFBooleanTrue!,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]
        
        var dataTypeRef: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &dataTypeRef)
        // returns nil if item not found
        guard status != errSecItemNotFound else {
            return nil
        }
        //throws error in case of I/O error,disk full etc
        guard status == errSecSuccess, let data = dataTypeRef as? Data, let result = String(data: data, encoding: .utf8) else {
            throw KeychainError.unexpectedError(status)
        }
        
        return result
    }
    
}

enum KeychainError: Error {
    case unexpectedError(OSStatus)
}
