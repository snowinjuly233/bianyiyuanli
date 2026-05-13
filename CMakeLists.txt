cmake_minimum_required(VERSION 3.20)

project(compilerlab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(compilerlab_common STATIC
    src/common/diagnostic.cpp
    src/common/source.cpp
    src/common/string_utils.cpp
    src/common/token.cpp
)
target_include_directories(compilerlab_common PUBLIC src)

add_library(compilerlab_runtime STATIC
    src/runtime/parser_runtime.cpp
    src/runtime/scanner_runtime.cpp
)
target_include_directories(compilerlab_runtime PUBLIC src)
target_link_libraries(compilerlab_runtime PUBLIC compilerlab_common)

add_library(compilerlab_semantic STATIC
    src/semantic/ast.cpp
    src/semantic/ast_factory.cpp
    src/semantic/ast_printer.cpp
    src/semantic/semantic_action_executor.cpp
    src/semantic/semantic_analyzer.cpp
    src/semantic/symbol_table.cpp
    src/semantic/types.cpp
)
target_include_directories(compilerlab_semantic PUBLIC src)
target_link_libraries(compilerlab_semantic PUBLIC compilerlab_common compilerlab_runtime)

add_library(compilerlab_ir STATIC
    src/ir/basic_block.cpp
    src/ir/ir_builder.cpp
    src/ir/ir_printer.cpp
    src/ir/quad.cpp
)
target_include_directories(compilerlab_ir PUBLIC src)
target_link_libraries(compilerlab_ir PUBLIC compilerlab_common compilerlab_semantic)

add_library(compilerlab_seulex STATIC
    src/seulex/dfa.cpp
    src/seulex/dfa_minimizer.cpp
    src/seulex/lex_spec.cpp
    src/seulex/lex_spec_parser.cpp
    src/seulex/nfa.cpp
    src/seulex/regex_normalizer.cpp
    src/seulex/regex_parser.cpp
    src/seulex/scanner_emitter.cpp
)
target_include_directories(compilerlab_seulex PUBLIC src)
target_link_libraries(compilerlab_seulex PUBLIC compilerlab_common compilerlab_runtime)

add_library(compilerlab_seuyacc STATIC
    src/seuyacc/first_set.cpp
    src/seuyacc/grammar.cpp
    src/seuyacc/lalr.cpp
    src/seuyacc/lr1.cpp
    src/seuyacc/parse_table.cpp
    src/seuyacc/parser_emitter.cpp
    src/seuyacc/yacc_spec.cpp
    src/seuyacc/yacc_spec_parser.cpp
)
target_include_directories(compilerlab_seuyacc PUBLIC src)
target_link_libraries(compilerlab_seuyacc PUBLIC compilerlab_common compilerlab_runtime compilerlab_semantic)

add_executable(seulex src/tools/seulex_main.cpp)
target_link_libraries(seulex PRIVATE compilerlab_common compilerlab_runtime compilerlab_seulex)

add_executable(seuyacc src/tools/seuyacc_main.cpp)
target_link_libraries(seuyacc PRIVATE
    compilerlab_common
    compilerlab_runtime
    compilerlab_semantic
    compilerlab_seulex
    compilerlab_seuyacc
)

add_executable(frontend src/tools/frontend_main.cpp)
target_link_libraries(frontend PRIVATE
    compilerlab_common
    compilerlab_runtime
    compilerlab_semantic
    compilerlab_ir
    compilerlab_seulex
    compilerlab_seuyacc
)
