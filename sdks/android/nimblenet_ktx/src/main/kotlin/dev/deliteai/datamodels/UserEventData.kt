/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

data class UserEventData(var eventType: String? = null, var eventJsonString: String? = null) {
    override fun toString(): String {
        return "eventType: $eventType \n eventJsonString: $eventJsonString"
    }
}
