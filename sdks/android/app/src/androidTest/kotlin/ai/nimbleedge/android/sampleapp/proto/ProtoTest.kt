package ai.nimbleedge.android.sampleapp.proto

import ai.nimbleedge.NimbleNet
import ai.nimbleedge.datamodels.NimbleNetTensor
import ai.nimbleedge.impl.delitePy.proto.ProtoMemberExtender
import ai.nimbleedge.impl.delitePy.proto.impl.ProtoObjectWrapper
import ai.nimbleedge.impl.common.DATATYPE
import android.app.Application
import androidx.test.platform.app.InstrumentationRegistry
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.BeforeClass
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.Parameterized

@RunWith(Parameterized::class)
class ProtoTest(private val functionName: String, private val expectedResult: ExpectedResult) {

    companion object {

        private val companyProto = createCompany(2, 3, 1)

        @JvmStatic
        @Parameterized.Parameters(name = "{0}")
        fun testCases() = listOf(
            arrayOf("test_as_is", ExpectedResult(payload = mapOf("response" to companyProto))),
            arrayOf("test_valid_getters", ExpectedResult(payload = getGetterResponse(companyProto))),
            arrayOf("test_invalid_key", ExpectedResult(status = false, error = "Field not found: invalidKey")),
            arrayOf("test_invalid_index", ExpectedResult(status = false, error = "Index 12 out of bounds for length 2")),
            arrayOf("test_invalid_key_in_map", ExpectedResult(status = false, error = "Field not found: invalidKey")),
            arrayOf("test_setters_object", ExpectedResult(payload = mapOf("response" to getUpdatedCompany1(companyProto)))),
            arrayOf("test_setters_map", ExpectedResult(payload = mapOf("response" to getUpdatedCompany2(companyProto)))),
            arrayOf("test_setters_list", ExpectedResult(payload = mapOf("response" to getUpdatedCompany3(companyProto)))),
            arrayOf("test_arrange_list", ExpectedResult(payload = mapOf("response" to getUpdatedCompany4(companyProto)))),
            arrayOf("test_any_object", ExpectedResult(payload = mapOf("response" to getUpdatedCompany5(companyProto)))),
            arrayOf("test_setters_list_each_index", ExpectedResult(payload = mapOf("response" to getUpdatedCompany6(companyProto)))),
            arrayOf("test_set_invalid_key", ExpectedResult(status = false, error = "Field not found: invalidKey")),
            arrayOf("test_set_invalid_index", ExpectedResult(status = false, error = "Index 13 out of bounds for length 3")),
            arrayOf("test_invalid_type_in_object", ExpectedResult(status = false, error = "Invalid type: Expected String, got Long")),
            arrayOf("test_invalid_type_in_list", ExpectedResult(status = false, error = "Invalid type: Expected Integer, got String")),
            arrayOf("test_invalid_type_in_map", ExpectedResult(status = false, error = "Invalid type: Expected String, got Long")),
            arrayOf("test_set_empty_list", ExpectedResult(payload = mapOf("response" to getUpdatedCompany7(companyProto)))),
            arrayOf("test_arrange_list_empty", ExpectedResult(payload = mapOf("response" to getUpdatedCompany7(companyProto)))),
            arrayOf("test_set_empty_map", ExpectedResult(payload = mapOf("response" to getUpdatedCompany8(companyProto)))),
            arrayOf("test_list_and_map_pop", ExpectedResult(payload = mapOf("response" to getUpdatedCompany10(companyProto)))),
            arrayOf("test_list_pop_invalid_index", ExpectedResult(status = false, error = "Index 3 out of bounds for length 2")),
            arrayOf("test_map_pop_key_not_present", ExpectedResult(status = false, error = "Trying to remove key=invalidKey, not present in map.")),
            arrayOf("test_list_append", ExpectedResult(payload = mapOf("response" to getUpdatedCompany11(companyProto)))),
            arrayOf("test_set_none", ExpectedResult(payload = mapOf("response" to getUpdatedCompany9(companyProto)))),
            arrayOf("test_set_none_list", ExpectedResult(status = false, error = "Cannot set None to list/map field")),
            arrayOf("test_set_none_map", ExpectedResult(status = false, error = "Cannot set None to list/map field")),
        )

        @JvmStatic
        @BeforeClass
        fun initializeNimbleNet(): Unit = runBlocking(Dispatchers.Default) {
            val context =
                InstrumentationRegistry.getInstrumentation().targetContext.applicationContext as Application
            val res = NimbleNet.initialize(context, config.toString())
            if (!res.status) throw RuntimeException("Init failed")

            var time = 0
            while (true) {
                val isReady = NimbleNet.isReady()
                if (isReady.status) break
                if (time > 30) throw RuntimeException("TLE")
                time++
                delay(5000)
            }
        }
    }

    @Test
    fun validateProtoOperations() = runBlocking {
        val protoInput = hashMapOf(
            "inputData" to NimbleNetTensor(
                data = ProtoObjectWrapper(companyProto, typeRegistry),
                datatype = DATATYPE.FE_OBJ,
                shape = intArrayOf()
            )
        )
        val retVal = NimbleNet.runMethod(functionName, protoInput)
        assertEquals("runMethod status does not match", expectedResult.status, retVal.status)
        if (retVal.status) {
            expectedResult.payload.forEach { (key, expectedValue) ->
                val data = retVal.payload!![key]?.data
                if (data is ProtoMemberExtender) {
                    val result = data.message
                    assertEquals("proto object in payload does not match for key: $key", expectedValue, result)
                } else if (data is JSONArray) {
                    assertEquals("list object in payload does not match for key: $key",
                        JSONArray(expectedValue as List<*>).toString(), data.toString())
                } else if (data is JSONObject) {
                    assertEquals("dict object in payload does not match for key: $key",
                        JSONObject(expectedValue as Map<*,*>).toString(), data.toString())
                } else {
                    assertEquals("payload does not match for key: $key", expectedValue, data)
                }
            }
        } else {
            val error = retVal.error?.message!!
            assertTrue("error message does not match, error: $error", error.contains(expectedResult.error))
        }
    }
}
