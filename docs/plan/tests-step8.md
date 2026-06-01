# Test Plan: Step 8 — expected<T, E&> Error-Reference Specialization

**Standard section:** Novel beyond standard — mirror of Step 7, error side.
**Test file:** `tests/beman/expected/expected_ref_e.test.cpp`
**Negative-compile files:** `tests/beman/expected/expected_ref_e_*_fail.cpp`

---

## Testing Strategy

Catch2. Include header twice. This specialization mirrors Step 7 but for the
error type:
- Value T is stored by value (same as primary template)
- Error E is stored as a non-owning pointer `E*` with rebind semantics
- `error()` returns `E&`
- Assignment from `unexpected<G>` rebinds the error pointer, does not assign through

---

## Type-Level Tests (static_assert)

```cpp
// expected<T, E&> is a valid specialization
static_assert(std::is_constructible_v<expected<int, int&>, std::in_place_t, int>);

// Default constructible (value side works normally)
static_assert(std::is_default_constructible_v<expected<int, int&>>);

// error() returns E&
static_assert(std::is_same_v<
    decltype(std::declval<expected<int, int&>>().error()),
    int&>);

// value() returns T& (owned)
static_assert(std::is_same_v<
    decltype(std::declval<expected<int, int&>>().value()),
    int&>);

// operator-> returns T*
static_assert(std::is_same_v<
    decltype(std::declval<expected<int, int&>>().operator->()),
    int*>);

// Not constructible from a temporary error (dangling prevention)
// Tested via fail-file
```

---

## Test Outline

```cpp
#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// Construction (value side)
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: default construct has value", "[expected_ref_e]") {
    expected<int, int&> e;
    REQUIRE(e.has_value());
    CHECK(*e == 0);
}

TEST_CASE("expected<T,E&>: construct from value", "[expected_ref_e]") {
    expected<int, int&> e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected<T,E&>: in_place value construction", "[expected_ref_e]") {
    expected<std::string, int&> e(std::in_place, 3, 'x');
    REQUIRE(e.has_value());
    CHECK(*e == "xxx");
}

// ---------------------------------------------------------------------------
// Construction (error side — binds reference)
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: construct from unexpected lvalue ref", "[expected_ref_e]") {
    int err = 7;
    // unexpected<int&> or just an lvalue error source
    expected<int, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 7);
}

TEST_CASE("expected<T,E&>: copy construct preserves error pointer", "[expected_ref_e]") {
    int err = 42;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b = a;
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

TEST_CASE("expected<T,E&>: move construct preserves error pointer", "[expected_ref_e]") {
    int err = 5;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

// ---------------------------------------------------------------------------
// Error rebind semantics on assignment
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: error rebind on unexpected assignment", "[expected_ref_e]") {
    int err1 = 1, err2 = 2;
    expected<int, int&> e(unexpect, err1);
    // Assign a new unexpected — rebind, do NOT assign through
    e = unexpected<int&>(err2);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err2);
    // err1 unchanged
    CHECK(err1 == 1);
}

TEST_CASE("expected<T,E&>: rebind does NOT assign through error reference", "[expected_ref_e]") {
    int err1 = 10, err2 = 20;
    expected<int, int&> e(unexpect, err1);
    e = unexpected<int&>(err2);
    CHECK(err1 == 10);  // err1 unchanged — rebind, not assign-through
    CHECK(e.error() == 20);
}

TEST_CASE("expected<T,E&>: assign value when in error state", "[expected_ref_e]") {
    int err = 5;
    expected<int, int&> e(unexpect, err);
    e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected<T,E&>: assign unexpected when in value state", "[expected_ref_e]") {
    int err = 99;
    expected<int, int&> e(42);
    e = unexpected<int&>(err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
}

// ---------------------------------------------------------------------------
// Shallow const on error
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: shallow const allows mutation of error referent", "[expected_ref_e]") {
    int err = 10;
    const expected<int, int&> e(unexpect, err);
    // error() should return int& (not const int&) — shallow const
    e.error() = 20;
    CHECK(err == 20);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: operator*() and operator->() work normally", "[expected_ref_e]") {
    expected<std::string, int&> e(std::in_place, "hello");
    CHECK(e->size() == 5);
    CHECK(*e == "hello");
}

TEST_CASE("expected<T,E&>: value() returns T& (owned)", "[expected_ref_e]") {
    expected<int, int&> e(42);
    static_assert(std::is_same_v<decltype(e.value()), int&>);
    e.value() = 99;
    CHECK(*e == 99);
}

TEST_CASE("expected<T,E&>: value() throws on error", "[expected_ref_e]") {
    int err = 5;
    expected<int, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
    // bad_expected_access<int> stores a copy of the error value
}

TEST_CASE("expected<T,E&>: error() returns E&", "[expected_ref_e]") {
    int err = 7;
    expected<int, int&> e(unexpect, err);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<T,E&>: value_or works normally for value side", "[expected_ref_e]") {
    expected<int, int&> a(42);
    int err = 0;
    expected<int, int&> b(unexpect, err);
    CHECK(a.value_or(0) == 42);
    CHECK(b.value_or(99) == 99);
}

TEST_CASE("expected<T,E&>: error_or returns E by value", "[expected_ref_e]") {
    int err = 7;
    expected<int, int&> a(unexpect, err);
    expected<int, int&> b(42);
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(0) == 0);
}

// ---------------------------------------------------------------------------
// Swap
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: swap value-value", "[expected_ref_e]") {
    expected<int, int&> a(1), b(2);
    a.swap(b);
    CHECK(*a == 2);
    CHECK(*b == 1);
}

TEST_CASE("expected<T,E&>: swap value-error", "[expected_ref_e]") {
    int err = 99;
    expected<int, int&> a(1), b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
    CHECK(*b == 1);
}

TEST_CASE("expected<T,E&>: swap error-error rebinds pointers", "[expected_ref_e]") {
    int e1 = 1, e2 = 2;
    expected<int, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: equality with expected<T2, E2&>", "[expected_ref_e]") {
    expected<int, int&> a(42), b(42);
    CHECK(a == b);
}

TEST_CASE("expected<T,E&>: equality with value type", "[expected_ref_e]") {
    expected<int, int&> e(42);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T,E&>: equality with unexpected", "[expected_ref_e]") {
    int err = 7;
    expected<int, int&> e(unexpect, err);
    // unexpected<E2> comparison — compares error values, not pointers
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

// ---------------------------------------------------------------------------
// Monadic operations — value side same as primary; error side passes E&
// ---------------------------------------------------------------------------

TEST_CASE("expected<T,E&>: and_then works on value side", "[expected_ref_e]") {
    expected<int, int&> e(5);
    auto r = e.and_then([](int v) -> expected<int, int&> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T,E&>: or_else receives E& and can rebind", "[expected_ref_e]") {
    int err = 3;
    expected<int, int&> e(unexpect, err);
    auto r = e.or_else([](int& v) -> expected<int, int&> {
        // v is E& — we can inspect/mutate the error
        return v * 10;
    });
    REQUIRE(r.has_value());
    CHECK(*r == 30);
}

TEST_CASE("expected<T,E&>: transform_error transforms E&", "[expected_ref_e]") {
    int err = 5;
    expected<int, int&> e(unexpect, err);
    auto r = e.transform_error([](int& v) -> std::string {
        return std::to_string(v);
    });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

// ---------------------------------------------------------------------------
// Dangling prevention
// ---------------------------------------------------------------------------

// Tested via fail-file below.
// At minimum, verify that binding to a non-temporary compiles:
TEST_CASE("expected<T,E&>: lvalue error reference compiles", "[expected_ref_e]") {
    int err = 1;
    expected<int, int&> e(unexpect, err);
    CHECK(&e.error() == &err);
}
```

---

## Negative Compile Tests

### `expected_ref_e_temporary_error_fail.cpp`
```cpp
// NEGATIVE: cannot bind a temporary to E& — dangling prevention
#include <beman/expected/expected.hpp>
void test() {
    // 42 is a temporary — expected<int, int&> must refuse to bind E& to it
    beman::expected::expected<int, int&> e(beman::expected::unexpect, 42);
}
```

### `expected_ref_e_const_lvalue_assignment_fail.cpp`
```cpp
// NEGATIVE: cannot bind a const lvalue to non-const E&
#include <beman/expected/expected.hpp>
void test() {
    const int err = 5;
    // expected<int, int&> (non-const E) from const lvalue must fail
    beman::expected::expected<int, int&> e(beman::expected::unexpect, err);
}
```
