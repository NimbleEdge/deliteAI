package dev.deliteai.client

interface TextToSpeech {
    fun getPhonemes(text: String): String?
}
