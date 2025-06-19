/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <csignal>

namespace ne {
/**
 * @brief Handles crash signal from Android/Ios.
 */
void handle_crash_signal(int signum, siginfo_t* info, void* context);

}  // namespace ne