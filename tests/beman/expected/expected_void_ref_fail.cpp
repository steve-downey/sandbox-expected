// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// NEGATIVE: expected<void, int&> is ill-formed — T=void not allowed in expected<T,E&>
// EXPECT: "T must not be void"
#include <beman/expected/expected.hpp>

beman::expected::expected<void, int&> x; // should fail
