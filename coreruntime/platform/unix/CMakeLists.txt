# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_include_directories(nimblenet PUBLIC "${PROJECT_SOURCE_DIR}/platform/unix/include/")
target_sources(nimblenet ${VISIBILITY} src/client.cpp src/frontend_data_variable.cpp)

## Linked publicly to set globals from python binder. These globals are used in private client.cpp
target_sources(nimblenet PUBLIC src/frontend_layer.cpp)
