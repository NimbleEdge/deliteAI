/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package ai.nimbleedge.impl.io

import ai.nimbleedge.impl.common.SDK_CONSTANTS
import ai.nimbleedge.impl.loggers.LocalLogger
import android.app.Application
import java.io.File
import org.json.JSONObject

internal class FileUtils(
    private val application: Application,
    private val localLogger: LocalLogger,
) {
    fun getInternalStorageFolderSizes(): String? =
        runCatching {
                val parent = application.filesDir.resolve(SDK_CONSTANTS.NIMBLE_SDK_FOLDER_NAME)
                val sizes = buildMap {
                    put(SDK_CONSTANTS.NIMBLE_SDK_FOLDER_NAME, parent.folderSize())
                    listOf("metrics", "logs").forEach { name ->
                        put(name, parent.resolve(name).takeIf(File::exists)?.folderSize() ?: 0L)
                    }
                }
                JSONObject(sizes).toString()
            }
            .onFailure { localLogger.e(it) }
            .getOrNull()

    fun moveFile(source: File, dest: File): Boolean =
        runCatching {
                source.inputStream().use { it.copyTo(dest.outputStream()) }
                if (!source.delete()) {
                    localLogger.d("Failed to delete the source file")
                }
                true
            }
            .onFailure { localLogger.e(it) }
            .getOrDefault(false)

    fun getSDKDirPath(): String =
        application.filesDir
            .resolve(SDK_CONSTANTS.NIMBLE_SDK_FOLDER_NAME)
            .apply { if (!exists()) mkdir() }
            .absolutePath

    private fun File.folderSize(): Long = walk().filter { it.isFile }.sumOf { it.length() }
}
