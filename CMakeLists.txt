cmake_minimum_required(VERSION 3.14)
project(VoltScript VERSION 0.8.6 LANGUAGES CXX)  # Optimization & Refinement Release

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generate version header from project version
configure_file(
    ${CMAKE_SOURCE_DIR}/src/version.h.in
    ${CMAKE_BINARY_DIR}/generated/version.h
    @ONLY
)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings and optimizations
if(MSVC)
    add_compile_options(/W4 /permissive- /O2 /Oi /Ot /GL)
    add_link_options(/LTCG)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native -flto)
    add_link_options(-flto)
endif()

# Sanitizers for Debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        # MSVC Address Sanitizer support (VS 2019 16.9+)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -fsanitize=undefined)
        add_link_options(-fsanitize=address -fsanitize=undefined)
    endif()
endif()

# Options
option(BUILD_TESTS "Build tests" ON)

# Source files
file(GLOB_RECURSE VOLT_SOURCES
    src/*.cpp
    src/features/*.cpp  # Include feature files
)

# Main executable
add_executable(volt
    ${VOLT_SOURCES}
)

if(MSVC)
    set_target_properties(volt PROPERTIES LINK_FLAGS "/STACK:33554432")
endif()

target_include_directories(volt PRIVATE 
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
)

# Set output directory
set_target_properties(volt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ========================================
# TESTS
# ========================================

if(BUILD_TESTS)
    # Fetch GoogleTest
    include(FetchContent)
    message(STATUS "Fetching Google Test...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    # Test sources
    set(TEST_SOURCES
        src/lexer/token.cpp
        src/lexer/lexer.cpp
        src/parser/ast.cpp
        src/parser/parser.cpp
        src/interpreter/value.cpp
        src/interpreter/environment.cpp
        src/features/callable.cpp
        src/interpreter/interpreter.cpp
        src/interpreter/module.cpp
        src/interpreter/natives/native_math.cpp
        src/interpreter/natives/native_string.cpp
        src/interpreter/natives/native_array.cpp
        src/interpreter/natives/native_io.cpp
        src/interpreter/natives/native_time.cpp
        src/interpreter/natives/native_json.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        tests/test_lexer.cpp
        tests/test_parser.cpp
        tests/test_evaluator.cpp
        tests/test_interpreter.cpp
        tests/test_functions.cpp
        tests/test_enhanced_features.cpp
        tests/test_arrays.cpp
        tests/test_error_reporting.cpp
        tests/test_run_until.cpp
        tests/test_hash_maps.cpp
        tests/test_improved_hash_maps.cpp
        tests/test_string_functions.cpp
        tests/test_math_functions.cpp
        tests/test_string_enhancements.cpp
        tests/test_v075_simple.cpp
        tests/test_new_features.cpp
        tests/test_array_advanced.cpp
        tests/test_string_file_functions.cpp
        tests/test_arrays_extended.cpp
        tests/test_interpreter_edge_cases.cpp
        tests/test_builtin_functions.cpp
        tests/test_basic_arithmetic.cpp
        tests/test_variable_operations.cpp
        tests/test_control_flow.cpp
        tests/test_string_operations.cpp
        tests/test_classes.cpp
        tests/test_json.cpp
        tests/test_performance.cpp
    )
    
    # Test executable
    add_executable(volt_tests ${TEST_SOURCES})

    if(MSVC)
        set_target_properties(volt_tests PROPERTIES LINK_FLAGS "/STACK:8388608")
    endif()
    
    target_include_directories(volt_tests PRIVATE 
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/lexer
        ${CMAKE_SOURCE_DIR}/src/parser
        ${CMAKE_SOURCE_DIR}/src/interpreter
        ${CMAKE_SOURCE_DIR}/src/features
    )
    
    # Link GoogleTest
    target_link_libraries(volt_tests 
        GTest::gtest_main
        GTest::gmock_main
    )
    
    # Set output directory for tests
    set_target_properties(volt_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(volt_tests)
    
    message(STATUS "✅ Tests enabled (566 tests)")

    # ========================================
    # INTEGRATION TESTS (Phase 3.2)
    # ========================================
    
    set(INTEGRATION_TEST_SOURCES
        tests/integration_tests.cpp
        src/lexer/token.cpp
        src/lexer/lexer.cpp
        src/parser/ast.cpp
        src/parser/parser.cpp
        src/interpreter/value.cpp
        src/interpreter/environment.cpp
        src/features/callable.cpp
        src/interpreter/interpreter.cpp
        src/interpreter/natives/native_math.cpp
        src/interpreter/natives/native_string.cpp
        src/interpreter/natives/native_array.cpp
        src/interpreter/natives/native_io.cpp
        src/interpreter/natives/native_time.cpp
        src/interpreter/natives/native_json.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        src/interpreter/module.cpp
    )
    
    add_executable(volt_integration_tests ${INTEGRATION_TEST_SOURCES})
    
    target_include_directories(volt_integration_tests PRIVATE 
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/lexer
        ${CMAKE_SOURCE_DIR}/src/parser
        ${CMAKE_SOURCE_DIR}/src/interpreter
        ${CMAKE_SOURCE_DIR}/src/features
    )
    
    target_link_libraries(volt_integration_tests 
        GTest::gtest_main
    )
    
    set_target_properties(volt_integration_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    gtest_discover_tests(volt_integration_tests)

    # ========================================
    # BENCHMARKS (Phase 3.3)
    # ========================================
    
    message(STATUS "Fetching Google Benchmark...")
    FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
    )
    FetchContent_MakeAvailable(googlebenchmark)
    
    set(BENCHMARK_SOURCES
        benchmarks/benchmark_interpreter.cpp
        src/lexer/token.cpp
        src/lexer/lexer.cpp
        src/parser/ast.cpp
        src/parser/parser.cpp
        src/interpreter/value.cpp
        src/interpreter/environment.cpp
        src/features/callable.cpp
        src/interpreter/interpreter.cpp
        src/interpreter/natives/native_math.cpp
        src/interpreter/natives/native_string.cpp
        src/interpreter/natives/native_array.cpp
        src/interpreter/natives/native_io.cpp
        src/interpreter/natives/native_time.cpp
        src/interpreter/natives/native_json.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        src/interpreter/module.cpp
    )
    
    add_executable(volt_benchmarks ${BENCHMARK_SOURCES})
    
    target_include_directories(volt_benchmarks PRIVATE 
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/lexer
        ${CMAKE_SOURCE_DIR}/src/parser
        ${CMAKE_SOURCE_DIR}/src/interpreter
        ${CMAKE_SOURCE_DIR}/src/features
    )
    
    target_link_libraries(volt_benchmarks benchmark::benchmark_main)
    
    set_target_properties(volt_benchmarks PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# ========================================
# SUMMARY
# ========================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "⚡ VoltScript v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "Milestone:      Build System Improvements 🚀")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:       ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests:    ${BUILD_TESTS}")
message(STATUS "========================================")
