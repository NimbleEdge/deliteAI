target_compile_definitions(nimblenet PUBLIC -DSCRIPTING)

target_sources(nimblenet ${VISIBILITY}
    task/src/task.cpp
    operators/src/bool_operators.cpp
    operators/src/unary_operators.cpp
    operators/src/compare_operators.cpp
    operators/src/custom_functions.cpp
    operators/src/list_operators.cpp
    task/src/dp_module.cpp
    task/src/node.cpp
    task/src/statements.cpp
    task/src/variable_scope.cpp
)

target_include_directories(nimblenet ${VISIBILITY} "${PROJECT_SOURCE_DIR}/nimblenet/task_manager/task_manager/include/"
    "${PROJECT_SOURCE_DIR}/nimblenet/task_manager/task/include/"
    "${PROJECT_SOURCE_DIR}/nimblenet/task_manager/operators/include/"
    "${PROJECT_SOURCE_DIR}/nimblenet/task_manager/other_data_variables/include/")
