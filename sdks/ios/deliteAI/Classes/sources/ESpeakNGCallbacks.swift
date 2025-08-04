/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

@objc public class EspeakNGCallbacks: NSObject {
    @objc public static var textToPhonemes: ((String) -> String)?
}
