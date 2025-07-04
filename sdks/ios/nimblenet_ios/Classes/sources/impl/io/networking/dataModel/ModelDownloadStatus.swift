/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
 
import Foundation

@objc
public enum ModelDownloadStatus: Int {
    case DOES_NOT_EXIST = 10000
    case PENDING = 10001
    case SUCCESS = 10002
    case FAILURE = 10003
    
    var value: Int {
        return self.rawValue
    }
}
