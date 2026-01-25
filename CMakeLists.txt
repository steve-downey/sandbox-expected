# CMakeLists.txt                                                    -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.28...4.2)

project(
    beman.expected # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
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

# for find of beman-install-library
include(infra/cmake/beman-install-library-config.cmake)

add_library(beman.expected INTERFACE)
add_library(beman::expected ALIAS beman.expected)

target_sources(
    beman.expected
    PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(beman.expected PROPERTIES VERIFY_INTERFACE_HEADER_SETS ON)

add_subdirectory(include/beman/expected)

beman_install_library(beman.expected)

if(BEMAN_EXPECTED_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)
endif()

if(BEMAN_EXPECTED_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/expected)
endif()

if(BEMAN_EXPECTED_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
