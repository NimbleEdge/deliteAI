/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.app.Application
import android.content.res.AssetManager
import dev.deliteai.impl.common.SDK_CONSTANTS
import dev.deliteai.impl.common.SDK_CONSTANTS.DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS
import dev.deliteai.impl.common.SDK_CONSTANTS.DELITE_ASSETS_TEMP_STORAGE
import dev.deliteai.impl.loggers.LocalLogger
import java.io.File
import java.io.FileOutputStream
import java.nio.file.Files
import java.nio.file.attribute.BasicFileAttributes
import org.json.JSONArray
import org.json.JSONObject

internal class FileUtils(
    private val application: Application,
    private val localLogger: LocalLogger,
) {
    private val assetManager: AssetManager = application.assets

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

        val targetDir =
            File(getSDKDirPath(), DELITE_ASSETS_TEMP_STORAGE).apply { if (!exists()) mkdirs() }

        val filesToRetain = mutableSetOf<File>()

        for (idx in 0 until assetsJson.length()) {
            val assetInfo = assetsJson.getJSONObject(idx)
            val name = assetInfo.getString("name")
            val version = assetInfo.getString("version")

            val locationObject = assetInfo.optJSONObject("location")
            val assetPath = locationObject?.optString("path")
            val arguments = assetInfo.optJSONArray("arguments")

            if (arguments != null) {
                copyAssetsAndUpdatePath(arguments)
            } else if (assetPath != null) {
                val targetFile = constructTargetFile(targetDir, assetPath, name, version)
                filesToRetain.add(targetFile)

                if (isAssetDir(assetPath)) {
                    copyAssetFolderRecursively(assetPath, targetFile)
                } else {
                    copyAssetFile(assetPath, targetFile)
                }
                locationObject.put("path", targetFile.absolutePath)
            } else {
                throw Exception("Both arguments & assetPath are null")
            }
        }

        pruneStaleAssets(targetDir, filesToRetain)
    }

    private fun constructTargetFile(
        targetDir: File,
        src: String,
        name: String,
        version: String,
    ): File {

        val ext = File(src).extension
        val filename = "${name}_${version}" + if (ext.isNotBlank()) ".$ext" else ""
        return File(targetDir, filename)
    }

    private fun copyAssetFile(src: String, target: File) {
        if (!target.exists()) {
            assetManager.open(src).use { input ->
                FileOutputStream(target).use { output -> input.copyTo(output) }
            }
        }
    }

    private fun copyAssetFolderRecursively(src: String, target: File) {
        if (!target.exists()) target.mkdirs()

        val children = assetManager.list(src) ?: return
        for (child in children) {
            val childAssetPath = if (src.isEmpty()) child else "$src/$child"
            val childTarget = File(target, child)
            if (isAssetDir(childAssetPath)) {
                copyAssetFolderRecursively(childAssetPath, childTarget)
            } else {
                copyAssetFile(childAssetPath, childTarget)
            }
        }
    }

    private fun pruneStaleAssets(target: File, filesToRetain: Set<File>) {
        target.listFiles()?.forEach { file ->
            try {
                if (!filesToRetain.contains(file) && !wasAccessedWithinExpiry(file)) {
                    file.delete()
                }
            } catch (e: Exception) {
                localLogger.e(e)
                file.delete()
            }
        }
    }

    private fun wasAccessedWithinExpiry(file: File): Boolean {
        val attrs = Files.readAttributes(file.toPath(), BasicFileAttributes::class.java)
        val lastAccessMillis = attrs.lastAccessTime().toMillis()
        return System.currentTimeMillis() - lastAccessMillis <=
            DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS
    }

    private fun isAssetDir(path: String): Boolean {
        return try {
            // list() returns the names of all entries under `path`
            // if it's a file, we get an empty array
            assetManager.list(path)?.isNotEmpty() == true
        } catch (e: Exception) {
            false
        }
    }
}
