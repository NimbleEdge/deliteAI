# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet ${VISIBILITY} src/nimblenet.cpp src/nimblejson.cpp)
target_include_directories(nimblenet PUBLIC include/)
