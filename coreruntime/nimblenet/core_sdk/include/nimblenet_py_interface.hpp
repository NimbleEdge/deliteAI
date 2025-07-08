/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
/**
 * @brief Use this header when some function from nimblenet_py needs to be called in nimblenet
 * library. The functions defined here will be implemented in binder/delitepy_script_parser which
 * uses pybind11.
 */
std::string parseScriptToAST(const std::string& scriptPath);
