// tests/beman/expected/expected_ref_e_ref_fail.cpp                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<T&, E> where E is a reference type is ill-formed
// EXPECT: "E must not be a reference"
#include <beman/expected/expected.hpp>
void test() {
    int x = 0;
    beman::expected::expected<int&, int&> e(x); // must not compile
}
