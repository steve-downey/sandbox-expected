# installtest/CMakeLists.txt                                        -*-CMake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.27)
project(TestInstalledExpected)

set(CMAKE_CXX_STANDARD 26) #current minimum C++ version

# Enable testing in this separate project
enable_testing()

# Find the installed package
set(EXPECTED_INSTALL_DIR "../.install/lib/cmake/beman.expected")
find_package(
    beman.expected
    REQUIRED
    PATHS ${EXPECTED_INSTALL_DIR}
    NO_DEFAULT_PATH
)

# Add the test executable
add_executable(TestInstalledExpected test.cpp)

# Link against the imported target
target_link_libraries(TestInstalledExpected beman::expected)

# Register the test with CTest
add_test(NAME RunInstalledTest COMMAND TestInstalledExpected)

# Ensure 'make test' first builds the 'all' target
set(CMAKE_SKIP_TEST_ALL_DEPENDENCY FALSE)
