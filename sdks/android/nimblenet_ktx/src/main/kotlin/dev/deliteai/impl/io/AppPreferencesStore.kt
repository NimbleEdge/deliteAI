/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.io

import android.content.SharedPreferences

internal class AppPreferencesStore(private val sharedPreferences: SharedPreferences) {
    fun get(key: String): String? = sharedPreferences.getString(key, null)

    fun put(key: String, value: String) = sharedPreferences.edit().putString(key, value).apply()

    fun delete(key: String) = sharedPreferences.edit().remove(key).apply()
}
