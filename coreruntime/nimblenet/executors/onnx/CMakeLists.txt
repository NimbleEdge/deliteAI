# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_compile_definitions(nimblenet PUBLIC -DONNX_EXECUTOR )
target_sources(nimblenet ${VISIBILITY} src/onnx_model.cpp
									   ../model/src/base_model.cpp
									   src/task_onnx_model.cpp
									   ../model/src/task_base_model.cpp)
target_include_directories(nimblenet ${VISIBILITY} include/)