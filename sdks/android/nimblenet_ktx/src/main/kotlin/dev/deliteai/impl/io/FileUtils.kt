/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import android.app.Application
import android.content.Context
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

    // TODO: Break this function
    fun processModules(context: Context, assetsJson: JSONArray): JSONArray {
        // Create target directory in internal storage
        val targetDir =
            File(getSDKDirPath(), DELITE_ASSETS_TEMP_STORAGE).apply { if (!exists()) mkdirs() }

        val retainedFiles = mutableSetOf<String>()

        fun processModuleObject(module: JSONObject) {
            if (module.has("location")) {
                val location = module.getJSONObject("location")
                val assetPath = location.getString("path")
                val fileExt = File(assetPath).extension
                val fileName =
                    "${module.getString("name")}_${module.getString("version")}" +
                        if (fileExt.isNotBlank()) ".$fileExt" else ""
                retainedFiles += fileName
                val outputFile = File(targetDir, fileName)

                // Only copy if file doesn't exist
                if (!outputFile.exists()) {
                    context.assets.open(assetPath).use { input ->
                        FileOutputStream(outputFile).use { output -> input.copyTo(output) }
                    }
                }

                // Update path in JSON
                location.put("path", outputFile.absolutePath)
            }

            // Recursively process nested arguments if present
            if (module.has("arguments")) {
                val argumentsArray = module.getJSONArray("arguments")
                for (j in 0 until argumentsArray.length()) {
                    processModuleObject(argumentsArray.getJSONObject(j))
                }
            }
        }

        for (i in 0 until assetsJson.length()) {
            processModuleObject(assetsJson.getJSONObject(i))
        }

        // Delete files not modified in last 7 days
        targetDir.listFiles()?.forEach { file ->
            if (file.name !in retainedFiles) {
                try {
                    val path = file.toPath()
                    val attrs = Files.readAttributes(path, BasicFileAttributes::class.java)
                    val lastAccessTime = attrs.lastAccessTime().toMillis()

                    if (
                        System.currentTimeMillis() - lastAccessTime >
                            DELITE_ASSETS_TEMP_FILES_EXPIRY_IN_MILLIS
                    ) {
                        file.delete()
                    }
                } catch (e: Exception) {
                    file.delete()
                }
            }
        }
        return assetsJson
    }
}
