cmake_minimum_required(VERSION 3.20)

project(minic_generated_frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMPILERLAB_ROOT "..")

add_library(compilerlab_common STATIC
    ${COMPILERLAB_ROOT}/src/common/source.cpp
    ${COMPILERLAB_ROOT}/src/common/token.cpp
    ${COMPILERLAB_ROOT}/src/common/diagnostic.cpp
    ${COMPILERLAB_ROOT}/src/common/string_utils.cpp
)
target_include_directories(compilerlab_common PUBLIC ${COMPILERLAB_ROOT}/src)

add_library(compilerlab_runtime STATIC
    ${COMPILERLAB_ROOT}/src/runtime/parser_runtime.cpp
)
target_include_directories(compilerlab_runtime PUBLIC ${COMPILERLAB_ROOT}/src)
target_link_libraries(compilerlab_runtime PUBLIC compilerlab_common)

add_library(compilerlab_semantic STATIC
    ${COMPILERLAB_ROOT}/src/semantic/ast.cpp
    ${COMPILERLAB_ROOT}/src/semantic/ast_factory.cpp
    ${COMPILERLAB_ROOT}/src/semantic/types.cpp
    ${COMPILERLAB_ROOT}/src/semantic/semantic_action_executor.cpp
)
target_include_directories(compilerlab_semantic PUBLIC ${COMPILERLAB_ROOT}/src)
target_link_libraries(compilerlab_semantic PUBLIC compilerlab_common compilerlab_runtime)

add_executable(minic_scanner
    minic_scanner.cpp
    minic_scanner_main.cpp
)
target_include_directories(minic_scanner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(minic_parser
    minic_scanner.cpp
    minic_parser.cpp
    minic_parser_main.cpp
)
target_include_directories(minic_parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${COMPILERLAB_ROOT}/src)
target_link_libraries(minic_parser PRIVATE compilerlab_common compilerlab_runtime compilerlab_semantic)
