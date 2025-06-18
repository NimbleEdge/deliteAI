# SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
#
# SPDX-License-Identifier: Apache-2.0

# The OBJECT library type defines a non-archival collection of object files resulting
# from compiling the given source files.
# Linking against an OBJECT library links these objects directly

# This is done so that when libnimblenet is a static library (as in IOS), we link these
# objects directly into the library. It's an issue if coreutil is a static library.
# This is because building a static library doesn't actually invoke the linker and the
# target_link_libraries is just carrying the dependencies with it. When the final executable/shared
# object is built, then coreutil isn't linked as CMake dependency is not propagated

add_library(core_utils OBJECT
${CMAKE_CURRENT_SOURCE_DIR}/src/ne_md5.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/shard.cpp
)
target_include_directories(core_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET core_utils PROPERTY POSITION_INDEPENDENT_CODE ON)
