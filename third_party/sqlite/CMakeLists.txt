add_library(sqlite3 INTERFACE)
target_include_directories(sqlite3 INTERFACE ./)
target_sources(sqlite3 INTERFACE sqlite3.c)
