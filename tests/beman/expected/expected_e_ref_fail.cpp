// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<int, int&> is ill-formed [expected.object.general] para 2
// EXPECT: "E must not be a reference"
#include <beman/expected/expected.hpp>

beman::expected::expected<int, int&> e; // must not compile
