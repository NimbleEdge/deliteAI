/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.notifications_summarizer.impl.notification

import dev.deliteai.notifications_summarizer.impl.notification.dataModels.NotificationSnapshot
import android.app.Notification
import android.os.Bundle
import android.service.notification.NotificationListenerService
import android.service.notification.StatusBarNotification
import android.util.Log

class NotificationListener : NotificationListenerService() {
    companion object {
        private const val TAG = "NotificationListener"
        private var notifications: List<NotificationSnapshot> = emptyList()
        private var serviceInstance: NotificationListener? = null
        private var isConnected = false

        fun getNotifications(): List<NotificationSnapshot> = notifications
        fun isListenerConnected(): Boolean = isConnected && (serviceInstance != null)
    }

    override fun onCreate() {
        super.onCreate()
        serviceInstance = this
        Log.d(TAG, "NotificationListener service onCreate() called")
    }

    override fun onDestroy() {
        super.onDestroy()
        serviceInstance = null
        Log.d(TAG, "NotificationListener service onDestroy() called")
    }

    override fun onListenerConnected() {
        super.onListenerConnected()
        Log.d(TAG, "onListenerConnected() called - Notification service is now connected!")
        refreshNotifications()
        isConnected = true
    }

    override fun onListenerDisconnected() {
        super.onListenerDisconnected()
        isConnected = false
        Log.w(TAG, "onListenerDisconnected() called - Notification service disconnected!")
    }

    override fun onNotificationPosted(sbn: StatusBarNotification?) {
        super.onNotificationPosted(sbn)
        Log.d(
            TAG,
            "onNotificationPosted: ${sbn?.packageName} - ${
                sbn?.notification?.extras?.getCharSequence(Notification.EXTRA_TITLE)
            }"
        )
        refreshNotifications()
    }

    override fun onNotificationRemoved(sbn: StatusBarNotification?) {
        super.onNotificationRemoved(sbn)
        Log.d(TAG, "onNotificationRemoved: ${sbn?.packageName}")
        refreshNotifications()
    }

    private fun refreshNotifications() {
        Log.d(TAG, "Refreshing notifications...")
        try {
            val activeNotifs = activeNotifications
            Log.d(TAG, "Active notifications count: ${activeNotifs?.size ?: 0}")

            notifications = activeNotifs?.mapNotNull { sbn ->
                sbn.notification.extras.extractSnapshot(sbn.packageName, sbn.notification)
            } ?: emptyList()

            Log.d(TAG, "Found ${notifications.size} valid notifications")
        } catch (e: Exception) {
            Log.e(TAG, "Error refreshing notifications: ${e.message}", e)
        }
    }

    private fun Bundle.extractSnapshot(
        packageName: String,
        notification: Notification
    ): NotificationSnapshot? {
        val title = getTextString(Notification.EXTRA_TITLE) ?: return null
        val body = getTextString(Notification.EXTRA_TEXT)
            ?: getTextString(Notification.EXTRA_BIG_TEXT)
            ?: ""
        val subText = getTextString(Notification.EXTRA_SUB_TEXT) ?: ""
        return NotificationSnapshot(
            packageName = packageName,
            channel = notification.channelId,
            priority = notification.priority,
            title = title,
            body = body,
            subText = subText
        )
    }

    private fun Bundle.getTextString(key: String): String? =
        getCharSequence(key)?.toString()
}
