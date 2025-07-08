# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

target_compile_definitions(nimblenet PUBLIC -DSIMULATION_MODE )
set(PYBIND11_PYTHON_VERSION ${PYTHON_VERSION})
find_package(pybind11 CONFIG REQUIRED)

target_include_directories(nimblenet PUBLIC 
    ${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/include/
    "${PROJECT_SOURCE_DIR}/../third_party/json/include/"
)

target_sources(nimblenet ${VISIBILITY}
    ${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/src/input_data_extractor.cpp 
    ${PROJECT_SOURCE_DIR}/nimblenet/simulation_manager/input_data_extractor/src/c_tensors_data_extractor.cpp
    ${PROJECT_SOURCE_DIR}/../nimblenet_py/simulator_binder/delitepy_script_parser.cpp
)

target_link_libraries(nimblenet PRIVATE pybind11::module)

pybind11_add_module(simulator ../../../nimblenet_py/simulator_binder/binder.cpp ../../../nimblenet_py/simulator_binder/binder_v2.cpp)
install(TARGETS simulator LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install(DIRECTORY ${BACKEND_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/ FILES_MATCHING PATTERN "*")
target_link_libraries(simulator ${VISIBILITY} nimblenet nlohmann_json::nlohmann_json)
