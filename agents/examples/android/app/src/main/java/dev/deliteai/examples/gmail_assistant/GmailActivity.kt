/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package dev.deliteai.examples.gmail_assistant

import android.accounts.AccountManager
import android.app.Application
import android.content.Intent
import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.material3.TextField
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import com.google.api.client.googleapis.extensions.android.gms.auth.GoogleAccountCredential
import com.google.api.client.googleapis.extensions.android.gms.auth.UserRecoverableAuthIOException
import com.google.api.services.gmail.GmailScopes
import dev.deliteai.NimbleNet
import dev.deliteai.agents.gmail_assistant.GmailAgent
import dev.deliteai.agents.gmail_assistant.dataModels.GmailSummary
import dev.deliteai.datamodels.NimbleNetConfig
import dev.deliteai.impl.common.NIMBLENET_VARIANTS
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext

const val HOST_TAG = "GMAIL_AGENT"

class GmailActivity : ComponentActivity() {

    private val RC_SIGN_IN = 1001
    private val REQUEST_AUTHORIZATION = 1002

    private lateinit var credential: GoogleAccountCredential

    private var isAuthorized = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            GmailScreen()
        }
    }

    // TODO: Move account selection and authorization inside Agent SDK if possible
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode != RESULT_OK) {
            Log.d(HOST_TAG, "Authorization failed or cancelled.")
            return
        }
        if (requestCode == RC_SIGN_IN) {
            val accountName = data!!.getStringExtra(AccountManager.KEY_ACCOUNT_NAME)
            if (accountName != null) {
                credential.selectedAccountName = accountName
                Log.d(HOST_TAG, "Selected gmail account: $accountName")
                lifecycleScope.launch {
                    withContext(Dispatchers.IO) {
                        try {
                            // Get token eagerly so authorization flow is triggered
                            credential.token
                            isAuthorized = true
                        } catch (e: UserRecoverableAuthIOException) {
                            Log.d(HOST_TAG, "UserRecoverableAuthIOException: ${e.message}")
                            startActivityForResult(e.intent, REQUEST_AUTHORIZATION)
                        }
                    }
                }

            }
        } else if (requestCode == REQUEST_AUTHORIZATION) {
            Log.d(HOST_TAG, "Gmail Authorization completed.")
            isAuthorized = true
        }
    }

    private fun initializeAgent(application: Application): String = runBlocking {
        return@runBlocking runCatching {

            val nimbleConfig = NimbleNetConfig(
                clientId = "d-ai-sample",
                host = "https://api.delite.ai",
                deviceId = "test-device",
                clientSecret = "secret_value",
                debug = true,
                compatibilityTag = "agent_gmail",
                libraryVariant = NIMBLENET_VARIANTS.STATIC
            )

            val res = NimbleNet.initialize(application, nimbleConfig)
            check(res.status)

            while (!NimbleNet.isReady().status) delay(1000)

            GmailAgent.initialize(credential)
            "Agent is initialized.\n\nOutput will appear here..."
        }.getOrElse {
            "Initialization failed: ${it.message}"
        }
    }

    @Composable
    fun GmailScreen() {
        val outputText = remember { mutableStateOf("Output will appear here...") }
        val userQuery = remember { mutableStateOf("") }
        val coroutineScope = rememberCoroutineScope()
        val application = LocalContext.current.applicationContext as Application

        LaunchedEffect(Unit) {
            if (!isAuthorized) {
                credential = GoogleAccountCredential.usingOAuth2(
                    application, listOf(
                        GmailScopes.GMAIL_READONLY,
                    )
                )
                startActivityForResult(credential.newChooseAccountIntent(), RC_SIGN_IN)
            } else {
                outputText.value = initializeAgent(application)
            }
        }

        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Column (
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .fillMaxHeight(0.7f)
                        .border(1.dp, Color.Gray)
                        .verticalScroll(rememberScrollState())
                        .padding(8.dp)
                ) {
                    Text(text = outputText.value)
                }

                Button(onClick = {
                    coroutineScope.launch(Dispatchers.Default) {
                        val result = GmailAgent.summarizeUnreadEmails()
                        if (!result.status) {
                            outputText.value = result.error!!.message
                        } else {
                            val summary = result.payload as GmailSummary
                            outputText.value = summary.summary
                        }
                    }
                }) {
                    Text("Summarize unread emails")
                }

                TextField(
                    value = userQuery.value,
                    onValueChange = { userQuery.value = it },
                    label = { Text("Enter custom query") },
                    modifier = Modifier.fillMaxWidth(0.9f)
                )

                Button(onClick = {
                    coroutineScope.launch(Dispatchers.Default) {
                        val result = GmailAgent.promptLlm(userQuery.value)
                        if (!result.status) {
                            outputText.value = result.error!!.message
                        } else {
                            val summary = result.payload as GmailSummary
                            outputText.value = summary.summary
                        }
                    }
                }) {
                    Text("Submit custom query")
                }
            }
        }
    }
}
