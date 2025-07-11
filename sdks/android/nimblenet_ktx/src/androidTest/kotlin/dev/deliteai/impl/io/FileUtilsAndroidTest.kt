/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.impl.io

import dev.deliteai.impl.DependencyContainer
import dev.deliteai.impl.common.SDK_CONSTANTS
import dev.deliteai.testUtils.nnConfig
import android.app.Application
import android.os.Environment
import androidx.test.core.app.ApplicationProvider
import java.io.File
import org.json.JSONObject
import org.json.JSONArray
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

class FileUtilsAndroidTest {
    private val fileIOTestFolder = SDK_CONSTANTS.NIMBLE_SDK_FOLDER_NAME
    private lateinit var parentFolder: File
    private lateinit var metricsFolder: File
    private lateinit var logsFolder: File
    private lateinit var fileUtils: FileUtils
    private lateinit var application: Application

    @Before
    fun setup() {
        application = ApplicationProvider.getApplicationContext<Application>()
        parentFolder = File(application.filesDir, fileIOTestFolder)
        metricsFolder = File(parentFolder, "metrics")
        logsFolder = File(parentFolder, "logs")

        val dependencyContainer = DependencyContainer.getInstance(application, nnConfig)
        fileUtils = dependencyContainer.getFileUtils()
        parentFolder.deleteRecursively()
    }

    @Test
    fun getInternalStorageFolderSizesShouldCalculateSizesRecursively() {
        parentFolder.mkdirs()
        metricsFolder.mkdirs()
        logsFolder.mkdirs()

        val file1 = File(parentFolder, "file1.txt").apply { writeText("12345") }
        val file2 = File(metricsFolder, "file2.txt").apply { writeText("Hello") }
        val file3 =
            File(logsFolder, "file3.txt").apply {
                writeText("World!World!World!World!World!World!World!World!World!World!")
            }

        val result = fileUtils.getInternalStorageFolderSizes()
        assertNotNull(result)

        val json = JSONObject(result!!)
        assertTrue(json.has(fileIOTestFolder))
        assertTrue(json.has("metrics"))
        assertTrue(json.has("logs"))

        val parentSize = json.getLong(fileIOTestFolder)
        val metricsSize = json.getLong("metrics")
        val logsSize = json.getLong("logs")

        val expectedParentSize = file1.length() + file2.length() + file3.length()
        assertEquals(expectedParentSize, parentSize)
        assertEquals(file2.length(), metricsSize)
        assertEquals(file3.length(), logsSize)
    }

    @Test
    fun sizeOfParentFolderAndChildFoldersMustBeZeroIfParentFolderDoNotExist() {
        val result = fileUtils.getInternalStorageFolderSizes()
        assertNotNull(result)

        val json = JSONObject(result!!)
        assertEquals(0, json.getLong(fileIOTestFolder))
        assertEquals(0, json.getLong("metrics"))
        assertEquals(0, json.getLong("logs"))
    }

    @Test
    fun sizeOfChildFolderMustBeZeroThatDoNotExist() {
        parentFolder.mkdirs()

        val result = fileUtils.getInternalStorageFolderSizes()
        assertNotNull(result)

        val json = JSONObject(result!!)
        assertEquals(0, json.getLong("metrics"))
        assertEquals(0, json.getLong("logs"))
    }

    @Test
    fun getSDKDirPathShouldCreateAndReturnPath() {
        val sdkDirPath = fileUtils.getSDKDirPath()
        val sdkDir = File(sdkDirPath)

        assertTrue(sdkDir.exists())
        assertTrue(sdkDir.isDirectory)

        sdkDir.delete()
    }

    @Test
    fun moveFileShouldMoveSuccessfully() {
        val sourceFile =
            File(application.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS), "source.txt")
                .apply { writeText("Move Me!") }
        val destinationFile = File(application.filesDir, "destination.txt")

        fileUtils.moveFile(sourceFile, destinationFile)

        assertFalse(sourceFile.exists())
        assertTrue(destinationFile.exists())
        assertEquals("Move Me!", destinationFile.readText())

        destinationFile.delete()
    }

    @Test
    fun copyAssetsAndUpdatePathShouldCopyModelScriptAndDocument() {
        val assetsJsonStr = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "misc/add_script.ast"
                }
            },
            {
                "name": "add_model",
                "version": "1.0.0",
                "type": "model",
                "location": {
                    "path": "misc/add_two_model.onnx"
                }
            },
            {
                "name": "groceryItems",
                "version": "1.0.0",
                "type": "document",
                "location": {
                    "path": "retriever/grocery.json"
                }
            }
        ]
        """.trimIndent()

        val assetsJson = JSONArray(assetsJsonStr)

        fileUtils.copyAssetsAndUpdatePath(assetsJson)

        // Validate each asset's path is updated to an absolute path and the file exists
        for (i in 0 until assetsJson.length()) {
            val asset = assetsJson.getJSONObject(i)
            val type = asset.getString("type")
            if (type.equals("retriever", ignoreCase = true)) continue // Not applicable here
            val location = asset.getJSONObject("location")
            val updatedPath = location.getString("path")
            val file = File(updatedPath)
            assertTrue("$type path should be absolute", file.isAbsolute)
            assertTrue("$type file should exist", file.exists())
        }
    }

    @Test
    fun copyAssetsAndUpdatePathShouldCopyNestedRetrieverAssets() {
        val assetsJsonStr = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "retriever/retriever.ast"
                }
            },
            {
                "name": "GroceryRAG",
                "version": "1.0.0",
                "type": "retriever",
                "arguments": [
                    {
                        "name": "embeddingModel",
                        "version": "1.0.0",
                        "type": "model",
                        "location": {
                            "path": "retriever/embedding_model.onnx"
                        }
                    },
                    {
                        "name": "embeddingStoreModel",
                        "version": "1.0.0",
                        "type": "model",
                        "location": {
                            "path": "retriever/embedding_store_model.onnx"
                        }
                    },
                    {
                        "name": "groceryItems",
                        "version": "1.0.0",
                        "type": "document",
                        "location": {
                            "path": "retriever/grocery.json"
                        }
                    }
                ]
            }
        ]
        """.trimIndent()

        val assetsJson = JSONArray(assetsJsonStr)

        fileUtils.copyAssetsAndUpdatePath(assetsJson)

        // Validate script asset
        val scriptAsset = assetsJson.getJSONObject(0)
        val scriptPath = scriptAsset.getJSONObject("location").getString("path")
        val scriptFile = File(scriptPath)
        assertTrue(scriptFile.isAbsolute)
        assertTrue(scriptFile.exists())

        // Validate nested assets inside retriever
        val retrieverAsset = assetsJson.getJSONObject(1)
        val argumentsArray = retrieverAsset.getJSONArray("arguments")
        for (i in 0 until argumentsArray.length()) {
            val arg = argumentsArray.getJSONObject(i)
            val location = arg.getJSONObject("location")
            val updatedPath = location.getString("path")
            val file = File(updatedPath)
            assertTrue(file.isAbsolute)
            assertTrue(file.exists())
        }
    }

    @Test
    fun copyAssetsAndUpdatePathShouldCopyLLMFolderRecursively() {
        val assetsJsonStr = """
        [
            {
                "name": "workflow_script",
                "version": "1.0.0",
                "type": "script",
                "location": {
                    "path": "misc/add_script.ast"
                }
            },
            {
                "name": "llama-3",
                "version": "1.0.0",
                "type": "llm",
                "location": {
                    "path": "llm"
                }
            }
        ]
        """.trimIndent()

        val assetsJson = JSONArray(assetsJsonStr)

        fileUtils.copyAssetsAndUpdatePath(assetsJson)

        // Validate script asset copied
        val scriptAsset = assetsJson.getJSONObject(0)
        val scriptFilePath = scriptAsset.getJSONObject("location").getString("path")
        val scriptFile = File(scriptFilePath)
        assertTrue(scriptFile.isAbsolute)
        assertTrue(scriptFile.exists())

        // Validate LLM folder copied recursively
        val llmAsset = assetsJson.getJSONObject(1)
        val llmPath = llmAsset.getJSONObject("location").getString("path")
        val llmDir = File(llmPath)
        assertTrue(llmDir.isAbsolute)
        assertTrue(llmDir.exists())
        assertTrue(llmDir.isDirectory)

        // Check that every file from the original assets/llm folder exists in copied directory
        val assetManager = application.assets

        fun collectAssetPaths(base: String): List<String> {
            val children = assetManager.list(base) ?: return emptyList()
            val paths = mutableListOf<String>()
            for (child in children) {
                val childPath = if (base.isEmpty()) child else "$base/$child"
                if ((assetManager.list(childPath)?.isNotEmpty() == true)) {
                    // directory
                    paths += collectAssetPaths(childPath).map { "$child/${it}" }
                } else {
                    // file
                    paths += child
                }
            }
            return paths
        }

        val expectedFiles = collectAssetPaths("llm")
        assertTrue("LLM assets should not be empty", expectedFiles.isNotEmpty())

        expectedFiles.forEach { relativePath ->
            val copiedFile = File(llmDir, relativePath)
            assertTrue("$relativePath should exist in copied LLM directory", copiedFile.exists())
        }
    }
}
