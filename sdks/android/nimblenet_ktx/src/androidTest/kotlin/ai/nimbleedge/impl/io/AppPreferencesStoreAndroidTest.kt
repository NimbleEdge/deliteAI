package ai.nimbleedge.impl.io

import android.content.Context
import android.content.SharedPreferences
import androidx.test.core.app.ApplicationProvider
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNull
import org.junit.Before
import org.junit.Test

class AppPreferencesStoreAndroidTest {
    private lateinit var appPreferencesStore: AppPreferencesStore
    private lateinit var sharedPreferences: SharedPreferences

    @Before
    fun setUp() {
        val context = ApplicationProvider.getApplicationContext<Context>()
        sharedPreferences = context.getSharedPreferences("test_prefs", Context.MODE_PRIVATE)
        appPreferencesStore = AppPreferencesStore(sharedPreferences)
        sharedPreferences.edit().clear().apply()
    }

    @After
    fun tearDown() {
        sharedPreferences.edit().clear().apply()
    }

    @Test
    fun putAndGetShouldStoreAndRetrieveValueCorrectly() {
        val key = "test_key"
        val value = "test_value"

        appPreferencesStore.put(key, value)
        val retrievedValue = appPreferencesStore.get(key)

        assertEquals(value, retrievedValue)
    }

    @Test
    fun getShouldReturnNullForKeyThatDoesNotExist() {
        val retrievedValue = appPreferencesStore.get("non_existent_key")

        assertNull(retrievedValue)
    }

    @Test
    fun deleteShouldRemoveKeyValuePair() {
        val key = "key_to_delete"
        val value = "value_to_delete"
        appPreferencesStore.put(key, value)

        appPreferencesStore.delete(key)
        val retrievedValue = appPreferencesStore.get(key)

        assertNull(retrievedValue)
    }

    @Test
    fun putShouldOverwriteExistingValue() {
        val key = "test_key"
        val initialValue = "initial_value"
        val newValue = "new_value"
        appPreferencesStore.put(key, initialValue)

        appPreferencesStore.put(key, newValue)
        val retrievedValue = appPreferencesStore.get(key)

        assertEquals(newValue, retrievedValue)
    }
}
