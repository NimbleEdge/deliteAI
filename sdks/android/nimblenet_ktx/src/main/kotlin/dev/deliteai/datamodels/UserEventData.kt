/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.datamodels

/**
 * Data container for user event information returned by the NimbleNet SDK.
 *
 * This data class encapsulates information about events that have been processed by the NimbleNet
 * platform. It contains the event type and the serialized event data.
 *
 * ## Usage Example
 *
 * ```kotlin
 * val eventData = mapOf(
 *     "action" to "button_click",
 *     "screen" to "home"
 * )
 *
 * val result = NimbleNet.addEvent(eventData, "user_interaction")
 *
 * if (result.status) {
 *     val userEventData = result.payload
 *     println("Event type: ${userEventData?.eventType}")
 *     println("Event data: ${userEventData?.eventJsonString}")
 * } else {
 *     Log.e("Events", "Failed to record event: ${result.error?.message}")
 * }
 * ```
 *
 * @param eventType The classification or category of the recorded event.
 * @param eventJsonString The serialized JSON representation of the event data.
 * @see NimbleNet.addEvent
 * @see NimbleNetResult
 * @since 1.0.0
 */
data class UserEventData(var eventType: String? = null, var eventJsonString: String? = null) {

    /**
     * Returns a formatted string representation of this event data.
     *
     * @return Formatted string containing event type and JSON data
     * @since 1.0.0
     */
    override fun toString(): String {
        return "eventType: $eventType \n eventJsonString: $eventJsonString"
    }
}
