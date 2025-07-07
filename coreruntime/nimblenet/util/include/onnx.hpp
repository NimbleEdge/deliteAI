/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef IOS
#include <onnxruntime/cpu_provider_factory.h>
#include <onnxruntime/onnxruntime_cxx_api.h>
#elif __ANDROID__
#include <cpu_provider_factory.h>
#include <nnapi_provider_factory.h>
#include <onnxruntime_cxx_api.h>
#else
#include <cpu_provider_factory.h>
#include <onnxruntime_cxx_api.h>
#endif
