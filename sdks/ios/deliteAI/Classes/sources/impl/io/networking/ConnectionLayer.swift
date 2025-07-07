/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import Alamofire


enum IOSErrorCodes: Int {
    case fileRenameError = 11001
    case fileDownloadError = 11002
}

enum SwiftError: Error {
    case runtimeError(String)
}

@objc public class ConnectionLayer: NSObject {
    
    @objc public static let shared = ConnectionLayer()
    
    
    private var startTime: Date?
    private let userDefaults = UserDefaults.standard
    
    private override init() {
        super.init()
    }
    
    @objc public func downloadFile(from url: String, to destinationPath: String, method: String, headers: String, fileName: String) -> FileDownloadInfo {
        var downloadManager = DownloadManager.shared
        let temporaryPath = destinationPath + ".part"
        let destinationURL = URL(fileURLWithPath: destinationPath)
        let temporaryURL = URL(fileURLWithPath: temporaryPath)
        var downloadItem: DownloadItem
        
        guard let urlString = URL(string: url) else {
            return downloadManager.emptyFileDownloadInfo()
        }
        
        // Handle existing download requests
        if var existingDownloadItem = downloadManager.getDownloadItem(for: url) {
            downloadItem = existingDownloadItem
            
            if downloadItem.currStatus == Int(DOWNLOAD_FAILURE.rawValue) {
                downloadManager.deleteDownloadItem(for: url)
                return downloadManager.convertDownloadItemToFileDownloadInfo(
                    downloadItem: downloadItem,
                    currentStatusReasonCode: Int32(IOSErrorCodes.fileDownloadError.rawValue)
                )
            }
            
            if downloadItem.currStatus == Int(DOWNLOAD_SUCCESS.rawValue) {
                downloadManager.deleteDownloadItem(for: url)
                do {
                    _ = try FileManager.default.replaceItemAt(destinationURL, withItemAt: temporaryURL)
                    return downloadManager.convertDownloadItemToFileDownloadInfo(downloadItem: downloadItem, currentStatusReasonCode: -1)
                } catch {
                    downloadItem.currStatus = Int(DOWNLOAD_FAILURE.rawValue)
                    downloadItem.prevStatus = Int(DOWNLOAD_SUCCESS.rawValue)
                    return downloadManager.convertDownloadItemToFileDownloadInfo(
                        downloadItem: downloadItem,
                        currentStatusReasonCode: Int32(IOSErrorCodes.fileRenameError.rawValue)
                    )
                }
            }
            
            if (!downloadManager.isDownloadActive(for: url)) {
                downloadManager.insertinActiveDownload(for: url)
                downloadManager.startOrResumeDownload(url: url, temporaryPath: temporaryPath, headers: headers, method: method)
            }
            
            if downloadItem.prevStatus != downloadItem.currStatus {
                downloadItem = downloadManager.registerDownloadStateChange(downloadItem: downloadItem, newCurrentState: Int(DOWNLOAD_RUNNING.rawValue), url: url)
                
                return downloadManager.convertDownloadItemToFileDownloadInfo(downloadItem: downloadItem, currentStatusReasonCode: -1)
            }
            return downloadManager.convertDownloadItemToFileDownloadInfo(downloadItem: downloadItem, currentStatusReasonCode: -1)
        }
        else{
            let downloadItem = DownloadItem(
                requestId: downloadManager.generateDownloadId(),
                prevStatus: Int(DOWNLOAD_UNKNOWN.rawValue),
                prevStatusTime: Constants.currentTimeInMicroseconds(),
                currStatus: Int(DOWNLOAD_RUNNING.rawValue)
            )
            
            downloadManager.setDownloadItem(for: url, downloadItem: downloadItem)
            
            return downloadManager.convertDownloadItemToFileDownloadInfo(downloadItem: downloadItem, currentStatusReasonCode: -1)
        }
    }
    
    
    @objc public func sendRequest(url: String, reqBody: String, reqHeaders: String, method: String, length:Int) -> CNetworkResponse {
        
        guard let urlString = URL(string: url) else {
            return CNetworkResponse()
        }
        var request = URLRequest(url: urlString, cachePolicy: .useProtocolCachePolicy, timeoutInterval: 10.0)
        request.httpMethod = method
        
        request.httpBody = reqBody.data(using: .utf8)
        
        request.headers = [:]
        if let requestHeadersArray = Constants.convertToObject(from: reqHeaders) {
            for requestHeader in requestHeadersArray {
                for header in requestHeader {
                    request.setValue(header.value, forHTTPHeaderField: header.key)
                }
            }
        }
        
        var statusCode: Int32 = 0
        var headers: UnsafeMutablePointer<CChar>? = nil
        var body: UnsafeMutablePointer<UInt8>? = nil
        var bodyLength: Int32 = 0
        
        let sem = DispatchSemaphore(value: 0)
        let task = URLSession.shared.dataTask(with: request) { data, response, error in
            
            defer {sem.signal()}
            
            guard error == nil else {
                return
            }
            
            if let httpResponse = response as? HTTPURLResponse {
                statusCode = Int32(httpResponse.statusCode)
                
                let headerDictionary = httpResponse.allHeaderFields.reduce(into: [String: String]()) { dict, item in
                    if let key = item.key as? String, let value = item.value as? String {
                        dict[key] = value
                    }
                }
                
                do {
                    let headerData = try JSONSerialization.data(withJSONObject: headerDictionary, options: [])
                    if let headerString = String(data: headerData, encoding: .utf8) {
                        headers = strdup(headerString)
                    }
                } catch {
                    print("Error serializing headers to JSON: \(error)")
                }
            }
            
            guard let data = data else {
                return
            }
            let bytes = [UInt8](data)
            let responseDataCount = data.count
            
            body = malloc(bytes.count * MemoryLayout<UInt8>.stride).bindMemory(to: UInt8.self, capacity: bytes.count)
            body?.initialize(from: bytes, count: responseDataCount)
            
            
            bodyLength = Int32(data.count)
            
        }
        task.resume()
        sem.wait()
        
        var bodyCCharPointer: UnsafeMutablePointer<CChar>?
        body?.withMemoryRebound(to: CChar.self, capacity: Int(bodyLength)) { cCharBuffer in
            // Inside the closure, cCharBuffer is the buffer with the correct type
            // Assign it to the cCharPointer for use outside the closure
            bodyCCharPointer = cCharBuffer
        }
        
        
        let response = CNetworkResponse(statusCode: statusCode, headers: headers, body: bodyCCharPointer, bodyLength: bodyLength)
        
        return response
        
    }
}

