/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.android.sampleapp

import android.app.Application
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.ScrollState
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import dev.deliteai.NimbleNet
import dev.deliteai.android.sampleapp.ui.theme.DeliteAiTheme
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

//        MODE == 0 -> UI WITH BUTTONS
//        MODE == 1 -> STRESS TEST FOR JNI MEMORY LEAK SCRIPT

        val sampleAppMode = 0

        setContent {
            DeliteAiTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background
                ) {
                    when (sampleAppMode) {
                        0 -> Homepage()
                        1 -> StressTestForMemDump()
                    }
                }
            }
        }
    }
}

@Composable
fun Homepage() {
    val outputText = remember { mutableStateOf("DeliteAi Android Test App") }
    val scroll = rememberScrollState(0)
    val coroutineScope = rememberCoroutineScope()
    val applicationContext = LocalContext.current.applicationContext as Application

    LaunchedEffect(Unit) {
        outputText.value = "initializing..."
        coroutineScope.launch(Dispatchers.Default) {
            initDeliteAi(applicationContext, outputText, coroutineScope)
        }
    }

    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
    ) {
        OutputBox(outputText, scroll)
        ActionButtonGrid(outputText)
    }
}

@Composable
fun OutputBox(outputText: MutableState<String>, scroll: ScrollState) {
    Box(
        Modifier
            .height(350.dp)
            .fillMaxWidth()
            .background(Color.DarkGray)
    ) {
        Text(text = outputText.value, Modifier.verticalScroll(scroll), color = Color.White)
    }
}

@Composable
fun ActionButtonGrid(outputText: MutableState<String>) {
    LazyVerticalGrid(
        columns = GridCells.Fixed(3), modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp)
    ) {
        item {
            ActionButton("Is Ready") {
                outputText.value = NimbleNet.isReady().toString()
            }
        }
        item {
            ActionButton("Add Event") {
                outputText.value = addEvent()
            }
        }
        item {
            ActionButton("Restart Session") {
                NimbleNet.restartSession()
                outputText.value = "session restarted with internal session id"
            }
        }
        item {
            ActionButton("Restart Session With Id") {
                val customSessionId = "client-session-id"
                NimbleNet.restartSessionWithId(customSessionId)
                outputText.value = "session restarted with $customSessionId"
            }
        }
    }
}

@Composable
fun ActionButton(text: String, onClick: () -> Unit) {
    Button(onClick = onClick) {
        Text(text = text)
    }
}

fun initDeliteAi(
    applicationContext: Application,
    outputText: MutableState<String>,
    coroutineScope: CoroutineScope
) {
    val nimbleNetResult = NimbleNet.initialize(applicationContext, nimblenetConfig)
    outputText.value = nimbleNetResult.toString()
}

fun addEvent() = NimbleNet.addEvent(
    mapOf(
        "productid" to 800,
        "contestType" to "special",
        "roundid" to 97,
        "winnerPercent" to 0.29,
        "prizeAmount" to 900,
        "entryFee" to 50
    ), "ContestJoinedClient"
).toString()

fun getDummyPreprocessorList() = List(30) {
    mapOf(
        "productid" to 800,
        "contestType" to "special",
        "roundid" to 97,
        "winnerPercent" to 0.29,
        "prizeAmount" to 900,
        "entryFee" to 50
    )
}

// Config
private val nimblenetConfig = NimbleNetConfig(
    clientId = BuildConfig.APP_CLIENT_ID,
    host = BuildConfig.APP_HOST,
    deviceId = "test",
    clientSecret = BuildConfig.APP_CLIENT_SECRET,
    debug = true,
    initTimeOutInMs = 1000000000,
    compatibilityTag = "android-output-verification",
    libraryVariant = NIMBLENET_VARIANTS.STATIC,
    online = true
)
