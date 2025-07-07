# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet ${VISIBILITY}
  src/client.cpp
  src/dependency_container_shadow.cpp
  src/frontend_data_variable.cpp
  src/hardware_info_shadow.cpp
  src/logs_upload_scheduler_shadow.cpp
  src/networking_shadow.cpp
)

if(GEMINI)
  target_compile_definitions(nimblenet PUBLIC -DGEMINI)
  target_sources(nimblenet ${VISIBILITY} src/gemini_nano_handler_shadow.cpp)
endif()

target_link_libraries(nimblenet PUBLIC android log)
target_include_directories(nimblenet PUBLIC "${PROJECT_SOURCE_DIR}/platform/android/include/")
