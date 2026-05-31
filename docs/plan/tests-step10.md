# Test Plan: Step 10 — expected<void, E&> Void+Error-Reference Specialization

**Standard section:** Novel — intersection of Step 4 (void) and Step 8 (E&).
**Test file:** `tests/beman/expected/expected_void_ref_e.test.cpp`
**Negative-compile files:** `tests/beman/expected/expected_void_ref_e_*_fail.cpp`

---

## Testing Strategy

Catch2. Include header twice. This is the final specialization: void value
semantics combined with reference error semantics. No value storage at all;
error is stored as `E*`.

Key properties:
- **Default-constructible** (void success state is always valid)
- **No value storage** — `operator*()` returns void, no `operator->`, no `value_or`
- **Error as reference** — `error()` returns `E&`, rebind on assignment
- **Trivial operations** — copy/move are trivial (just copy pointer + bool)
- **Dangling prevention** — constructors that bind temporaries to E& are deleted

---

## Type-Level Tests (static_assert)

```cpp
// Default constructible (void state)
static_assert(std::is_default_constructible_v<expected<void, int&>>);
static_assert(std::is_nothrow_default_constructible_v<expected<void, int&>>);

// Trivially copyable (pointer + bool only)
static_assert(std::is_trivially_copy_constructible_v<expected<void, int&>>);
static_assert(std::is_trivially_move_constructible_v<expected<void, int&>>);
static_assert(std::is_trivially_destructible_v<expected<void, int&>>);

// error() returns E&
static_assert(std::is_same_v<
    decltype(std::declval<expected<void, int&>>().error()),
    int&>);

// No operator-> or value_or
static_assert(!requires(expected<void, int&> e) { e.operator->(); });
static_assert(!requires(expected<void, int&> e) { e.value_or(0); });

// operator* returns void
static_assert(std::is_void_v<decltype(*std::declval<expected<void, int&>>())>);
```

---

## Test Outline

```cpp
#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: default construct has value", "[expected_void_ref_e]") {
    expected<void, int&> e;
    REQUIRE(e.has_value());
    static_assert(std::is_nothrow_default_constructible_v<expected<void, int&>>);
}

TEST_CASE("expected<void,E&>: construct from unexpected binds E&", "[expected_void_ref_e]") {
    int err = 42;
    expected<void, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 42);
}

TEST_CASE("expected<void,E&>: in_place_t constructor", "[expected_void_ref_e]") {
    expected<void, int&> e(std::in_place);
    CHECK(e.has_value());
    static_assert(noexcept(expected<void, int&>(std::in_place)));
}

TEST_CASE("expected<void,E&>: copy construct is trivial", "[expected_void_ref_e]") {
    int err = 5;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b = a;
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: move construct copies pointer", "[expected_void_ref_e]") {
    int err = 3;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: convert from expected<void, G&>", "[expected_void_ref_e]") {
    int err = 7;
    expected<void, int&> src(unexpect, err);
    expected<void, int&> dst = src;
    REQUIRE(!dst.has_value());
    CHECK(&dst.error() == &err);
}

// ---------------------------------------------------------------------------
// Error rebind semantics on assignment
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: rebind error on unexpected assignment", "[expected_void_ref_e]") {
    int e1 = 1, e2 = 2;
    expected<void, int&> e(unexpect, e1);
    e = unexpected<int&>(e2);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &e2);
    CHECK(e1 == 1);  // unchanged — rebind, not assign-through
}

TEST_CASE("expected<void,E&>: rebind does NOT assign through error", "[expected_void_ref_e]") {
    int e1 = 100, e2 = 200;
    expected<void, int&> e(unexpect, e1);
    e = unexpected<int&>(e2);
    CHECK(e1 == 100);  // e1 unchanged
    CHECK(e.error() == 200);
}

TEST_CASE("expected<void,E&>: assign unexpected transitions from value to error",
          "[expected_void_ref_e]") {
    int err = 99;
    expected<void, int&> e;
    e = unexpected<int&>(err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

// ---------------------------------------------------------------------------
// Shallow const on error
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: shallow const allows mutation of error referent",
          "[expected_void_ref_e]") {
    int err = 10;
    const expected<void, int&> e(unexpect, err);
    e.error() = 20;  // error() returns int& even on const expected
    CHECK(err == 20);
}

// ---------------------------------------------------------------------------
// emplace() — transition to void state
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: emplace from error state sets has_value", "[expected_void_ref_e]") {
    int err = 5;
    expected<void, int&> e(unexpect, err);
    e.emplace();
    CHECK(e.has_value());
    static_assert(noexcept(e.emplace()));
}

TEST_CASE("expected<void,E&>: emplace from value state is no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    e.emplace();
    CHECK(e.has_value());
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: operator bool and has_value", "[expected_void_ref_e]") {
    expected<void, int&> a;
    int err = 0;
    expected<void, int&> b(unexpect, err);
    CHECK(a.has_value());
    CHECK(bool(a));
    CHECK(!b.has_value());
    CHECK(!bool(b));
}

TEST_CASE("expected<void,E&>: operator*() is void no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    static_assert(std::is_void_v<decltype(*e)>);
    *e;  // no-op
}

TEST_CASE("expected<void,E&>: value() on success is no-op", "[expected_void_ref_e]") {
    expected<void, int&> e;
    e.value();  // must not throw
}

TEST_CASE("expected<void,E&>: value() throws bad_expected_access on error",
          "[expected_void_ref_e]") {
    int err = 7;
    expected<void, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<void,E&>: rvalue value() throws on error", "[expected_void_ref_e]") {
    int err = 3;
    expected<void, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(std::move(e).value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<void,E&>: error() returns E& with correct address", "[expected_void_ref_e]") {
    int err = 99;
    expected<void, int&> e(unexpect, err);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<void,E&>: error_or returns E by value", "[expected_void_ref_e]") {
    int err = 7;
    expected<void, int&> a(unexpect, err);
    expected<void, int&> b;
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(42) == 42);
}

// ---------------------------------------------------------------------------
// Swap
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: swap value-value (no-op)", "[expected_void_ref_e]") {
    expected<void, int&> a, b;
    a.swap(b);
    CHECK(a.has_value());
    CHECK(b.has_value());
}

TEST_CASE("expected<void,E&>: swap value-error", "[expected_void_ref_e]") {
    int err = 42;
    expected<void, int&> a, b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
}

TEST_CASE("expected<void,E&>: swap error-value", "[expected_void_ref_e]") {
    int err = 5;
    expected<void, int&> a(unexpect, err), b;
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<void,E&>: swap error-error rebinds pointers", "[expected_void_ref_e]") {
    int e1 = 1, e2 = 2;
    expected<void, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
    CHECK(e1 == 1 && e2 == 2);  // values unchanged
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: equality both have values", "[expected_void_ref_e]") {
    expected<void, int&> a, b;
    CHECK(a == b);
}

TEST_CASE("expected<void,E&>: equality both have errors (same value)", "[expected_void_ref_e]") {
    int e1 = 5, e2 = 5;
    expected<void, int&> a(unexpect, e1), b(unexpect, e2);
    CHECK(a == b);  // compares error values (5 == 5), not pointers
}

TEST_CASE("expected<void,E&>: equality mixed value/error", "[expected_void_ref_e]") {
    expected<void, int&> a;
    int err = 0;
    expected<void, int&> b(unexpect, err);
    CHECK(!(a == b));
}

TEST_CASE("expected<void,E&>: equality with unexpected", "[expected_void_ref_e]") {
    int err = 7;
    expected<void, int&> e(unexpect, err);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

// ---------------------------------------------------------------------------
// Monadic operations — void value + reference error
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: and_then calls F with no args", "[expected_void_ref_e]") {
    expected<void, int&> e;
    int calls = 0;
    auto r = e.and_then([&]() -> expected<int, int&> { ++calls; return 42; });
    CHECK(calls == 1);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<void,E&>: and_then short-circuits on error", "[expected_void_ref_e]") {
    int err = 3;
    expected<void, int&> e(unexpect, err);
    bool called = false;
    auto r = e.and_then([&]() -> expected<int, int&> { called = true; return 0; });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(&r.error() == &err);
}

TEST_CASE("expected<void,E&>: or_else receives E& and can rebind", "[expected_void_ref_e]") {
    int err = 5;
    expected<void, int&> e(unexpect, err);
    auto r = e.or_else([](int& v) -> expected<void, int&> {
        (void)v;
        return {};  // success
    });
    CHECK(r.has_value());
}

TEST_CASE("expected<void,E&>: transform calls F with no args", "[expected_void_ref_e]") {
    expected<void, int&> e;
    auto r = e.transform([]() { return 42; });
    static_assert(std::is_same_v<decltype(r), expected<int, int&>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<void,E&>: transform with void-returning F", "[expected_void_ref_e]") {
    expected<void, int&> e;
    int count = 0;
    auto r = e.transform([&]() { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, int&>>);
    CHECK(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("expected<void,E&>: transform_error transforms E& to new type",
          "[expected_void_ref_e]") {
    int err = 3;
    expected<void, int&> e(unexpect, err);
    auto r = e.transform_error([](int& v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("expected<void,E&>: transform_error with value short-circuits",
          "[expected_void_ref_e]") {
    expected<void, int&> e;
    bool called = false;
    auto r = e.transform_error([&](int&) -> std::string { called = true; return ""; });
    CHECK(!called);
    CHECK(r.has_value());
}

// ---------------------------------------------------------------------------
// End-to-end chaining
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: monadic chaining", "[expected_void_ref_e]") {
    int err = 0;
    auto pass = [&]() -> expected<void, int&> { return {}; };
    auto fail = [&]() -> expected<void, int&> { return unexpected<int&>(err); };

    // Happy path
    auto r1 = pass()
        .and_then([]() -> expected<int, int&> { return 42; })
        .transform([](int v) { return v * 2; });
    REQUIRE(r1.has_value());
    CHECK(*r1 == 84);

    // Error path
    auto r2 = fail()
        .and_then([]() -> expected<int, int&> { return 0; })
        .transform_error([](int& v) -> std::string { return std::to_string(v); });
    REQUIRE(!r2.has_value());
}

// ---------------------------------------------------------------------------
// Triviality
// ---------------------------------------------------------------------------

TEST_CASE("expected<void,E&>: trivial operations", "[expected_void_ref_e]") {
    static_assert(std::is_trivially_copyable_v<expected<void, int&>>);
    static_assert(std::is_trivially_destructible_v<expected<void, int&>>);
}
```

---

## Negative Compile Tests

### `expected_void_ref_e_temporary_fail.cpp`
```cpp
// NEGATIVE: cannot bind temporary to E& — dangling prevention
#include <beman/expected/expected.hpp>
void test() {
    // 42 is a temporary — must not compile
    beman::expected::expected<void, int&> e(beman::expected::unexpect, 42);
}
```

### `expected_void_ref_e_const_lvalue_fail.cpp`
```cpp
// NEGATIVE: cannot bind const lvalue to non-const E&
#include <beman/expected/expected.hpp>
void test() {
    const int err = 5;
    beman::expected::expected<void, int&> e(beman::expected::unexpect, err);
}
```

### `expected_void_ref_e_no_value_or_fail.cpp`
```cpp
// NEGATIVE: expected<void, E&> has no value_or member
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<void, int&> e;
    e.value_or(0);  // must not compile — void has no value to fall back from
}
```

---

## CMakeLists additions

Same pattern as all previous steps: OBJECT library + EXCLUDE_FROM_ALL +
WILL_FAIL ctest for each `_fail.cpp`.
