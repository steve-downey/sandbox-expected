# examples/CMakeLists.txt                                           -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(ALL_EXAMPLES expected)


message("Examples to be built: ${ALL_EXAMPLES}")

foreach(example ${ALL_EXAMPLES})
    add_executable(beman.expected.examples.${example})
    target_sources(beman.expected.examples.${example} PRIVATE ${example}.cpp)
    target_link_libraries(
        beman.expected.examples.${example}
        PRIVATE beman::expected
    )
    if(BEMAN_EXPECTED_USE_MODULES)
        set_target_properties(
            beman.expected.examples.${example}
            PROPERTIES CXX_MODULE_STD ON
        )
    endif()
endforeach()
