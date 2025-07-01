# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet ${VISIBILITY} 
  src/client.cpp
  src/frontend_data_variable.cpp
  src/frontend_layer.cpp
  src/ios_reference_data_variable.cpp
  src/ios_helper.cpp
)
target_include_directories(nimblenet PUBLIC "${PROJECT_SOURCE_DIR}/platform/ios/include/")
