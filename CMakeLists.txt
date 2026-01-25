cmake_minimum_required(VERSION 3.14)
project(VoltScript VERSION 0.6.8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTS "Build tests" ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
)

# Source files
set(SOURCES
    src/main.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
    src/interpreter/value.cpp
    src/interpreter/environment.cpp
    src/features/callable.cpp
    src/interpreter/interpreter.cpp
    src/features/array.cpp
)

# Main executable
add_executable(volt ${SOURCES})

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
        src/features/array.cpp
        tests/test_lexer.cpp
        tests/test_parser.cpp
        tests/test_evaluator.cpp
        tests/test_interpreter.cpp
        tests/test_functions.cpp
        tests/test_enhanced_features.cpp
        tests/test_arrays.cpp
        tests/test_error_reporting.cpp
    )
    
    # Test executable
    add_executable(volt_tests ${TEST_SOURCES})
    
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
    
    message(STATUS "✅ Tests enabled (261 tests)")
endif()

# ========================================
# SUMMARY
# ========================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "⚡ VoltScript v${PROJECT_VERSION}")
message(STATUS "========================================")
message(STATUS "Milestone:      Arrays & Enhanced Features")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:       ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests:    ${BUILD_TESTS}")
message(STATUS "========================================")
