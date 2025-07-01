add_executable(nimble_client src/main.cpp)
target_include_directories(nimble_client PUBLIC ../nimblenet/simulation_manager/input_data_extractor/include/)
target_link_libraries(nimble_client ${VISIBILITY} nimblenet nlohmann_json::nlohmann_json)
