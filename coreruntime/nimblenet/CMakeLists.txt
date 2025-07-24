# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

set(BASE
	server_api/src/server_api_structs.cpp
	resource_manager/src/resource_manager.cpp
	resource_manager/include/resource_manager.hpp
	asset_manager/include/asset_manager.hpp
	resource_loader/include/resource_loader.hpp
	resource_loader/include/resource_downloader.hpp
	resource_loader/src/resource_downloader.cpp
	resource_loader/src/resource_loader.cpp
	resource_manager/include/resource_manager_structs.hpp
	config_manager/src/config_manager.cpp
	config_manager/include/config_manager.hpp
	time_manager/src/time_manager.cpp
	core_sdk/src/core_sdk.cpp
	core_sdk/src/extra_core_sdk.cpp
	core_sdk/src/nimble_exec_info.cpp
	command_center/src/command_center.cpp
	native_interface/src/native_interface.cpp
	data_variable/src/custom_func_data_variable.cpp
	data_variable/src/data_variable.cpp
	data_variable/src/dataframe_variable.cpp
	data_variable/src/enumerate_data_variable.cpp
	data_variable/src/filtered_dataframe_variable.cpp
	data_variable/src/future_data_variable.cpp
	data_variable/src/list_data_variable.cpp
	data_variable/src/map_data_variable.cpp
	data_variable/src/match_object_data_variable.cpp
	data_variable/src/model_nimble_net_variable.cpp
	data_variable/src/nimble_net_data_variable.cpp
	data_variable/src/nimble_net_internal_data_variable.cpp
	data_variable/src/pre_processor_nimble_net_variable.cpp
	data_variable/src/raw_event_store_data_variable.cpp
	data_variable/src/regex_data_variable.cpp
	data_variable/src/single_variable.cpp
	data_variable/src/tensor_data_variable.cpp
	job_scheduler/src/base_job.cpp
	job_scheduler/src/job_scheduler.cpp
	job_scheduler/src/asset_download_job.cpp
	job_scheduler/src/asset_load_job.cpp
	job_scheduler/src/script_load_job.cpp
	job_scheduler/src/script_ready_job.cpp
	asset_manager/src/asset_manager.cpp
	user_events/aggregate_column/src/count_column.cpp
	user_events/aggregate_column/src/sum_column.cpp
	user_events/aggregate_column/src/average_column.cpp
	user_events/aggregate_column/src/min_column.cpp
	user_events/aggregate_column/src/max_column.cpp
	user_events/pre_processor/include/pre_processor.hpp
	user_events/pre_processor/src/pre_processor.cpp
	user_events/table_store/include/table_store.hpp
	user_events/table_store/src/table_store.cpp
	user_events/raw_store/src/raw_store.cpp
	user_events/rolling_window/src/time_based_rolling_window.cpp
	user_events/rolling_window/src/rolling_window.cpp
	user_events/user_events_manager/src/user_events_manager.cpp
	user_events/user_events_manager/src/user_events_struct.cpp
	util/src/json.cpp
	util/src/logger.cpp
	util/src/log_sender.cpp
	util/src/util.cpp
)

if (NOT MINIMAL_BUILD)
	list(APPEND BASE
		data_variable/src/concurrent_executor_variable.cpp
		util/src/thread_pool.cpp
	)
endif()

set(INCLUDES
	"${PROJECT_SOURCE_DIR}/nimblenet/resource_manager/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/core_sdk/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/command_center/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/config_manager/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/logger/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/native_interface/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/job_scheduler/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/util/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/data_variable/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/aggregate_column/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/pre_processor/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/rolling_window/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/user_events_manager/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/table_store/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/user_events/raw_store/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/asset_manager/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/resource_loader/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/time_manager/include/")

if(IOS OR TESTING)
	add_library(nimblenet STATIC ${BASE})
else()
	add_library(nimblenet SHARED ${BASE})
endif()

target_compile_options(nimblenet PUBLIC -Werror=unused-result)

target_compile_definitions(nimblenet PRIVATE LIBNIMBLENET_NAME="$<TARGET_FILE_NAME:nimblenet>")

include(FindGit)

if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE git_rev
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process(COMMAND ${GIT_EXECUTABLE} symbolic-ref --short HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		OUTPUT_VARIABLE git_branch
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	message(FATAL "Git version not found, refusing to build")
endif()

set(CONFIG_FILE "${PROJECT_SOURCE_DIR}/../config.yml")
# Run the sed command
execute_process(
    COMMAND sed -n "s/.*sdk_version:[[:space:]]*\"\\([^\"]*\\)\".*/\\1/p" ${CONFIG_FILE}
    OUTPUT_VARIABLE SDK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(nimblenet PUBLIC
	NIMBLE_GIT_REV="${git_rev}"
	NIMBLE_GIT_BRANCH="${git_branch}"

	# Manual SDK Version to be used by ServerAPI and other resources to either send to NimbleCloud for
	# different purposes. Need to be bumped on any API contract change
	SDKVERSION="${SDK_VERSION}"
)

if(SCRIPTING)
	add_subdirectory(task_manager)
endif()

if(GENAI)
	target_compile_definitions(nimblenet PUBLIC -DGENAI)

	target_sources(nimblenet ${VISIBILITY}
		data_variable/src/stream_data_variable.cpp
		data_variable/src/llm_data_variable.cpp
		stream/src/char_stream.cpp
		stream/src/json_stream.cpp
		stream/src/dummy_offloaded_stream.cpp
		retriever/src/retriever.cpp
		util/src/llm_utils.cpp
	)

	target_include_directories(nimblenet ${VISIBILITY}
		"${PROJECT_SOURCE_DIR}/nimblenet/stream/include/"
		"${PROJECT_SOURCE_DIR}/nimblenet/retriever/include/"
	)
endif()

if(ORT_EXTENSIONS)
	target_compile_definitions(nimblenet PUBLIC -DORT_EXTENSIONS)
endif()

set_target_properties(nimblenet PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_include_directories(nimblenet ${VISIBILITY} ${INCLUDES})
add_subdirectory(executors)

if(GENAI)
	add_subdirectory(llm_executors)
endif()

# target_sources(nimblenet ${VISIBILITY} nimblenet/src/nimblenet.cpp)
add_subdirectory(server_api)

if(NOSQL)
	target_sources(nimblenet ${VISIBILITY} util/src/file_store.cpp
		database/src/database.cpp)
	target_include_directories(nimblenet ${VISIBILITY} "${PROJECT_SOURCE_DIR}/nimblenet/database/include/")
else()
	add_subdirectory("${PROJECT_SOURCE_DIR}/../third_party/sqlite" "${CMAKE_BINARY_DIR}/third_party/sqlite")
	target_sources(nimblenet ${VISIBILITY} pre_processor_database_manager/src/pre_processor_database_manager.cpp)
	target_include_directories(nimblenet ${VISIBILITY} "${PROJECT_SOURCE_DIR}/nimblenet/pre_processor_database_manager/include/")
	target_link_libraries(nimblenet PRIVATE sqlite3)
endif()

install(TARGETS nimblenet LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/)
target_include_directories(nimblenet PUBLIC
	"${PROJECT_SOURCE_DIR}/commonclient/include/"
	"${PROJECT_SOURCE_DIR}/nimblenet/cross_platform/include/")
