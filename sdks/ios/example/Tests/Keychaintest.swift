/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import XCTest
import Foundation
@testable import DeliteAI

final class Keychaintest: XCTestCase {
    let appBundleIdentifier = "com.example.testappBundleIdentifier"
    let appName = "example"
    let password = "securePassword123"
    let newPassword = "newSecurePassword456"
    
    override func setUpWithError() throws {
        delete(appBundleIdentifier: appBundleIdentifier, appName: appName)
    }
     func delete(appBundleIdentifier: String, appName: String) {
         let query: [String: Any] = [
             kSecClass as String: kSecClassGenericPassword,
             kSecAttrService as String: appBundleIdentifier,
             kSecAttrAccount as String: appName
         ]
         
        
        SecItemDelete(query as CFDictionary)
    }
    
    func testKeychainPersistsAcrossMultipleInstalls() {
        guard let appName = Bundle.main.infoDictionary?["CFBundleName"] as? String else {
            XCTFail("Failed to retrieve CFBundleName from infoDictionary")
            return
        }
        
        guard let appBundleIdentifier = Bundle.main.bundleIdentifier else {
            XCTFail("Failed to retrieve bundle identifier")
            return
        }
        
        guard let deviceUUID = UIDevice.current.identifierForVendor?.uuidString else {
            XCTFail("Failed to retrieve device UUID")
            return
        }
        var storedUUID: String? = nil
        do{
          storedUUID = try KeychainManager.retrieve(appBundleIdentifier: appName, appName: appBundleIdentifier) ?? deviceUUID
        }
        catch{
            XCTFail("Failed to retrieve device UUID")
        }
        guard let storedUUID = storedUUID else { 
            XCTFail("no device UUID present")
            return
        }
        
        // this should be same for a device across multiple installs
        let expectedUUID = "07AA2ADA-17AC-4F2C-83DA-33B94287D7BA" // Modify this as needed
        

        XCTAssertEqual(storedUUID, expectedUUID, "Stored UUID does not match expected UUID")
    }

    
    func testSaveAndRetrievePassword() {

        KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: password)
        var retrievedPassword: String?
        do{
          retrievedPassword = try KeychainManager.retrieve(appBundleIdentifier: appBundleIdentifier, appName: appName)
        }
        catch{
            XCTFail("Failed to retrieve device UUID")
        }
        XCTAssertEqual(retrievedPassword, password, "Retrieved password does not match the saved password.")
    }
    
    func testRetrieveNonExistentPassword() {
        var retrievedPassword: String?
        do{
           retrievedPassword = try KeychainManager.retrieve(appBundleIdentifier: appBundleIdentifier, appName: appName)
        }
        catch{
            XCTFail("Failed to retrieve device UUID")
        }
        XCTAssertNil(retrievedPassword, "Retrieving a non-existent password should return nil.")
    }
    
    func testDontOverwriteSavedPassword() {
        KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: password)
        
        KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: newPassword)
        
        var retrievedPassword: String?
        do{
            retrievedPassword = try KeychainManager.retrieve(appBundleIdentifier: appBundleIdentifier, appName: appName)
        }
        catch{
            XCTFail("Failed to retrieve device UUID")
        }
        
        XCTAssertEqual(retrievedPassword, password, "password should not be over written.")
    }
    func testOverwritingSavedPasswordreturnFalse() {
        let status = KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: password)
        
        let resaveStatus = KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: newPassword)
        
        XCTAssertTrue(status)
        XCTAssertFalse(resaveStatus)
    }
    
    func testSavePassword() {
       let status = KeychainManager.save(appBundleIdentifier: appBundleIdentifier, appName: appName, password: password)
       XCTAssertTrue(status)
    }


}
