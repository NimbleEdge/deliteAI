# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

add_executable(nimble_diff src/nimble_diff.cpp)
target_link_libraries(nimble_diff PRIVATE nlohmann_json::nlohmann_json)
