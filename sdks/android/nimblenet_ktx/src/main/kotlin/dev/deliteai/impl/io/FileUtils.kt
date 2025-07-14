/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.app.Application
import dev.deliteai.impl.common.ASSET_TYPE
import dev.deliteai.impl.common.SDK_CONSTANTS
import dev.deliteai.impl.common.SDK_CONSTANTS.DELITE_ASSETS_TEMP_STORAGE
import dev.deliteai.impl.loggers.LocalLogger
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.io.FileOutputStream

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

    fun copyAssetsAndUpdatePath(assetsJson: JSONArray?) {
        if (assetsJson == null) return

        for (idx in 0 until assetsJson.length()) {
            val assetInfo = assetsJson.getJSONObject(idx)
            val name = assetInfo.getString("name")
            val version = assetInfo.getString("version")
            val type = ASSET_TYPE.fromString(assetInfo.getString("type"))

            val locationObject = assetInfo.optJSONObject("location")
            val assetPath = locationObject?.optString("path")
            val arguments = assetInfo.optJSONArray("arguments")

            when (type) {
                ASSET_TYPE.MODEL, ASSET_TYPE.SCRIPT, ASSET_TYPE.DOCUMENT -> {
                    if (assetPath == null) throw Exception("missing key: location")

                    val targetFile = constructTargetFile(assetPath, name, version)
                    copyAssetFile(assetPath, targetFile)
                    locationObject.put("path", targetFile.absolutePath)
                }

                ASSET_TYPE.LLM -> {
                    if (assetPath == null) throw Exception("missing key: location")

                    val targetFile = constructTargetFile(assetPath, name, version)
                    copyAssetFolderRecursively(assetPath, targetFile)
                    locationObject.put("path", targetFile.absolutePath)
                }

                ASSET_TYPE.RETRIEVER -> {
                    if (arguments == null) throw Exception("missing key: arguments")

                    copyAssetsAndUpdatePath(arguments)
                }
            }
        }
    }

    private fun constructTargetFile(src: String, name: String, version: String): File {
        val targetBase = File(getSDKDirPath(), DELITE_ASSETS_TEMP_STORAGE)
            .apply { if (!exists()) mkdirs() }
        val ext = File(src).extension
        val filename = "${name}_${version}" + if (ext.isNotBlank()) ".$ext" else ""
        return File(targetBase, filename)
    }

    private fun copyAssetFile(src: String, target: File) {
        val assetManager = application.assets

        if (!target.exists()) {
            assetManager.open(src).use { input ->
                FileOutputStream(target).use { output ->
                    input.copyTo(output)
                }
            }
        }
    }

    private fun copyAssetFolderRecursively(
        src: String,
        target: File
    ) {
        val assetManager = application.assets
        val children = assetManager.list(src) ?: return
        for (child in children) {
            val childAssetPath = if (src.isEmpty()) child else "$src/$child"
            val childTarget = File(target, child)
            if (assetManager.list(childAssetPath)?.isNotEmpty() == true) {
                // nested folder
                copyAssetFolderRecursively(childAssetPath, childTarget)
            } else {
                // single file
                childTarget.parentFile?.mkdirs()
                assetManager.open(childAssetPath).use { input ->
                    FileOutputStream(childTarget).use { output ->
                        input.copyTo(output)
                    }
                }
            }
        }
    }
}
