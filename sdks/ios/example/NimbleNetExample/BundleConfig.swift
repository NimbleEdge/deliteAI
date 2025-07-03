/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

class BundleConfig {
    public static var clientSecret = Bundle.main.infoDictionary?["CLIENT_SECERT"] ?? ""
    public static var host = Bundle.main.infoDictionary?["HOST_URL"] ?? ""
}
