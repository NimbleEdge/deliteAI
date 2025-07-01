# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet ${VISIBILITY} src/server_api.cpp include/server_api.hpp)

target_include_directories(nimblenet ${VISIBILITY} ./include/)
