// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<void, E&> where E is a reference is ill-formed
// EXPECT: "E must not be a reference"
#include <beman/expected/expected.hpp>

beman::expected::expected<void, int&> x; // should fail
