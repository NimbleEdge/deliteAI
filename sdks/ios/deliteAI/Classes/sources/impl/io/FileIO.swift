/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

@objc public class FileIO:NSObject {
    @objc public static let shared = FileIO()

    @objc public func performDiskCleanup(
        commonFolderName: String,
        tagFolderName: String
    ) -> Bool {
        do {
            let nimbleSdkFolder = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0].appendingPathComponent("nimbleSDK")
            let commonFolderAbsolutePath = URL(fileURLWithPath: commonFolderName).path
            let tagFolderNameAbsolutePath = URL(fileURLWithPath: tagFolderName).path
            let nimbleSdkAbsolutePath = nimbleSdkFolder.path

            try FileManager.default.enumerator(at: nimbleSdkFolder, includingPropertiesForKeys: nil)?.forEach { url in
                guard let fileURL = url as? URL else { return }
                let fileAbsolutePath = fileURL.path

                if fileAbsolutePath == nimbleSdkAbsolutePath || fileAbsolutePath.contains(commonFolderAbsolutePath) || fileAbsolutePath.contains(tagFolderNameAbsolutePath) {
                    return
                }
                try FileManager.default.removeItem(at: fileURL)
            }

            return true

        } catch {
            return false
        }
    }
    
    let DELITE_ASSETS_TEMP_STORAGE = "deliteAssets"
    let DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS: Int64 = 7 * 24 * 60 * 60 * 1000  // 7 days

    public func processModules(assetsJson: inout [[String: Any]]) throws -> [[String: Any]] {
        let nimbleSdkFolder = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0].appendingPathComponent("nimbleSDK")
        let targetDir = nimbleSdkFolder.appendingPathComponent(DELITE_ASSETS_TEMP_STORAGE)
        try FileManager.default.createDirectory(at: targetDir, withIntermediateDirectories: true)

        var retainedFiles = Set<String>()

        func processModuleObject(_ module: inout [String: Any]) throws {
            if var location = module["location"] as? [String: Any],
               let assetPath = location["path"] as? String {

                let fileExt = URL(fileURLWithPath: assetPath).pathExtension
                guard let name = module["name"] as? String,
                      let version = module["version"] as? String else {
                    throw NSError(domain: "Missing required fields", code: 1)
                }

                let fileName = fileExt.isEmpty
                    ? "\(name)_\(version)"
                    : "\(name)_\(version).\(fileExt)"

                retainedFiles.insert(fileName)

                let outputFile = targetDir.appendingPathComponent(fileName)

                // Copy if not exists
                if !FileManager.default.fileExists(atPath: outputFile.path) {
                    guard let inputUrl = Bundle.main.url(forResource: assetPath, withExtension: nil) else {
                        throw NSError(domain: "Asset not found: \(assetPath)", code: 2)
                    }
                    let data = try Data(contentsOf: inputUrl)
                    try data.write(to: outputFile)
                }

                // Update path in JSON
                location["path"] = outputFile.path
                module["location"] = location
            }

            // Recursively process nested arguments
            if var arguments = module["arguments"] as? [[String: Any]] {
                for i in 0..<arguments.count {
                    var arg = arguments[i]
                    try processModuleObject(&arg)
                    arguments[i] = arg
                }
                module["arguments"] = arguments
            }
        }

        for i in 0..<assetsJson.count {
            var module = assetsJson[i]
            try processModuleObject(&module)
            assetsJson[i] = module
        }

        // Delete old unused files
        let now = Date().timeIntervalSince1970 * 1000
        if let files = try? FileManager.default.contentsOfDirectory(at: targetDir, includingPropertiesForKeys: [.contentAccessDateKey]) {
            for file in files {
                if !retainedFiles.contains(file.lastPathComponent) {
                    let resourceValues = try? file.resourceValues(forKeys: [.contentAccessDateKey])
                    let lastAccess = resourceValues?.contentAccessDate?.timeIntervalSince1970 ?? 0
                    if (now - lastAccess * 1000) > Double(DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS) {
                        try? FileManager.default.removeItem(at: file)
                    }
                }
            }
        }

        return assetsJson
    }
}
