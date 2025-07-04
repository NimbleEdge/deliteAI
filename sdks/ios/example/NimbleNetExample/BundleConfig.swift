/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import Foundation

class BundleConfig {
    public static var clientSecret: String = Bundle.main.infoDictionary?["CLIENT_SECERT"] as? String ?? ""
    public static var host: String = Bundle.main.infoDictionary?["HOST_URL"] as? String ?? ""
}
