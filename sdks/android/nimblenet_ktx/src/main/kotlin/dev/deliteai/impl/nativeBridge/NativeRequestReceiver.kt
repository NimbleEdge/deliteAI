package dev.deliteai.impl.nativeBridge

import dev.deliteai.client.TextToSpeech

internal class NativeRequestReceiver(private val textToSpeech: TextToSpeech?) {
    fun dispatch(functionName: String, vararg args: Any?): Any? = when (functionName) {
        "getPhonemes" -> handleGetPhonemes(*args)

        else -> throw IllegalArgumentException("Unknown function: $functionName")
    }

    private fun handleGetPhonemes(vararg args: Any?): String? {
        if(textToSpeech == null) throw Exception("TextToSpeechImpl not found in NimbleNetConfig")

        val txt = args.getOrNull(0) as? String
            ?: throw IllegalArgumentException("get_phonemes requires a String")

        return textToSpeech.getPhonemes(txt)
    }
}
