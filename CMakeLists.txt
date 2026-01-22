cmake_minimum_required(VERSION 3.20)
project(VoltScript VERSION 0.5.0 LANGUAGES CXX)

# C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /O2 /EHsc)
    add_compile_options(/wd4251 /wd4275)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
file(GLOB_RECURSE VOLT_SOURCES
    src/*.cpp
)

# Main executable
add_executable(volt
    src/main.cpp
    ${VOLT_SOURCES}
)

target_include_directories(volt PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Lexer
    ${CMAKE_SOURCE_DIR}/src/Parser
    ${CMAKE_SOURCE_DIR}/src/Interpreter
)

# ==============================================================================
# TESTING
# ==============================================================================
option(VOLT_BUILD_TESTS "Build tests" ON)

if(VOLT_BUILD_TESTS)
    message(STATUS "Fetching Google Test...")
    
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    
    # Create test executable
    add_executable(volt_tests
        tests/test_lexer.cpp
        tests/test_parser.cpp
        tests/test_evaluator.cpp
        tests/test_interpreter.cpp
        ${VOLT_SOURCES}
    )
    
    target_include_directories(volt_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(volt_tests GTest::gtest_main)
    
    # Add test manually (instead of gtest_discover_tests)
    add_test(NAME AllTests COMMAND volt_tests)
    
    message(STATUS "✅ Tests enabled")
else()
    message(STATUS "⚠️  Tests disabled")
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS "VoltScript Milestone 5 - Statements")
message(STATUS "========================================")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler:       ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests:    ${VOLT_BUILD_TESTS}")
message(STATUS "========================================")
