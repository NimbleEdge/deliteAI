# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

add_executable(nimble_client)

target_sources(nimble_client
 PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

target_include_directories(nimble_client
 PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/../nimblenet/simulation_manager/input_data_extractor/include"
)

target_link_libraries(nimble_client
 "${VISIBILITY}"
  nimblenet
  nlohmann_json::nlohmann_json
)
