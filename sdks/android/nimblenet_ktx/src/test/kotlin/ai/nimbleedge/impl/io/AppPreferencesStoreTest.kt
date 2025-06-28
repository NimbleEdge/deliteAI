package ai.nimbleedge.impl.io

import android.content.SharedPreferences
import io.mockk.MockKAnnotations
import io.mockk.every
import io.mockk.mockk
import io.mockk.verify
import kotlin.test.assertEquals
import kotlin.test.assertNull
import org.junit.Before
import org.junit.Test

class AppPreferencesStoreTest {

    private lateinit var appPreferencesStore: AppPreferencesStore
    private val sharedPreferences: SharedPreferences = mockk(relaxed = true)
    private val editor: SharedPreferences.Editor = mockk(relaxed = true)

    @Before
    fun setup() {
        MockKAnnotations.init(this)
        every { sharedPreferences.edit() } returns editor
        every { editor.putString(any(), any()) } returns editor
        every { editor.remove(any()) } returns editor
        every { editor.apply() } returns Unit

        appPreferencesStore = AppPreferencesStore(sharedPreferences)
    }

    @Test
    fun `get returns value when key exists`() {
        val key = "test_key"
        val expectedValue = "test_value"
        every { sharedPreferences.getString(key, null) } returns expectedValue

        val result = appPreferencesStore.get(key)

        assertEquals(expectedValue, result)
        verify { sharedPreferences.getString(key, null) }
    }

    @Test
    fun `get returns null when key does not exist`() {
        val key = "non_existent_key"
        every { sharedPreferences.getString(key, null) } returns null

        val result = appPreferencesStore.get(key)

        assertNull(result)
        verify { sharedPreferences.getString(key, null) }
    }

    @Test
    fun `put stores key-value pair`() {
        val key = "test_key"
        val value = "test_value"

        appPreferencesStore.put(key, value)

        verify { sharedPreferences.edit() }
        verify { editor.putString(key, value) }
        verify { editor.apply() }
    }

    @Test
    fun `delete removes key from preferences`() {
        val key = "test_key"

        appPreferencesStore.delete(key)

        verify { sharedPreferences.edit() }
        verify { editor.remove(key) }
        verify { editor.apply() }
    }

    @Test
    fun `multiple operations work correctly`() {
        val key1 = "key1"
        val value1 = "value1"
        val key2 = "key2"

        every { sharedPreferences.getString(key1, null) } returns value1
        every { sharedPreferences.getString(key2, null) } returns null

        appPreferencesStore.put(key1, value1)
        verify { editor.putString(key1, value1) }

        val result1 = appPreferencesStore.get(key1)
        assertEquals(value1, result1)

        val result2 = appPreferencesStore.get(key2)
        assertNull(result2)

        appPreferencesStore.delete(key1)
        verify { editor.remove(key1) }
    }
}
