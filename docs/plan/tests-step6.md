# Test Plan: Step 6 — Monadic Operations for expected<void, E>

**Standard section:** [expected.void.monadic] (22.8.7.7)
**Test file:** `tests/beman/expected/expected_void_monadic.test.cpp`

---

## Testing Strategy

Catch2. Include header twice. Key differences from Step 5 (primary template):
- `and_then(F)` invokes F with **no arguments** (void has no value to pass)
- `or_else(F)` invokes F with `error()` — same as primary
- `transform(F)` invokes F with **no arguments**
- `transform_error(F)` invokes F with `error()` — same as primary
- `or_else` has no Constraints (unlike primary): the void case does not need
  `is_constructible_v<T, ...>` because T is void
- Return type for `or_else` must have `value_type = void`

---

## and_then [expected.void.monadic] para 1–8

### Lvalue overloads (para 1–4)
`U = remove_cvref_t<invoke_result_t<F>>`

- **Constraint:** `is_constructible_v<E, decltype(error())>` is true
- **Mandates:** U is a specialization of expected; `U::error_type` is `E`
- **Effects:** if has_value(), return `invoke(f)`; else return `U(unexpect, error())`

**Note:** F is invoked with **no arguments** for void expected — this differs
from the primary template where F receives the stored value.

### Rvalue overloads (para 5–8)
Same F invocation (no args); `is_constructible_v<E, decltype(std::move(error()))>`

---

## or_else [expected.void.monadic] para 9–14

`G = remove_cvref_t<invoke_result_t<F, decltype(error())>>` (lvalue)

- **No Constraint** on T (T is void — no constructibility needed)
- **Mandates:** G is a specialization of expected; `G::value_type` is `T` (void)
- **Effects:** if has_value(), return `G()`; else return `invoke(f, error())`

---

## transform [expected.void.monadic] para 15–22

`U = remove_cv_t<invoke_result_t<F>>` (invoked with no args)

- **Constraint:** `is_constructible_v<E, decltype(error())>`
- **Mandates:** U is valid value type; if !is_void_v<U>, `U u(invoke(f))` well-formed
- **Effects (three branches):**
  - has_value() == false → return `expected<U,E>(unexpect, error())`
  - has_value() && !is_void_v<U> → return expected<U,E> with invoke result
  - has_value() && is_void_v<U> → `invoke(f)`, return `expected<U,E>()`

---

## transform_error [expected.void.monadic] para 23–28

`G = remove_cv_t<invoke_result_t<F, decltype(error())>>`

- **Mandates:** G is valid for unexpected; `G g(invoke(f, error()))` well-formed
- **Effects:** if has_value(), return `expected<void,G>()`; else apply f to error

---

## Test Outline

```cpp
#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>
#include <string>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// and_then — F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("and_then void: has value — calls F with no args", "[expected_void_monadic]") {
    expected<void, std::string> e;
    int calls = 0;
    auto r = e.and_then([&]() -> expected<int, std::string> {
        ++calls;
        return 42;
    });
    CHECK(calls == 1);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("and_then void: has error — short-circuits", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "bad");
    bool called = false;
    auto r = e.and_then([&]() -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "bad");
}

TEST_CASE("and_then void: rvalue overload propagates error by move", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "err");
    auto r = std::move(e).and_then([]() -> expected<int, std::string> {
        return 1;
    });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "err");
}

TEST_CASE("and_then void: return void expected", "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e.and_then([]() -> expected<void, int> { return {}; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(r.has_value());
}

TEST_CASE("and_then void: chaining void-to-value", "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e
        .and_then([]() -> expected<int, int> { return 1; })
        .and_then([](int v) -> expected<int, int> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 2);
}

// ---------------------------------------------------------------------------
// or_else — F called with error when void expected has error
// ---------------------------------------------------------------------------

TEST_CASE("or_else void: has error — calls F", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 7);
    auto r = e.or_else([](int v) -> expected<void, int> {
        (void)v;
        return {};  // success
    });
    CHECK(r.has_value());
}

TEST_CASE("or_else void: has value — short-circuits, returns G()", "[expected_void_monadic]") {
    expected<void, int> e;
    bool called = false;
    auto r = e.or_else([&](int) -> expected<void, int> {
        called = true;
        return {};
    });
    CHECK(!called);
    CHECK(r.has_value());
}

TEST_CASE("or_else void: error propagated through lambda", "[expected_void_monadic]") {
    expected<void, std::string> e(unexpect, "original");
    auto r = e.or_else([](std::string s) -> expected<void, std::string> {
        return unexpected(s + "_fixed");
    });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "original_fixed");
}

// ---------------------------------------------------------------------------
// transform — F called with no args when void
// ---------------------------------------------------------------------------

TEST_CASE("transform void: has value — calls F, returns expected<U, E>",
          "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(r), expected<int, int>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform void: has error — propagates", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 5);
    bool called = false;
    auto r = e.transform([&]() { called = true; return 0; });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == 5);
}

TEST_CASE("transform void: F returns void → expected<void, E>()",
          "[expected_void_monadic]") {
    expected<void, int> e;
    int count = 0;
    auto r = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int>>);
    CHECK(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("transform void: rvalue overload", "[expected_void_monadic]") {
    expected<void, std::string> e;
    auto r = std::move(e).transform([]() -> std::string { return "done"; });
    REQUIRE(r.has_value());
    CHECK(*r == "done");
}

// ---------------------------------------------------------------------------
// transform_error — F called with error, same as primary
// ---------------------------------------------------------------------------

TEST_CASE("transform_error void: has error — transforms error", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 3);
    auto r = e.transform_error([](int v) -> std::string {
        return std::to_string(v);
    });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("transform_error void: has value — returns expected<void, G>()",
          "[expected_void_monadic]") {
    expected<void, int> e;
    bool called = false;
    auto r = e.transform_error([&](int) -> std::string {
        called = true;
        return "";
    });
    CHECK(!called);
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// Chaining combinations
// ---------------------------------------------------------------------------

TEST_CASE("void monadic chaining: and_then → transform_error", "[expected_void_monadic]") {
    expected<void, int> e;
    auto r = e
        .and_then([]() -> expected<void, int> { return {}; })
        .transform_error([](int v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    CHECK(r.has_value());
}

TEST_CASE("void monadic chaining: error path end-to-end", "[expected_void_monadic]") {
    expected<void, int> e(unexpect, 42);
    auto r = e
        .and_then([]() -> expected<void, int> { return {}; })
        .or_else([](int v) -> expected<void, int> {
            if (v == 42) return {};
            return unexpected(v);
        });
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// All 4 ref-qualifications compile
// ---------------------------------------------------------------------------

TEST_CASE("void and_then: all ref qualifications compile", "[expected_void_monadic]") {
    using E = expected<void, int>;
    auto f = []() -> E { return {}; };

    E e;
    (void)(e.and_then(f));
    (void)(std::as_const(e).and_then(f));
    (void)(std::move(e).and_then(f));
    (void)(std::move(std::as_const(e)).and_then(f));
}
```

---

## Negative Compile Tests

### `void_and_then_wrong_error_type_fail.cpp`
```cpp
// NEGATIVE: and_then on void expected mandates U::error_type == E
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int> e;
    e.and_then([]() -> beman::expected::expected<int, double> { return 0; });
}
```

### `void_or_else_wrong_value_type_fail.cpp`
```cpp
// NEGATIVE: or_else on void expected mandates G::value_type == void
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int> e(beman::expected::unexpect, 1);
    e.or_else([](int) -> beman::expected::expected<int, int> { return 0; });
}
```
