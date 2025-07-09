# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.18)

project(Core_Runtime)

set(CMAKE_CXX_STANDARD 17)
include(CMakePrintHelpers)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bin/")

# In MacOS $ORIGIN is not natively supported, rather @loader_path and @executable_path are to specify relative path to libraries
if(APPLE)
	set(CMAKE_MACOSX_RPATH TRUE)
	list(APPEND CMAKE_INSTALL_RPATH "@loader_path/" "@executable_path/")
else()
	set(CMAKE_INSTALL_RPATH "\$ORIGIN")
endif()

set(JSON_BuildTests OFF CACHE INTERNAL "")

if(NOT ANDROID_ABI)
	find_package(ZLIB REQUIRED)

	if(ZLIB_FOUND)
		message("Found zlib: ${ZLIB_INCLUDE_DIRS}")
	else()
		message(FATAL_ERROR "zlib not found. Please install zlib.")
	endif()
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address") #used to switch on address sanitizer
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread") #used to switch on thread sanitizer

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-4") #used to switch on valgrind

# set(DEBUGFLAGS " -Werror -Wno-write-strings  -Weffc++ -Wall -Wuninitialized -Wnon-virtual-dtor -Wshadow -Werror=format-security -Wunused-member-function -Wunused-function ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=switch -Werror=return-type -Werror=implicit-fallthrough \
	-Werror=non-virtual-dtor -Werror=format -Werror=format-security -Werror=unused-member-function -Werror=unused-function -Werror=writable-strings")

# string(CONCAT RELEASEFLAGS ${DEBUGFLAGS} " -fstack-protector-strong -ffunction-sections -fdata-sections ")
# #
# # -fvisibility=hidden to be looked at in future
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
# set(CMAKE_CXX_FLAGS ${DEBUGFLAGS})

# else()
# set(CMAKE_CXX_FLAGS ${RELEASEFLAGS})
# endif()
set(VISIBILITY "PRIVATE")

if(TESTING)
	message("Creating Test build for Core Runtime")
	set(VISIBILITY "PUBLIC")

	if(SIMULATION_MODE)
		message(FATAL_ERROR "You can not do this at all, Enabling Testing and Simulation together CMake will exit.")
	endif()
endif()

# This potentially helps in reducing the size of eh_frame section but is not working currently
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-asynchronous-unwind-tables")

# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Oz -flto=full -g -DNDEBUG")

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	message("Adding gc section flags")
	add_compile_options(-ffunction-sections -fdata-sections)
	if (APPLE)
		# Ref: https://stackoverflow.com/questions/24734409/make-error-in-mac-clang-ld-unknown-option-gc-sections
		add_link_options(-Wl,-dead_strip)
	else()
		add_link_options(-Wl,--gc-sections)
	endif()
endif()

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Oz -g -DNDEBUG")

# Disable LTO for ONNX because otherwise we get a SEGFAULT from onnx library, internally it calls GetApi() which returns nullptr

add_subdirectory(nimblenet)
add_subdirectory(delitepy)
add_subdirectory("../third_party/json" "${CMAKE_BINARY_DIR}/third_party/json")
add_subdirectory("../third_party/SPSCQueue" "${CMAKE_BINARY_DIR}/third_party/SPSCQueue")
if (GENAI)
	add_subdirectory("../third_party/miniz" "${CMAKE_BINARY_DIR}/third_party/miniz")
endif()

add_subdirectory(core_utils)
target_link_libraries(nimblenet ${VISIBILITY} core_utils)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("Adding ALLOW_VERBOSE_LOGGING flag")
	target_compile_definitions(nimblenet PUBLIC -DALLOW_VERBOSE_LOGGING)
endif()

if (MINIMAL_BUILD)
	message("Adding MINIMAL_BUILD flag")
	target_compile_definitions(nimblenet PUBLIC -DMINIMAL_BUILD)
endif()

if(THREADSLEEPTIME)
	message("USING THREADSLEEPTIME=${THREADSLEEPTIME}")
	target_compile_definitions(nimblenet PUBLIC -DTHREADSLEEPTIME=${THREADSLEEPTIME})
endif()

if(REGEX_ENABLED)
	message("USING REGEX_ENABLED")
	target_compile_definitions(nimblenet PUBLIC -DREGEX_ENABLED=1)
endif()

if(ANDROID_ABI)
	message("USING ANDROID_ABI")
	add_subdirectory(platform/android)
	target_compile_definitions(nimblenet PUBLIC -DANDROID_ABI)

	if(NNAPI)
		target_compile_definitions(nimblenet PUBLIC -DNNAPI=1)
	endif()

	if(JNITESTING)
		target_compile_definitions(nimblenet PUBLIC -DJNITESTING=1)
	endif()
elseif(IOS)
	message("USING IOS")
	target_compile_definitions(nimblenet PUBLIC -DIOS=1)

	if(PLATFORM STREQUAL "OS64")
		message("PLATFORM is set to iOS device arm64")
		target_compile_definitions(nimblenet PUBLIC -DIOS_PLATFORM="")
		if (ONNX_EXECUTOR)
			set(BACKEND_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXBuild}/onnxruntime.xcframework/ios-arm64/onnxruntime.framework/onnxruntime CACHE INTERNAL "")
			set(BACKEND_INCLUDES ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXBuild}/onnxruntime.xcframework/ios-arm64/onnxruntime.framework/Headers/ CACHE INTERNAL "")
		endif()

		if(GENAI AND ONNXGENAI_EXECUTOR)
			set(BACKEND_LIBS "${BACKEND_LIBS}" ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXgenai}/onnxruntime-genai.xcframework/ios-arm64/onnxruntime-genai.framework/onnxruntime-genai CACHE INTERNAL "")
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXgenai}/onnxruntime-genai.xcframework/ios-arm64/onnxruntime-genai.framework/Headers/)
		endif()

		if(GENAI AND EXECUTORCH_EXECUTOR)
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/ios/${Executorchgenai}/Executorch_Headers/)
		endif()

		# ort extension
		if(ORT_EXTENSIONS)
			set(BACKEND_LIBS "${BACKEND_LIBS}" ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXext}/onnxruntime_extensions.xcframework/ios-arm64/onnxruntime_extensions.framework/onnxruntime_extensions CACHE INTERNAL "")
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXext}/onnxruntime_extensions.xcframework/ios-arm64/onnxruntime_extensions.framework/Headers/)
		endif()
	elseif(PLATFORM STREQUAL "SIMULATOR64COMBINED")
		message("PLATFORM is set to iOS simulator")
		target_compile_definitions(nimblenet PUBLIC -DIOS_PLATFORM="sim")
		if (ONNX_EXECUTOR)
			set(BACKEND_LIBS ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXBuild}/onnxruntime.xcframework/ios-arm64_x86_64-simulator/onnxruntime.framework/onnxruntime CACHE INTERNAL "")
			set(BACKEND_INCLUDES ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXBuild}/onnxruntime.xcframework/ios-arm64_x86_64-simulator/onnxruntime.framework/Headers/ CACHE INTERNAL "")
		endif()

		if(GENAI AND ONNXGENAI_EXECUTOR)
			set(BACKEND_LIBS "${BACKEND_LIBS}" ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXgenai}/onnxruntime-genai.xcframework/ios-arm64_x86_64-simulator/onnxruntime-genai.framework/onnxruntime-genai CACHE INTERNAL "")
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXgenai}/onnxruntime-genai.xcframework/ios-arm64_x86_64-simulator/onnxruntime-genai.framework/Headers)
		endif()

		if(GENAI AND EXECUTORCH_EXECUTOR)
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/executorch/ios/${Executorchgenai}/Executorch_Headers/)
		endif()

		if(ORT_EXTENSIONS)
			set(BACKEND_LIBS "${BACKEND_LIBS}" ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXext}/onnxruntime_extensions.xcframework/ios-arm64_x86_64-simulator/onnxruntime_extensions.framework/onnxruntime_extensions CACHE INTERNAL "")
			target_include_directories(nimblenet ${VISIBILITY} ${PROJECT_SOURCE_DIR}/../third_party/runtime/onnx/ios/${ONNXext}/onnxruntime_extensions.xcframework/ios-arm64_x86_64-simulator/onnxruntime_extensions.framework/Headers/)
		endif()
	else()
		message("PLATFORM is set to something else")
	endif()

	set(BACKEND_DIR ${TEMP_EXEC_DIR} CACHE INTERNAL "")
	add_subdirectory(platform/ios)
	target_link_libraries(nimblenet PRIVATE "-framework Foundation")
else()
	# Adding this IOS_PLATFORM works only for mac builds
	target_compile_definitions(nimblenet PUBLIC -DIOS_PLATFORM="mac")
	add_subdirectory(platform/unix) # produces ${CLIENT_INCLUDES}
	add_subdirectory("../third_party/runtime" "${CMAKE_BINARY_DIR}/third_party/runtime") # -> produces ${BACKEND_LIBS} ${BACKEND_DIR} ${BACKED_INCLUDES}
	target_link_libraries(nimblenet ${VISIBILITY} curl)

	# target_link_libraries(nimblenet ${VISIBILITY} clientlib)
endif()

# set additional libs on requirement basis depending on OS
set(ADDITIONAL_LIBS)

if(NOT ANDROID_ABI)
	list(APPEND ADDITIONAL_LIBS ZLIB::ZLIB)
endif()

target_link_libraries(nimblenet PRIVATE nlohmann_json::nlohmann_json ${VISIBILITY} SPSCQueue ${VISIBILITY} ${BACKEND_LIBS} ${VISIBILITY} ${ADDITIONAL_LIBS})
if (GENAI)
	target_link_libraries(nimblenet PRIVATE miniz)
endif()
target_include_directories(nimblenet ${VISIBILITY} ${BACKEND_INCLUDES} ${VISIBILITY} ${CLIENT_INCLUDES})

if(SIMULATION_MODE)
	add_subdirectory(nimblenet/simulation_manager)
	add_subdirectory(nimblenet/nimble_net)
elseif(TESTING)
	find_package(GTest REQUIRED) # added
	target_include_directories(nimblenet PUBLIC "${GTEST_INCLUDE_DIRS}") # added
	target_link_libraries(nimblenet PUBLIC ${GTEST_LIBRARIES} pthread) # added
	target_compile_definitions(nimblenet PUBLIC -DTESTING -DSENDLOGS)
	include_directories(${PROJECT_SOURCE_DIR}/tests/unittests)
	add_subdirectory(nimblenet/nimble_net)
	set(TESTSOURCES ${PROJECT_SOURCE_DIR}/tests/unittests/nimbletest.cpp
		${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/src/c_tensors_data_extractor.cpp
		${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/include/input_structs.hpp
		${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/src/input_data_extractor.cpp
		${PROJECT_SOURCE_DIR}/nimblenet/data_variable/src/data_variable.cpp
		${PROJECT_SOURCE_DIR}/nimblenet/data_variable/src/single_variable.cpp
		${PROJECT_SOURCE_DIR}/nimblenet/data_variable/src/tensor_data_variable.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/scripting_test.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/command_center_test.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/end_to_end_tests.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/util_test.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/add_event_end_to_end_test.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/native_interface_test.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/tests_util.cpp
		${PROJECT_SOURCE_DIR}/tests/unittests/tests_util_structs.cpp)
	set_target_properties(nimblenet PROPERTIES CXX_VISIBILITY_PRESET default)
	target_include_directories(nimblenet PUBLIC "${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/include/")
	add_executable(nimbletest ${TESTSOURCES})
	target_link_libraries(nimbletest PUBLIC nlohmann_json::nlohmann_json)

	if(GENAI)
		target_sources(nimbletest PUBLIC
			${PROJECT_SOURCE_DIR}/tests/unittests/stream_test.cpp
		)
		target_link_libraries(nimbletest PUBLIC miniz)
	endif()

	target_include_directories(nimbletest PUBLIC ${PROJECT_SOURCE_DIR}/tests/unittests)
	target_link_libraries(nimbletest ${VISIBILITY} nimblenet core_utils)
	file(COPY ${CMAKE_SOURCE_DIR}/tests/assets DESTINATION ${CMAKE_BINARY_DIR}/)
else()
	target_compile_definitions(nimblenet PUBLIC -DSENDLOGS)
	add_subdirectory(nimblenet/nimble_net) # adds C layer for production

	if(NOT DEFINED IOS AND NOT DEFINED ANDROID_ABI)
		add_subdirectory(nimble_client)
		add_subdirectory(nimble_diff)
		add_subdirectory(nimble_shard)
	endif()
endif()
