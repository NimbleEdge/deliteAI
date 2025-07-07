/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation
import Alamofire

class DownloadManager {
    static let shared = DownloadManager()
    private var activeDownloads: Set<String> = []

    
    // MARK: - Public Methods
    
    func isDownloadActive(for url: String) -> Bool {
        return activeDownloads.contains(url)
    }
    
    func insertinActiveDownload(for url: String){
        activeDownloads.insert(url)
    }
    
    func removeActiveDownload(for url: String){
        activeDownloads.remove(url)
    }
    
    func startOrResumeDownload(url: String, temporaryPath: String, headers: String, method: String) {
        let fileSize = getDownloadedFileSize(at: temporaryPath)
        let afHeaders = prepareHeaders(fileSize: fileSize, headers: headers)
        
        let request = AF.streamRequest(url, method: HTTPMethod(rawValue: method), headers: afHeaders)
        // This creates a dispatch queue that executes tasks serially.
        // See https://developer.apple.com/documentation/dispatch/dispatchqueue/init(label:qos:attributes:autoreleasefrequency:target:)
         let backgroundQueue = DispatchQueue(label: "com.nimblenet.asyncDownloadQueue", qos: .background)
        request.responseStream(on: backgroundQueue) { stream in
            var currentStatus: FileDownloadStatus = DOWNLOAD_RUNNING
            
            defer {
                    if(currentStatus != DOWNLOAD_RUNNING){
                      if let dataItem = self.getDownloadItem(for: url) {
                        self.registerDownloadStateChange(downloadItem: dataItem, newCurrentState: Int(currentStatus.rawValue), url: url)
                      }
                      self.removeActiveDownload(for: url)
                    }
                  }
            
            switch stream.event {
            case .stream(let result):
                if let data = try? result.get() {
                    self.appendDataToFile(data, at: temporaryPath)
                } else {
                    currentStatus = DOWNLOAD_FAILURE
                }
            case .complete(let completion):
                if let error = completion.error {
                    currentStatus = DOWNLOAD_FAILURE
                } else {
                    if let statusCode = completion.response?.statusCode, (statusCode == 200 || statusCode == 206) {
                        currentStatus = DOWNLOAD_SUCCESS
                    } else {
                        currentStatus = DOWNLOAD_FAILURE
                    }
                }
            }
        }
    }
    
    func generateDownloadId() -> Int64 {
        return Int64(UUID().hashValue)
    }
    
    func getDownloadItem(for url: String) -> DownloadItem? {
        guard let jsonString = UserDefaults.standard.string(forKey: url) else { return nil }
        return DownloadItem.convertJSONStringToDownloadItem(jsonString: jsonString)
    }
    
    func setDownloadItem(for url: String, downloadItem: DownloadItem) {
        let jsonString = DownloadItem.convertToJSONString(from: downloadItem)
        UserDefaults.standard.set(jsonString, forKey: url)
        UserDefaults.standard.synchronize()
    }
    
    func deleteDownloadItem(for url: String) {
        UserDefaults.standard.removeObject(forKey: url)
        UserDefaults.standard.synchronize()
    }
    
    func convertDownloadItemToFileDownloadInfo(downloadItem: DownloadItem, currentStatusReasonCode: Int32) -> FileDownloadInfo {
        let elapsedTime = (downloadItem.currStatus != downloadItem.prevStatus)
        ? Constants.currentTimeInMicroseconds() - downloadItem.prevStatusTime
        : 0
        
        return FileDownloadInfo(
            requestId: Int(downloadItem.requestId),
            prevStatus: FileDownloadStatus(rawValue: UInt32(downloadItem.prevStatus)),
            currentStatus: FileDownloadStatus(rawValue: UInt32(downloadItem.currStatus)),
            timeElapsedInMicro: Int(elapsedTime),
            currentStatusReasonCode: currentStatusReasonCode
        )
    }
    
    func emptyFileDownloadInfo() -> FileDownloadInfo {
        return FileDownloadInfo(
            requestId: 0,
            prevStatus: DOWNLOAD_PENDING,
            currentStatus: DOWNLOAD_FAILURE,
            timeElapsedInMicro: 0,
            currentStatusReasonCode: -1
        )
    }
    
    
    func registerDownloadStateChange(downloadItem: DownloadItem, newCurrentState: Int, url: String)-> DownloadItem {
        let updatedItem = downloadItem.updatedWithNewState(newCurrentState: newCurrentState)
        setDownloadItem(for: url, downloadItem: updatedItem)
        return updatedItem
    }
    
    // MARK: - Private Methods
    
    private func getDownloadedFileSize(at path: String) -> Int64 {
        do {
            let attributes = try FileManager.default.attributesOfItem(atPath: path)
            return attributes[.size] as? Int64 ?? 0
        } catch {
            return 0
        }
    }
    
    private func appendDataToFile(_ data: Data, at path: String) {
        let fileURL = URL(fileURLWithPath: path)
        do {
            if FileManager.default.fileExists(atPath: path) {
                let fileHandle = try FileHandle(forWritingTo: fileURL)
                defer { fileHandle.closeFile() }
                fileHandle.seekToEndOfFile()
                fileHandle.write(data)
            } else {
                try data.write(to: fileURL)
            }
        } catch {
            print("Failed to write data to \(path): \(error.localizedDescription)")
        }
    }
    
    private func prepareHeaders(fileSize: Int64, headers: String) -> HTTPHeaders {
        var afHeaders: HTTPHeaders = fileSize > 0 ? ["Range": "bytes=\(fileSize)-"] : [:]
        
        if let headerArray = Constants.convertToObject(from: headers) {
            for requestHeader in headerArray {
                for header in requestHeader {
                    afHeaders[header.key] = header.value
                }
            }
        }
        
        return afHeaders
    }
    
}

// MARK: - DownloadItem Extension

extension DownloadItem {
    func updatedWithNewState(newCurrentState: Int) -> DownloadItem {
        return DownloadItem(
            requestId: self.requestId,
            prevStatus: self.currStatus,
            prevStatusTime: Constants.currentTimeInMicroseconds(),
            currStatus: newCurrentState
        )
    }
}
