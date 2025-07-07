# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

if(ONNXGENAI_EXECUTOR)
  target_compile_definitions(nimblenet PUBLIC -DONNXGENAI_EXECUTOR)
endif()
if(EXECUTORCH_EXECUTOR)
  target_compile_definitions(nimblenet PUBLIC -DEXECUTORCH_EXECUTOR)
endif()

target_include_directories(nimblenet ${VISIBILITY} include/)
target_sources(nimblenet ${VISIBILITY}
  src/base_llm_executor.cpp
)

if(GEMINI)
  target_sources(nimblenet ${VISIBILITY} src/gemini_nano_executor.cpp)
endif()

if(ONNXGENAI_EXECUTOR)
  target_sources(nimblenet ${VISIBILITY} src/onnx_llm_executor.cpp)
endif()

if(EXECUTORCH_EXECUTOR)
  target_sources(nimblenet ${VISIBILITY} src/executorch_llm_executor.cpp)
endif()
