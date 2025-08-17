# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet
 "${VISIBILITY}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/c_tensor.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/nimblejson.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/nimblenet.cpp"
)

target_include_directories(nimblenet
 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
