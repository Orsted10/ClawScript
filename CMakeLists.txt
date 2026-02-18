cmake_minimum_required(VERSION 3.14)
project(VoltScript VERSION 1.0.0 LANGUAGES CXX)  # NaN-boxing, VM, JIT & Runtime Optimizations

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
    add_compile_options(/W4 /permissive-
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/Ot>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/arch:AVX2>
        $<$<CONFIG:Release>:/favor:INTEL64>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/RTC1>)
    add_link_options($<$<CONFIG:Release>:/LTCG>)
else()
    add_compile_options(-Wall -Wextra -Wpedantic $<$<CONFIG:Release>:-O3> $<$<CONFIG:Release>:-march=native> $<$<CONFIG:Release>:-flto> $<$<CONFIG:Debug>:-O0>)
    add_link_options($<$<CONFIG:Release>:-flto>)
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
option(VOLT_ENABLE_AOT "Enable LLVM-based AoT compilation (requires LLVM)" OFF)
option(VOLT_ENABLE_JIT "Enable adaptive JIT (experimental stubs)" ON)
option(ENABLE_ASAN "Enable AddressSanitizer for supported compilers" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for supported compilers" OFF)
option(ENABLE_MSAN "Enable MemorySanitizer for supported compilers" OFF)
option(ENABLE_LIBFUZZER "Enable libFuzzer targets (requires Clang)" OFF)

# Source files
file(GLOB_RECURSE VOLT_SOURCES
    src/*.cpp
    src/features/*.cpp  # Include feature files
)

if(NOT VOLT_ENABLE_AOT)
    file(GLOB VOLT_AOT_SOURCES "${CMAKE_SOURCE_DIR}/src/aot/*.cpp")
    list(REMOVE_ITEM VOLT_SOURCES ${VOLT_AOT_SOURCES})
endif()
if(NOT VOLT_ENABLE_JIT)
    list(FILTER VOLT_SOURCES EXCLUDE REGEX ".*/jit/.*")
endif()

if(VOLT_ENABLE_AOT)
    find_package(LLVM CONFIG QUIET)
    if(LLVM_FOUND)
        message(STATUS "✅ LLVM for AoT: ${LLVM_PACKAGE_VERSION}")
        llvm_map_components_to_libnames(LLVM_LIBS
            core
            support
            nativecodegen
            target
            asmparser
            asmprinter
            passes
            scalaropts
        )
    else()
        # Fallback: attempt to use llvm-config or provided LLVM_ROOT/LLVM_DIR
        if(NOT LLVM_FALLBACK_FOUND)
            find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config llvm-config.exe)
            if(LLVM_CONFIG_EXECUTABLE)
                execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --version OUTPUT_VARIABLE LLVM_CONFIG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
                execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir OUTPUT_VARIABLE LLVM_INCLUDEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
                execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
                execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core support nativecodegen target asmparser asmprinter passes scalaropts OUTPUT_VARIABLE LLVM_LIBS_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
                execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
                string(REPLACE " " ";" LLVM_LIBS_FALLBACK "${LLVM_LIBS_OUTPUT} ${LLVM_SYSTEM_LIBS}")
                set(LLVM_FALLBACK_FOUND ON)
                message(STATUS "✅ Using llvm-config fallback for AoT: ${LLVM_CONFIG_VERSION}")
            else()
                if(DEFINED LLVM_ROOT)
                    set(_LLVM_ROOT "${LLVM_ROOT}")
                elseif(DEFINED LLVM_DIR)
                    get_filename_component(_LLVM_ROOT "${LLVM_DIR}" DIRECTORY)
                endif()
                if(DEFINED _LLVM_ROOT)
                    set(LLVM_INCLUDEDIR "${_LLVM_ROOT}/include")
                    set(LLVM_LIBDIR "${_LLVM_ROOT}/lib")
                    find_library(LLVM_MONO_LIB NAMES LLVM PATHS "${LLVM_LIBDIR}" NO_DEFAULT_PATH)
                    if(LLVM_MONO_LIB)
                        set(LLVM_LIBS_FALLBACK "${LLVM_MONO_LIB}")
                        set(LLVM_FALLBACK_FOUND ON)
                        message(STATUS "✅ Using monolithic LLVM lib for AoT at ${LLVM_MONO_LIB}")
                    else()
                        file(GLOB LLVM_LIBS_ALL "${LLVM_LIBDIR}/*.lib")
                        if(LLVM_LIBS_ALL)
                            set(LLVM_LIBS_FALLBACK "${LLVM_LIBS_ALL}")
                            set(LLVM_FALLBACK_FOUND ON)
                            message(STATUS "✅ Using lib directory fallback for AoT at ${LLVM_LIBDIR}")
                        endif()
                    endif()
                elseif(DEFINED LLVM_LIBDIR)
                    get_filename_component(_LLVM_ROOT_FROM_LIB "${LLVM_LIBDIR}" DIRECTORY)
                    set(LLVM_INCLUDEDIR "${_LLVM_ROOT_FROM_LIB}/include")
                endif()
            endif()
            if(LLVM_FALLBACK_FOUND AND LLVM_INCLUDEDIR)
                if(EXISTS "${LLVM_INCLUDEDIR}/llvm/IR/IRBuilder.h")
                    message(STATUS "✅ Found LLVM headers for AoT at ${LLVM_INCLUDEDIR}")
                else()
                    message(WARNING "LLVM headers not found under ${LLVM_INCLUDEDIR}. AoT may fail to build.")
                endif()
            endif()
        endif()
        if(NOT LLVM_FOUND AND NOT LLVM_FALLBACK_FOUND)
            message(WARNING "LLVM not found for AoT. Building without AoT support.")
            set(VOLT_ENABLE_AOT OFF)
        endif()
    endif()
endif()
if(VOLT_ENABLE_JIT)
    find_package(LLVM CONFIG QUIET)
    if(LLVM_FOUND)
        message(STATUS "✅ LLVM for JIT: ${LLVM_PACKAGE_VERSION}")
        llvm_map_components_to_libnames(LLVM_JIT_LIBS
            core
            support
            nativecodegen
            target
            asmparser
            asmprinter
            passes
            scalaropts
            orcjit
            orcshared
            orctargetprocess
            jitlink
            mcjit
            executionengine
            irreader
            transformutils
            analysis
        )
    else()
        find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config llvm-config.exe)
        if(LLVM_CONFIG_EXECUTABLE)
            execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --version OUTPUT_VARIABLE LLVM_CONFIG_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir OUTPUT_VARIABLE LLVM_INCLUDEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir OUTPUT_VARIABLE LLVM_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core support nativecodegen target asmparser asmprinter passes scalaropts orcjit orcshared orctargetprocess jitlink mcjit executionengine irreader transformutils analysis OUTPUT_VARIABLE LLVM_LIBS_OUTPUT OUTPUT_STRIP_TRAILING_WHITESPACE)
            execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(REPLACE " " ";" LLVM_JIT_LIBS_FALLBACK "${LLVM_LIBS_OUTPUT} ${LLVM_SYSTEM_LIBS}")
            set(LLVM_FALLBACK_FOUND ON)
            message(STATUS "✅ Using llvm-config fallback for JIT: ${LLVM_CONFIG_VERSION}")
        else()
            if(DEFINED LLVM_ROOT)
                set(_LLVM_ROOT "${LLVM_ROOT}")
            elseif(DEFINED LLVM_DIR)
                get_filename_component(_LLVM_ROOT "${LLVM_DIR}" DIRECTORY)
            endif()
            if(DEFINED _LLVM_ROOT)
                set(LLVM_INCLUDEDIR "${_LLVM_ROOT}/include")
                set(LLVM_LIBDIR "${_LLVM_ROOT}/lib")
                find_library(LLVM_MONO_LIB NAMES LLVM PATHS "${LLVM_LIBDIR}" NO_DEFAULT_PATH)
                if(LLVM_MONO_LIB)
                    set(LLVM_JIT_LIBS_FALLBACK "${LLVM_MONO_LIB}")
                    set(LLVM_FALLBACK_FOUND ON)
                    message(STATUS "✅ Using monolithic LLVM lib for JIT at ${LLVM_MONO_LIB}")
                else()
                    file(GLOB LLVM_LIBS_ALL "${LLVM_LIBDIR}/*.lib")
                    if(LLVM_LIBS_ALL)
                        set(LLVM_JIT_LIBS_FALLBACK "${LLVM_LIBS_ALL}")
                        set(LLVM_FALLBACK_FOUND ON)
                        message(STATUS "✅ Using lib directory fallback for JIT at ${LLVM_LIBDIR}")
                    endif()
                endif()
            elseif(DEFINED LLVM_LIBDIR)
                get_filename_component(_LLVM_ROOT_FROM_LIB "${LLVM_LIBDIR}" DIRECTORY)
                set(LLVM_INCLUDEDIR "${_LLVM_ROOT_FROM_LIB}/include")
            endif()
            # Verify headers are available; try common Windows locations before disabling
            if(LLVM_FALLBACK_FOUND)
                set(_llvm_candidate_includes "")
                if(LLVM_INCLUDEDIR)
                    list(APPEND _llvm_candidate_includes "${LLVM_INCLUDEDIR}")
                endif()
                if(DEFINED ENV{LLVM_INCLUDEDIR})
                    list(APPEND _llvm_candidate_includes "$ENV{LLVM_INCLUDEDIR}")
                endif()
                list(APPEND _llvm_candidate_includes
                    "C:/Program Files/LLVM/include"
                    "C:/LLVM/include"
                    "D:/LLVM/include"
                    )
                set(_found_lljit_header OFF)
                foreach(_inc IN LISTS _llvm_candidate_includes)
                    if(EXISTS "${_inc}/llvm/ExecutionEngine/Orc/LLJIT.h")
                        set(LLVM_INCLUDEDIR "${_inc}")
                        set(_found_lljit_header ON)
                        message(STATUS "✅ Found ORC LLJIT headers at ${LLVM_INCLUDEDIR}")
                        break()
                    endif()
                endforeach()
                if(NOT _found_lljit_header)
                    message(WARNING "ORC LLJIT headers not found. Checked: ${_llvm_candidate_includes}. Disabling JIT ORC integration.")
                    unset(LLVM_FALLBACK_FOUND)
                endif()
            endif()
            if(NOT LLVM_FALLBACK_FOUND)
                message(WARNING "LLVM not found for JIT. Building with JIT scaffolding only (no ORC translator).")
            endif()
        endif()
    endif()
endif()

# Ensure AoT sources excluded after detection
if(NOT VOLT_ENABLE_AOT)
    file(GLOB VOLT_AOT_SOURCES "${CMAKE_SOURCE_DIR}/src/aot/*.cpp")
    list(REMOVE_ITEM VOLT_SOURCES ${VOLT_AOT_SOURCES})
endif()

# Main executable
add_executable(volt
    ${VOLT_SOURCES}
)

if(MSVC)
    set_target_properties(volt PROPERTIES LINK_FLAGS "/STACK:33554432")
endif()
set_target_properties(volt PROPERTIES OUTPUT_NAME "volt")

target_include_directories(volt PRIVATE 
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
    ${CMAKE_SOURCE_DIR}/src/vm
    ${CMAKE_SOURCE_DIR}/src/compiler
)

if(VOLT_ENABLE_AOT)
    target_include_directories(volt PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(volt PRIVATE ${LLVM_LIBS})
    target_compile_definitions(volt PRIVATE VOLT_ENABLE_AOT)
endif()
if(VOLT_ENABLE_AOT AND NOT LLVM_FOUND AND LLVM_FALLBACK_FOUND)
    target_include_directories(volt PRIVATE ${LLVM_INCLUDEDIR})
    if(LLVM_LIBDIR)
        target_link_directories(volt PRIVATE ${LLVM_LIBDIR})
    endif()
    target_link_libraries(volt PRIVATE ${LLVM_LIBS_FALLBACK})
    target_compile_definitions(volt PRIVATE VOLT_ENABLE_AOT)
endif()
if(VOLT_ENABLE_JIT AND LLVM_FOUND)
    target_include_directories(volt PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(volt PRIVATE ${LLVM_JIT_LIBS})
    target_compile_definitions(volt PRIVATE VOLT_ENABLE_JIT)
elseif(VOLT_ENABLE_JIT AND LLVM_FALLBACK_FOUND)
    target_include_directories(volt PRIVATE ${LLVM_INCLUDEDIR})
    if(LLVM_LIBDIR)
        target_link_directories(volt PRIVATE ${LLVM_LIBDIR})
    endif()
    target_link_libraries(volt PRIVATE ${LLVM_JIT_LIBS_FALLBACK})
    target_compile_definitions(volt PRIVATE VOLT_ENABLE_JIT)
endif()

# Set output directory
set_target_properties(volt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set(VOLT_RUNTIME_SOURCES ${VOLT_SOURCES})
list(FILTER VOLT_RUNTIME_SOURCES EXCLUDE REGEX ".*/main.cpp$")
add_library(volt_runtime STATIC ${VOLT_RUNTIME_SOURCES})
target_include_directories(volt_runtime PRIVATE 
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
    ${CMAKE_SOURCE_DIR}/src/vm
    ${CMAKE_SOURCE_DIR}/src/compiler
)
if(VOLT_ENABLE_AOT)
    target_include_directories(volt_runtime PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(volt_runtime PRIVATE ${LLVM_LIBS})
    target_compile_definitions(volt_runtime PRIVATE VOLT_ENABLE_AOT)
endif()
if(VOLT_ENABLE_AOT AND NOT LLVM_FOUND AND LLVM_FALLBACK_FOUND)
    target_include_directories(volt_runtime PRIVATE ${LLVM_INCLUDEDIR})
    if(LLVM_LIBDIR)
        target_link_directories(volt_runtime PRIVATE ${LLVM_LIBDIR})
    endif()
    target_link_libraries(volt_runtime PRIVATE ${LLVM_LIBS_FALLBACK})
    target_compile_definitions(volt_runtime PRIVATE VOLT_ENABLE_AOT)
endif()
if(VOLT_ENABLE_JIT AND LLVM_FOUND)
    target_include_directories(volt_runtime PRIVATE ${LLVM_INCLUDE_DIRS})
    target_link_libraries(volt_runtime PRIVATE ${LLVM_JIT_LIBS})
    target_compile_definitions(volt_runtime PRIVATE VOLT_ENABLE_JIT)
elseif(VOLT_ENABLE_JIT AND LLVM_FALLBACK_FOUND)
    target_include_directories(volt_runtime PRIVATE ${LLVM_INCLUDEDIR})
    if(LLVM_LIBDIR)
        target_link_directories(volt_runtime PRIVATE ${LLVM_LIBDIR})
    endif()
    target_link_libraries(volt_runtime PRIVATE ${LLVM_JIT_LIBS_FALLBACK})
    target_compile_definitions(volt_runtime PRIVATE VOLT_ENABLE_JIT)
endif()
set_target_properties(volt_runtime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ========================================
# TESTS
# ========================================

if(BUILD_TESTS)
    # Fetch GoogleTest
    include(FetchContent)
    message(STATUS "Fetching Google Test...")
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
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
        src/interpreter/gc_alloc.cpp
        src/observability/profiler.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        src/features/string_pool.cpp
        src/vm/vm.cpp
        src/compiler/compiler.cpp
        src/jit/jit.cpp
        src/jit/llvm_jit.cpp
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
        tests/test_vm.cpp
        tests/test_call_ic_diagnostics.cpp
        tests/test_hotness_counters.cpp
        tests/test_benchmark_assertions.cpp
        tests/test_property_fuzz.cpp
        tests/test_property_ic_megamorphic.cpp
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
        ${CMAKE_SOURCE_DIR}/src/vm
        ${CMAKE_SOURCE_DIR}/src/compiler
    )
    
    target_link_libraries(volt_tests PRIVATE GTest::gtest_main GTest::gmock_main)
    if(VOLT_ENABLE_JIT AND LLVM_FOUND)
        target_include_directories(volt_tests PRIVATE ${LLVM_INCLUDE_DIRS})
        if(DEFINED LLVM_LIBRARY_DIRS)
            target_link_directories(volt_tests PRIVATE ${LLVM_LIBRARY_DIRS})
        endif()
        target_link_libraries(volt_tests PRIVATE ${LLVM_JIT_LIBS})
        target_compile_definitions(volt_tests PRIVATE VOLT_ENABLE_JIT)
    elseif(VOLT_ENABLE_JIT AND LLVM_FALLBACK_FOUND)
        target_include_directories(volt_tests PRIVATE ${LLVM_INCLUDEDIR})
        if(LLVM_LIBDIR)
            target_link_directories(volt_tests PRIVATE ${LLVM_LIBDIR})
        endif()
        target_link_libraries(volt_tests PRIVATE ${LLVM_JIT_LIBS_FALLBACK})
        target_compile_definitions(volt_tests PRIVATE VOLT_ENABLE_JIT)
    endif()
    
    # Set output directory for tests
    set_target_properties(volt_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(volt_tests)
    
    message(STATUS "✅ Tests enabled (598 tests)")

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
        src/interpreter/gc_alloc.cpp
        src/observability/profiler.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        src/features/string_pool.cpp
        src/interpreter/module.cpp
        src/vm/vm.cpp
        src/compiler/compiler.cpp
        src/jit/jit.cpp
        src/jit/llvm_jit.cpp
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
    
    target_link_libraries(volt_integration_tests PRIVATE GTest::gtest_main)
    if(VOLT_ENABLE_JIT AND LLVM_FOUND)
        target_include_directories(volt_integration_tests PRIVATE ${LLVM_INCLUDE_DIRS})
        if(DEFINED LLVM_LIBRARY_DIRS)
            target_link_directories(volt_integration_tests PRIVATE ${LLVM_LIBRARY_DIRS})
        endif()
        target_link_libraries(volt_integration_tests PRIVATE ${LLVM_JIT_LIBS})
        target_compile_definitions(volt_integration_tests PRIVATE VOLT_ENABLE_JIT)
    elseif(VOLT_ENABLE_JIT AND LLVM_FALLBACK_FOUND)
        target_include_directories(volt_integration_tests PRIVATE ${LLVM_INCLUDEDIR})
        if(LLVM_LIBDIR)
            target_link_directories(volt_integration_tests PRIVATE ${LLVM_LIBDIR})
        endif()
        target_link_libraries(volt_integration_tests PRIVATE ${LLVM_JIT_LIBS_FALLBACK})
        target_compile_definitions(volt_integration_tests PRIVATE VOLT_ENABLE_JIT)
    endif()
    
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
        benchmarks/benchmark_main.cpp
        benchmarks/benchmark_vm.cpp
        benchmarks/benchmark_jit.cpp
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
        src/interpreter/gc_alloc.cpp
        src/observability/profiler.cpp
        src/features/array.cpp
        src/features/hashmap.cpp
        src/features/class.cpp
        src/features/string_pool.cpp
        src/interpreter/module.cpp
        src/vm/vm.cpp
        src/compiler/compiler.cpp
        src/jit/jit.cpp
        src/jit/llvm_jit.cpp
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
    
    target_link_libraries(volt_benchmarks PRIVATE benchmark::benchmark_main)
    if(VOLT_ENABLE_JIT AND LLVM_FOUND)
        target_include_directories(volt_benchmarks PRIVATE ${LLVM_INCLUDE_DIRS})
        if(DEFINED LLVM_LIBRARY_DIRS)
            target_link_directories(volt_benchmarks PRIVATE ${LLVM_LIBRARY_DIRS})
        endif()
        target_link_libraries(volt_benchmarks PRIVATE ${LLVM_JIT_LIBS})
        target_compile_definitions(volt_benchmarks PRIVATE VOLT_ENABLE_JIT)
    elseif(VOLT_ENABLE_JIT AND LLVM_FALLBACK_FOUND)
        target_include_directories(volt_benchmarks PRIVATE ${LLVM_INCLUDEDIR})
        if(LLVM_LIBDIR)
            target_link_directories(volt_benchmarks PRIVATE ${LLVM_LIBDIR})
        endif()
        target_link_libraries(volt_benchmarks PRIVATE ${LLVM_JIT_LIBS_FALLBACK})
        target_compile_definitions(volt_benchmarks PRIVATE VOLT_ENABLE_JIT)
    endif()
    
    set_target_properties(volt_benchmarks PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# ========================================
# TOOLING: Coverage and Static Analysis
# ========================================
option(ENABLE_COVERAGE "Enable code coverage (GCC/Clang)" OFF)
if(ENABLE_COVERAGE AND NOT MSVC)
    add_compile_options(-O0 -g --coverage)
    add_link_options(--coverage)
    add_custom_target(coverage
        COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE} --output-on-failure
        COMMAND gcovr -r ${CMAKE_SOURCE_DIR} --xml -o ${CMAKE_BINARY_DIR}/coverage.xml || true
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Run tests and generate coverage report"
    )
endif()

option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if(CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
        message(STATUS "✅ clang-tidy enabled: ${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()
option(ENABLE_PGO "Enable Profile-Guided Optimization flags (clang/gcc)" OFF)
set(PGO_PHASE "" CACHE STRING "PGO phase: 'generate' or 'use'")
if(ENABLE_PGO)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        if(PGO_PHASE STREQUAL "generate")
            add_compile_options(-fprofile-generate)
            add_link_options(-fprofile-generate)
            message(STATUS "✅ PGO generate enabled (-fprofile-generate)")
        elseif(PGO_PHASE STREQUAL "use")
            add_compile_options(-fprofile-use -fprofile-correction)
            add_link_options(-fprofile-use -fprofile-correction)
            message(STATUS "✅ PGO use enabled (-fprofile-use)")
        else()
            message(WARNING "ENABLE_PGO is ON but PGO_PHASE not set to 'generate' or 'use'")
        endif()
    else()
        message(WARNING "PGO flags configured for clang/gcc only. Compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()
# ========================================
# DEV TOOLS (Phase 8)
# ========================================
add_executable(volt_lsp
    tools/lsp/json.cpp
    tools/lsp/lsp_server.cpp
    tools/fmt/formatter.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
    src/interpreter/value.cpp
    src/interpreter/environment.cpp
    src/interpreter/interpreter.cpp
    src/interpreter/natives/native_math.cpp
    src/interpreter/natives/native_string.cpp
    src/interpreter/natives/native_array.cpp
    src/interpreter/natives/native_io.cpp
    src/interpreter/natives/native_time.cpp
    src/interpreter/natives/native_json.cpp
    src/interpreter/gc_alloc.cpp
    src/observability/profiler.cpp
    src/interpreter/module.cpp
    src/features/array.cpp
    src/features/hashmap.cpp
    src/features/class.cpp
    src/features/callable.cpp
    src/features/string_pool.cpp
    src/vm/vm.cpp
)
target_include_directories(volt_lsp PRIVATE
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
    ${CMAKE_SOURCE_DIR}/tools/fmt
)
set_target_properties(volt_lsp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "volt_lsp"
)
add_executable(volt_fmt
    tools/fmt/formatter.cpp
    tools/fmt/main.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/features/string_pool.cpp
)
target_include_directories(volt_fmt PRIVATE
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
)
set_target_properties(volt_fmt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "volt-fmt"
)

# ========================================
# HARDENING & FUZZING (Phase 9)
# ========================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        message(STATUS "✅ ASan enabled")
    endif()
    if(ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=undefined)
        message(STATUS "✅ UBSan enabled")
    endif()
    if(ENABLE_MSAN AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
        add_link_options(-fsanitize=memory)
        message(STATUS "✅ MSan enabled")
    endif()
endif()

if(ENABLE_LIBFUZZER AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_executable(volt_lexer_fuzz
        tools/fuzz/lexer_fuzz.cpp
        src/lexer/token.cpp
        src/lexer/lexer.cpp
        src/features/string_pool.cpp
    )
    target_include_directories(volt_lexer_fuzz PRIVATE
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/lexer
    )
    target_link_options(volt_lexer_fuzz PRIVATE -fsanitize=fuzzer)
    target_compile_options(volt_lexer_fuzz PRIVATE -fsanitize=fuzzer)
    set_target_properties(volt_lexer_fuzz PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        OUTPUT_NAME "volt_lexer_fuzz"
    )

    add_executable(volt_parser_fuzz
        tools/fuzz/parser_fuzz.cpp
        src/lexer/token.cpp
        src/lexer/lexer.cpp
        src/parser/ast.cpp
        src/parser/parser.cpp
        src/features/string_pool.cpp
    )
    target_include_directories(volt_parser_fuzz PRIVATE
        ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/lexer
        ${CMAKE_SOURCE_DIR}/src/parser
    )
    target_link_options(volt_parser_fuzz PRIVATE -fsanitize=fuzzer)
    target_compile_options(volt_parser_fuzz PRIVATE -fsanitize=fuzzer)
    set_target_properties(volt_parser_fuzz PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        OUTPUT_NAME "volt_parser_fuzz"
    )
    message(STATUS "✅ libFuzzer targets enabled (lexer, parser)")
endif()

# ========================================
# API GENERATOR & PACKAGE MANAGER (Phase 9)
# ========================================
add_executable(volt_api_gen
    tools/docs/gen_api.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/ast.cpp
    src/parser/parser.cpp
    src/interpreter/value.cpp
    src/interpreter/environment.cpp
    src/interpreter/interpreter.cpp
    src/interpreter/module.cpp
    src/interpreter/natives/native_math.cpp
    src/interpreter/natives/native_string.cpp
    src/interpreter/natives/native_array.cpp
    src/interpreter/natives/native_io.cpp
    src/interpreter/natives/native_time.cpp
    src/interpreter/natives/native_json.cpp
    src/interpreter/gc_alloc.cpp
    src/observability/profiler.cpp
    src/features/array.cpp
    src/features/hashmap.cpp
    src/features/class.cpp
    src/features/callable.cpp
    src/features/string_pool.cpp
    src/vm/vm.cpp
)
target_include_directories(volt_api_gen PRIVATE
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lexer
    ${CMAKE_SOURCE_DIR}/src/parser
    ${CMAKE_SOURCE_DIR}/src/interpreter
    ${CMAKE_SOURCE_DIR}/src/features
)
set_target_properties(volt_api_gen PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "volt_api_gen"
)

add_executable(volt_pm
    tools/pm/main.cpp
)
set_target_properties(volt_pm PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME "volt-pm"
)
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
