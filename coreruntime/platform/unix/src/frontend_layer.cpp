/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "client.h"

DeallocateFrontendType globalDeallocate __attribute__((visibility("default"))) = nullptr;
FreeFrontendContextType globalFrontendContextFree __attribute__((visibility("default"))) = nullptr;