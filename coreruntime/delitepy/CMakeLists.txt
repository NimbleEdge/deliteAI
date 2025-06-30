cmake_minimum_required(VERSION 3.18)

project(DelitePy
  LANGUAGES
)

add_custom_target(delitepy_docs
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_docs.sh"
          "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM
)
