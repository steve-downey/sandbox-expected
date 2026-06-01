# CMakeLists.txt                                                    -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(infra/cmake/enable-experimental-import-std.cmake)

project(
    beman.expected
    DESCRIPTION "Expected Over References"
    LANGUAGES CXX
    VERSION 0.1.0
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_EXPECTED_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_EXPECTED_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(BEMAN_EXPECTED_USE_MODULES "Provide beman.expected as a C++ module" OFF)

if(BEMAN_EXPECTED_USE_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/expected/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/expected/config_generated.hpp"
    @ONLY
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

if(BEMAN_EXPECTED_USE_MODULES)
    add_library(beman.expected STATIC)
else()
    add_library(beman.expected INTERFACE)
endif()
add_library(beman::expected ALIAS beman.expected)

if(BEMAN_EXPECTED_USE_MODULES)
    target_sources(
        beman.expected
        PUBLIC
            FILE_SET CXX_MODULES
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(beman.expected PROPERTIES CXX_MODULE_STD ON)
    target_compile_features(beman.expected PUBLIC cxx_std_23)
else()
    target_sources(
        beman.expected
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    set_target_properties(
        beman.expected
        PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
    )
endif()

add_subdirectory(include/beman/expected)

beman_install_library(beman.expected TARGETS beman.expected)
configure_build_telemetry()

if(BEMAN_EXPECTED_BUILD_TESTS)
    find_package(Catch2 CONFIG REQUIRED)
    # When Catch2 is fetched via FetchContent, its extras dir needs to be on the module path.
    # FetchContent_GetProperties reads global state set during fetch, so works from any scope.
    include(FetchContent)
    FetchContent_GetProperties(catch2)
    if(catch2_POPULATED AND catch2_SOURCE_DIR)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
endif()

if(BEMAN_EXPECTED_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/expected)
endif()

if(BEMAN_EXPECTED_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
