# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
# SPDX-License-Identifier: Apache-2.0

add_library(dljni OBJECT)
target_include_directories(dljni
 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_sources(dljni
 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/dljnicurrentthreadattacher.cpp"
)
target_compile_features(dljni
 PUBLIC
  cxx_std_17
)
