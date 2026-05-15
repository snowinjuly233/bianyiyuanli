cmake_minimum_required(VERSION 3.20)

project(CompilerLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(common STATIC
    src/common/source.cpp
    src/common/token.cpp
    src/common/diagnostic.cpp
    src/common/string_utils.cpp
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(seulex_core STATIC
    src/seulex/lex_spec.cpp
    src/seulex/lex_spec_parser.cpp
    src/seulex/regex_parser.cpp
    src/seulex/regex_normalizer.cpp
    src/seulex/nfa.cpp
    src/seulex/dfa.cpp
    src/seulex/dfa_minimizer.cpp
    src/seulex/scanner_emitter.cpp
)
target_include_directories(seulex_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(seulex_core PUBLIC common)

add_library(runtime_lib STATIC
    src/runtime/scanner_runtime.cpp
    src/runtime/parser_runtime.cpp
)
target_include_directories(runtime_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(runtime_lib PUBLIC common)

add_library(semantic_lib STATIC
    src/semantic/ast.cpp
    src/semantic/ast_printer.cpp
    src/semantic/ast_factory.cpp
    src/semantic/types.cpp
    src/semantic/symbol_table.cpp
    src/semantic/semantic_analyzer.cpp
    src/semantic/semantic_action_executor.cpp
)
target_include_directories(semantic_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(semantic_lib PUBLIC common)

add_library(seuyacc_core STATIC
    src/seuyacc/yacc_spec.cpp
    src/seuyacc/yacc_spec_parser.cpp
    src/seuyacc/grammar.cpp
    src/seuyacc/first_set.cpp
    src/seuyacc/lr1.cpp
    src/seuyacc/lalr.cpp
    src/seuyacc/parse_table.cpp
    src/seuyacc/parser_emitter.cpp
)
target_include_directories(seuyacc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(seuyacc_core PUBLIC common semantic_lib runtime_lib)

add_library(ir_lib STATIC
    src/ir/basic_block.cpp
    src/ir/quad.cpp
    src/ir/ir_builder.cpp
    src/ir/ir_printer.cpp
)
target_include_directories(ir_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ir_lib PUBLIC common semantic_lib)

add_executable(seulex src/tools/seulex_main.cpp)
target_link_libraries(seulex PRIVATE common seulex_core runtime_lib)

add_executable(seuyacc src/tools/seuyacc_main.cpp)
target_link_libraries(seuyacc PRIVATE common seuyacc_core seulex_core runtime_lib)

add_executable(frontend src/tools/frontend_main.cpp)
target_link_libraries(frontend PRIVATE common seulex_core seuyacc_core runtime_lib semantic_lib ir_lib)
