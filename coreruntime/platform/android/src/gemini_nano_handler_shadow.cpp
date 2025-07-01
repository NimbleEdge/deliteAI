/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "gemini_nano_handler_shadow.hpp"

#include <stdexcept>

#include "../utils/jni_logger.h"

GeminiNanoHandlerShadow::GeminiNanoHandlerShadow(JNIEnv *env) {
	if (env == nullptr) return;

	jclass localClass = env->FindClass("dev/deliteai/scriptWrappers/GeminiNanoHandler");
	if (localClass == nullptr) {
		LOGE("Class dev.deliteai.scriptWrappers.GeminiNanoHandler not found.\n");
		return;
	}

	geminiNanoHandlerClass = static_cast<jclass>(env->NewGlobalRef(localClass));
	env->DeleteLocalRef(localClass);
	if (geminiNanoHandlerClass == nullptr) {
		LOGE("Failed to create global reference for GeminiNanoHandler class.\n");
		return;
	}

	initializeMethodId = env->GetStaticMethodID(geminiNanoHandlerClass, "initialize", "(Landroid/content/Context;)V");
	promptMethodId = env->GetStaticMethodID(geminiNanoHandlerClass, "prompt", "(Ljava/lang/String;)V");
	cancelMethodId = env->GetStaticMethodID(geminiNanoHandlerClass, "cancel", "()V");
	getStatusMethodId = env->GetStaticMethodID(geminiNanoHandlerClass, "getStatus", "()I");

	if (initializeMethodId == nullptr || promptMethodId == nullptr || cancelMethodId == nullptr || getStatusMethodId == nullptr) {
		LOGE("One or more methods in GeminiNanoHandler class could not be found.\n");
	}
}

void GeminiNanoHandlerShadow::initialize(JNIEnv *env, jobject context) {
	if (env == nullptr || geminiNanoHandlerClass == nullptr || initializeMethodId == nullptr) {
		throw std::runtime_error("Invalid state to call initialize.");
	}

	env->CallStaticVoidMethod(geminiNanoHandlerClass, initializeMethodId, context);
}

void GeminiNanoHandlerShadow::prompt(JNIEnv *env, const std::string &prompt) {
	if (env == nullptr || geminiNanoHandlerClass == nullptr || promptMethodId == nullptr) {
		throw std::runtime_error("Invalid state to call prompt.");
	}

	jstring jPrompt = env->NewStringUTF(prompt.c_str());
	env->CallStaticVoidMethod(geminiNanoHandlerClass, promptMethodId, jPrompt);
	env->DeleteLocalRef(jPrompt);
}

void GeminiNanoHandlerShadow::cancel(JNIEnv *env) {
	if (env == nullptr || geminiNanoHandlerClass == nullptr || cancelMethodId == nullptr) {
		throw std::runtime_error("Invalid state to call cancel.");
	}

	env->CallStaticVoidMethod(geminiNanoHandlerClass, cancelMethodId);
}

FileDownloadStatus GeminiNanoHandlerShadow::getStatus(JNIEnv *env) {
	if (env == nullptr || geminiNanoHandlerClass == nullptr || getStatusMethodId == nullptr) {
		throw std::runtime_error("Invalid state to call getStatus.");
	}

	jint statusValue = env->CallStaticIntMethod(geminiNanoHandlerClass, getStatusMethodId);
	return static_cast<FileDownloadStatus>(statusValue);
}
