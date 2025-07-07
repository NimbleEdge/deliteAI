add_library(miniz INTERFACE)
target_include_directories(miniz INTERFACE ./)
target_compile_definitions(miniz INTERFACE MINIZ_NO_ZLIB_COMPATIBLE_NAMES)
target_sources(miniz INTERFACE miniz.c)
