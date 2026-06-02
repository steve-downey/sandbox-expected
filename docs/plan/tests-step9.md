# Test Plan: Step 9 — expected<T&, E&> Both-Reference Specialization

**Standard section:** Novel — combines Step 7 (T&) and Step 8 (E&) patterns.
**Test file:** `tests/beman/expected/expected_ref_both.test.cpp`
**Negative-compile files:** `tests/beman/expected/expected_ref_both_*_fail.cpp`

---

## Testing Strategy

Catch2. Include header twice. Both sides are references, stored as pointers.
This specialization has the simplest storage (two pointers + bool) but the
most constraints (both dangling-prevention checks, both rebind semantics,
both shallow-const properties, no default constructor from either side).

---

## Type-Level Tests (static_assert)

```cpp
// expected<T&, E&> is a valid specialization
static_assert(std::is_constructible_v<
    expected<int&, int&>, int&>);           // from value lvalue

// No default constructor
static_assert(!std::is_default_constructible_v<expected<int&, int&>>);

// Trivially copyable (just two pointers + bool)
static_assert(std::is_trivially_copy_constructible_v<expected<int&, int&>>);
static_assert(std::is_trivially_move_constructible_v<expected<int&, int&>>);
static_assert(std::is_trivially_destructible_v<expected<int&, int&>>);

// operator* returns T&
static_assert(std::is_same_v<
    decltype(*std::declval<expected<int&, int&>>()),
    int&>);

// error() returns E&
static_assert(std::is_same_v<
    decltype(std::declval<expected<int&, int&>>().error()),
    int&>);
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

TEST_CASE("expected<T&,E&>: construct from lvalue reference (value)", "[expected_ref_both]") {
    int x = 42;
    expected<int&, int&> e(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
    CHECK(*e == 42);
}

TEST_CASE("expected<T&,E&>: construct from unexpected (error ref)", "[expected_ref_both]") {
    int err = 7;
    expected<int&, int&> e(unexpect, err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(e.error() == 7);
}

TEST_CASE("expected<T&,E&>: copy construct is trivial (copies pointers)", "[expected_ref_both]") {
    int x = 1;
    expected<int&, int&> a(x);
    expected<int&, int&> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&,E&>: move construct copies pointers", "[expected_ref_both]") {
    int err = 5;
    expected<int&, int&> a(unexpect, err);
    expected<int&, int&> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(&b.error() == &err);
}

// ---------------------------------------------------------------------------
// Rebind semantics — both sides
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: rebind value reference", "[expected_ref_both]") {
    int x = 1, y = 2;
    expected<int&, int&> e(x);
    e = y;
    CHECK(&*e == &y);
    CHECK(x == 1);    // x unchanged — rebind, not assign-through
    CHECK(*e == 2);
}

TEST_CASE("expected<T&,E&>: rebind does NOT assign through T", "[expected_ref_both]") {
    int x = 100, y = 200;
    expected<int&, int&> e(x);
    e = y;
    CHECK(x == 100);  // unchanged
}

TEST_CASE("expected<T&,E&>: rebind error reference", "[expected_ref_both]") {
    int e1 = 10, e2 = 20;
    expected<int&, int&> e(unexpect, e1);
    e = unexpected<int&>(e2);
    CHECK(&e.error() == &e2);
    CHECK(e1 == 10);  // e1 unchanged — rebind, not assign-through
}

TEST_CASE("expected<T&,E&>: transition value → error", "[expected_ref_both]") {
    int x = 5, err = 99;
    expected<int&, int&> e(x);
    e = unexpected<int&>(err);
    REQUIRE(!e.has_value());
    CHECK(&e.error() == &err);
    CHECK(x == 5);  // unchanged
}

TEST_CASE("expected<T&,E&>: transition error → value", "[expected_ref_both]") {
    int x = 7, err = 3;
    expected<int&, int&> e(unexpect, err);
    e = x;
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
    CHECK(err == 3);  // unchanged
}

TEST_CASE("expected<T&,E&>: sequential rebinds", "[expected_ref_both]") {
    int x = 1, y = 2, z = 3;
    expected<int&, int&> e(x);
    e = y;
    e = z;
    CHECK(&*e == &z);
    CHECK(x == 1 && y == 2);  // all unchanged
}

// ---------------------------------------------------------------------------
// Shallow const — both sides
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: shallow const — can mutate T referent", "[expected_ref_both]") {
    int x = 10;
    const expected<int&, int&> e(x);
    *e = 20;
    CHECK(x == 20);
}

TEST_CASE("expected<T&,E&>: shallow const — can mutate E referent", "[expected_ref_both]") {
    int err = 10;
    const expected<int&, int&> e(unexpect, err);
    e.error() = 20;
    CHECK(err == 20);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: operator*() returns T&", "[expected_ref_both]") {
    int x = 1;
    expected<int&, int&> e(x);
    static_assert(std::is_same_v<decltype(*e), int&>);
    *e = 99;
    CHECK(x == 99);
}

TEST_CASE("expected<T&,E&>: operator->() returns T*", "[expected_ref_both]") {
    struct S { int v; };
    S s{5};
    expected<S&, int&> e(s);
    CHECK(e->v == 5);
    e->v = 7;
    CHECK(s.v == 7);
}

TEST_CASE("expected<T&,E&>: value() returns T& or throws", "[expected_ref_both]") {
    int x = 42;
    expected<int&, int&> e(x);
    static_assert(std::is_same_v<decltype(e.value()), int&>);
    e.value() = 0;
    CHECK(x == 0);
}

TEST_CASE("expected<T&,E&>: value() throws on error", "[expected_ref_both]") {
    int err = 9;
    expected<int&, int&> e(unexpect, err);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<T&,E&>: error() returns E&", "[expected_ref_both]") {
    int err = 7;
    expected<int&, int&> e(unexpect, err);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    CHECK(&e.error() == &err);
}

TEST_CASE("expected<T&,E&>: value_or returns T by value", "[expected_ref_both]") {
    int x = 42;
    expected<int&, int&> e(x);
    int err = 0;
    expected<int&, int&> f(unexpect, err);
    CHECK(e.value_or(0) == 42);
    CHECK(f.value_or(99) == 99);
}

TEST_CASE("expected<T&,E&>: error_or returns E by value", "[expected_ref_both]") {
    int err = 7;
    expected<int&, int&> a(unexpect, err);
    int x = 1;
    expected<int&, int&> b(x);
    CHECK(a.error_or(0) == 7);
    CHECK(b.error_or(0) == 0);
}

// ---------------------------------------------------------------------------
// Swap — all 4 state combinations
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: swap value-value", "[expected_ref_both]") {
    int x = 1, y = 2;
    expected<int&, int&> a(x), b(y);
    a.swap(b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
    CHECK(x == 1 && y == 2);  // values unchanged
}

TEST_CASE("expected<T&,E&>: swap value-error", "[expected_ref_both]") {
    int x = 1, err = 99;
    expected<int&, int&> a(x), b(unexpect, err);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(&a.error() == &err);
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&,E&>: swap error-error", "[expected_ref_both]") {
    int e1 = 1, e2 = 2;
    expected<int&, int&> a(unexpect, e1), b(unexpect, e2);
    a.swap(b);
    CHECK(&a.error() == &e2);
    CHECK(&b.error() == &e1);
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: equality both have values", "[expected_ref_both]") {
    int x = 5, y = 5, z = 6;
    expected<int&, int&> a(x), b(y), c(z);
    CHECK(a == b);
    CHECK(!(a == c));
}

TEST_CASE("expected<T&,E&>: equality with value type", "[expected_ref_both]") {
    int x = 42;
    expected<int&, int&> e(x);
    CHECK(e == 42);
}

TEST_CASE("expected<T&,E&>: equality with unexpected", "[expected_ref_both]") {
    int err = 7;
    expected<int&, int&> e(unexpect, err);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

// ---------------------------------------------------------------------------
// Monadic operations — reference semantics on both sides
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: and_then passes T& to callable", "[expected_ref_both]") {
    int x = 5;
    expected<int&, int&> e(x);
    auto r = e.and_then([](int& v) -> expected<int, int&> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&,E&>: or_else passes E& to callable", "[expected_ref_both]") {
    int err = 3;
    expected<int&, int&> e(unexpect, err);
    int x = 0;
    auto r = e.or_else([&](int& v) -> expected<int&, int&> {
        x = v;
        return unexpected<int&>(v);
    });
    CHECK(x == 3);
}

TEST_CASE("expected<T&,E&>: transform passes T& to callable", "[expected_ref_both]") {
    int x = 4;
    expected<int&, int&> e(x);
    auto r = e.transform([](int& v) { return v * v; });
    REQUIRE(r.has_value());
    CHECK(*r == 16);
}

TEST_CASE("expected<T&,E&>: transform_error passes E& to callable", "[expected_ref_both]") {
    int err = 5;
    expected<int&, int&> e(unexpect, err);
    auto r = e.transform_error([](int& v) -> std::string {
        return std::to_string(v);
    });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

// ---------------------------------------------------------------------------
// Triviality
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&,E&>: is trivially copyable", "[expected_ref_both]") {
    static_assert(std::is_trivially_copyable_v<expected<int&, int&>>);
}
```

---

## Negative Compile Tests

### `expected_ref_both_temp_value_fail.cpp`
```cpp
// NEGATIVE: binding T& to a temporary is deleted
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int&> e(42);  // temporary — must not compile
}
```

### `expected_ref_both_temp_error_fail.cpp`
```cpp
// NEGATIVE: binding E& to a temporary via unexpect_t is deleted
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int&> e(beman::expected::unexpect, 99);
}
```

### `expected_ref_both_no_default_fail.cpp`
```cpp
// NEGATIVE: no default constructor — both T& and E& would be unbound
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int&> e;
}
```
