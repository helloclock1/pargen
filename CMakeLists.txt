cmake_minimum_required(VERSION 3.31)
project(ParserGenerator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(Boost REQUIRED COMPONENTS program_options)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.0
)
FetchContent_MakeAvailable(Catch2)

enable_testing()

add_library(pargen_lib
    src/pargen/Automaton.cpp
    src/pargen/BNFParser.cpp
    src/pargen/Entities.cpp
    src/pargen/GrammarAnalyzer.cpp
    src/pargen/Helpers.cpp
    src/pargen/TableBuilder.cpp
)
add_library(codegen_lib
    src/codegen/CodeGenerator.cpp
    src/codegen/LexerGenerator.cpp
    src/codegen/ParserGenerator.cpp
)

target_include_directories(pargen_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/pargen)
target_include_directories(codegen_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pargen
    ${CMAKE_CURRENT_SOURCE_DIR}/include/codegen
)

add_executable(gen apps/main.cpp)
add_executable(tests
    test/TestHelpers.cpp
    test/TestBNFParser.cpp
    test/TestGrammarAnalyzer.cpp
    test/TestAutomaton.cpp
    test/TestTableBuilder.cpp
)

option(ENABLE_COVERAGE "Generate coverage report" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        foreach(target tests pargen_lib codegen_lib)
            target_compile_options(${target} PRIVATE --coverage -O0 -g)
            target_link_options(${target} PRIVATE --coverage)
        endforeach()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        foreach(target tests pargen_lib codegen_lib)
            target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping -O0 -g)
            target_link_options(${target} PRIVATE -fprofile-instr-generate --coverage)
        endforeach()
    else()
        message(FATAL_ERROR "Coverage report generation is not supported by this compiler")
    endif()
endif()

target_link_libraries(gen PRIVATE pargen_lib codegen_lib Boost::program_options)
target_link_libraries(tests PRIVATE pargen_lib codegen_lib Catch2::Catch2WithMain)

target_compile_options(gen PRIVATE -Werror -Wall -Wextra -Wpedantic -g)

include(CTest)
include(Catch)
catch_discover_tests(tests)
