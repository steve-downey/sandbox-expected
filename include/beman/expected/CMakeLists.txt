# include/beman/expected/CMakeLists.txt                             -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_EXPECTED_USE_MODULES)
    target_sources(
        beman.expected
        PUBLIC
            FILE_SET CXX_MODULES FILES expected.cppm
            FILE_SET HEADERS
                FILES expected.hpp unexpected.hpp bad_expected_access.hpp
    )
else()
    target_sources(
        beman.expected
        PUBLIC
            FILE_SET HEADERS
                FILES expected.hpp unexpected.hpp bad_expected_access.hpp
    )
endif()
