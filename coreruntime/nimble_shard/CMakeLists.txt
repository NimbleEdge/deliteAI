# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

add_executable(nimble_shard src/nimble_shard.cpp)
target_link_libraries(nimble_shard PRIVATE core_utils)
