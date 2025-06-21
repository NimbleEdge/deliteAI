# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_sources(nimblenet ${VISIBILITY}
	executor/include/model_executor_structs.hpp
	executor/src/model_executor_structs.cpp)

if(ONNX_EXECUTOR)
	message("Using ONNX_EXECUTOR")
	add_subdirectory(onnx)
else()
	message(FATAL_ERROR "Only Onnx executor is supported for running traditional ML models.")
endif()

target_include_directories(nimblenet ${VISIBILITY} executor/include/ model/include/)
