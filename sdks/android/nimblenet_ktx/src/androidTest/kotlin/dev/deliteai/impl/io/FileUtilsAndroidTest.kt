/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

packagedev.deliteai.impl.io

importdev.deliteai.impl.DependencyContainer
importdev.deliteai.impl.common.SDK_CONSTANTS
importdev.deliteai.testUtils.nnConfig
import android.app.Application
import android.os.Environment
import androidx.test.core.app.ApplicationProvider
import java.io.File
import org.json.JSONObject
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
}
