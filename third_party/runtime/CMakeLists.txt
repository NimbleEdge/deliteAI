# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

set(TEMP_EXEC_INCLUDES "")
set(TEMP_EXEC_LIBS "")
set(TEMP_EXEC_DIR "")

if(TORCH_EXECUTOR)
	list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/include/")
	list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/libtorch.so
		${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/libtorch_cpu.so
		${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/libc10.so
		${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/libcpuinfo.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/libclog.a)
	list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/libtorch/unix/lib/")
endif()
if(TFLITE_EXECUTOR)
	message("TFLite Lib Used")
	list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/tflite/unix/include/")
	list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/tflite/unix/libs/libflatbuffers.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/tflite/unix/libs/libtensorflowlite.so)
	list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/tflite/unix/libs/")
endif()
if(ONNX_EXECUTOR)
	# Common code for Linux and OSX
	list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/lib/")
	list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/include/")

	if(MACOS)
		list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/lib/libonnxruntime.dylib)
		if(ORT_EXTENSIONS)
			list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/lib/libortextensions.dylib)
		endif()
	else()
		list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/lib/libonnxruntime.so)
		if(ORT_EXTENSIONS)
			list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/unix/lib/libortextensions.so)
		endif()
	endif()
endif()
if(ONNXGENAI_EXECUTOR)
		list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/onnxgenai/unix/include/")
		list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/onnxgenai/unix/lib/")
		if(MACOS)
			list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnxgenai/unix/lib/libonnxruntime-genai.dylib)
		else()
			list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnxgenai/unix/lib/libonnxruntime-genai.so)
		endif()
endif()
if(MNN_EXECUTOR)
	list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/mnn/unix/include/")
	list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/mnn/unix/lib/Release/")
	if(MACOS)
		list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/mnn/unix/lib/Release/libMNN.dylib)
	else()
		list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/mnn/unix/lib/Release/libMNN.so)
	endif()
endif()
if(EXECUTORCH_EXECUTOR)
	# TODO: Need to build from source and then add .a files here
	list(APPEND TEMP_EXEC_INCLUDES "${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/Headers/")
	list(APPEND TEMP_EXEC_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libexecutorch_macos.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libexecutorch_core.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libllama_runner.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libkernels_optimized_macos.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libportable_kernels.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/libportable_ops_lib.a
		${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/liboptimized_portable_kernels.a)
	list(APPEND TEMP_EXEC_DIR "${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/unix/lib/")
endif()

# ## setting globals to use later
set(BACKEND_LIBS ${TEMP_EXEC_LIBS} CACHE INTERNAL "")
set(BACKEND_INCLUDES ${TEMP_EXEC_INCLUDES} CACHE INTERNAL "")
set(BACKEND_DIR ${TEMP_EXEC_DIR} CACHE INTERNAL "")
