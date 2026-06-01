# Test Plan: Step 7 — expected<T&, E> Reference Specialization

**Standard section:** No direct standard wording — this is novel beyond [expected.expected].
   Design follows P2988 (optional<T&>) rebind semantics.
**Test file:** `tests/beman/expected/expected_ref.test.cpp`
**Negative-compile files:** `tests/beman/expected/expected_ref_*_fail.cpp`

---

## Testing Strategy

Catch2. Include header twice. This specialization is not in the C++26 standard
(it's the proposal being implemented), so tests validate the P2988-derived
design rather than quoting standard wording verbatim.

Key behaviors to test:
1. **Rebind semantics on assignment** — assigning to an expected<T&, E> changes
   what T is referred to, never assigns through the T reference
2. **Shallow const** — `const expected<T&, E>` still allows mutation through `*e`
3. **No default constructor** — T& cannot be null; no "empty" state
4. **Dangling prevention** — constructors that would bind temporaries are `= delete`
5. **No in-place T constructor** — `in_place_t` args for the value side don't exist
   (just pass the lvalue reference directly); `unexpect_t` args for error still work

---

## Type-Level Tests (static_assert)

```cpp
// expected<T&, E> is a valid specialization
static_assert(std::is_constructible_v<expected<int&, int>, int&>);

// No default constructor
static_assert(!std::is_default_constructible_v<expected<int&, int>>);

// Copy/move constructors exist (copy the stored pointer)
static_assert(std::is_copy_constructible_v<expected<int&, int>>);
static_assert(std::is_move_constructible_v<expected<int&, int>>);

// Not constructible from a temporary (dangling prevention)
// The binding T& to an rvalue must be deleted
// This is tested via a fail-file (cannot express in static_assert easily)

// operator-> returns T*
static_assert(std::is_same_v<
    decltype(std::declval<expected<int&, int>>().operator->()),
    int*>);

// operator* returns T&
static_assert(std::is_same_v<
    decltype(*std::declval<expected<int&, int>>()),
    int&>);

// value() returns T&
static_assert(std::is_same_v<
    decltype(std::declval<expected<int&, int>>().value()),
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

TEST_CASE("expected<T&>: construct from lvalue reference", "[expected_ref]") {
    int x = 42;
    expected<int&, int> e(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
    CHECK(*e == 42);
}

TEST_CASE("expected<T&>: construct from unexpected", "[expected_ref]") {
    expected<int&, int> e = unexpected(7);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 7);
}

TEST_CASE("expected<T&>: construct from unexpect_t in-place error", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "err");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "err");
}

TEST_CASE("expected<T&>: copy construct (copies pointer)", "[expected_ref]") {
    int x = 1;
    expected<int&, int> a(x);
    expected<int&, int> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: move construct (copies pointer)", "[expected_ref]") {
    int x = 2;
    expected<int&, int> a(x);
    expected<int&, int> b = std::move(a);
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

// Base-to-derived / derived-to-base conversions
TEST_CASE("expected<T&>: construct from derived expected<U&, G>", "[expected_ref]") {
    struct Base { virtual ~Base() = default; int v; };
    struct Derived : Base { Derived(int i) { v = i; } };

    Derived d{99};
    expected<Derived&, int> src(d);
    expected<Base&, int> dst = src;
    REQUIRE(dst.has_value());
    CHECK(dst->v == 99);
    CHECK(&*dst == static_cast<Base*>(&d));
}

// ---------------------------------------------------------------------------
// Rebind semantics on assignment
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: rebind reference on assignment from lvalue", "[expected_ref]") {
    int x = 1, y = 2;
    expected<int&, int> e(x);
    e = y;
    // e now refers to y, not x
    CHECK(&*e == &y);
    CHECK(*e == 2);
    // x is unchanged
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: rebind does NOT assign through reference", "[expected_ref]") {
    int x = 100, y = 200;
    expected<int&, int> e(x);
    e = y;
    // rebind — x must still be 100
    CHECK(x == 100);
    CHECK(*e == 200);
}

TEST_CASE("expected<T&>: assign from unexpected transitions to error state", "[expected_ref]") {
    int x = 5;
    expected<int&, int> e(x);
    e = unexpected(99);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 99);
    CHECK(x == 5);  // x unchanged
}

TEST_CASE("expected<T&>: assign lvalue rebinds from error state", "[expected_ref]") {
    int x = 7;
    expected<int&, int> e = unexpected(1);
    e = x;
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

// ---------------------------------------------------------------------------
// Shallow const
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: shallow const allows mutation of referent", "[expected_ref]") {
    int x = 10;
    const expected<int&, int> e(x);
    // *e returns int& (not const int&) — const applies to expected, not T
    *e = 20;
    CHECK(x == 20);
}

// operator-> on const expected<T&, E> returns T*
TEST_CASE("expected<T&>: operator-> on const returns T*", "[expected_ref]") {
    int x = 5;
    const expected<int&, int> e(x);
    static_assert(std::is_same_v<decltype(e.operator->()), int*>);
    *e.operator->() = 99;
    CHECK(x == 99);
}

// ---------------------------------------------------------------------------
// Observers
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: operator* returns T&", "[expected_ref]") {
    int x = 42;
    expected<int&, int> e(x);
    static_assert(std::is_same_v<decltype(*e), int&>);
    *e = 99;
    CHECK(x == 99);
}

TEST_CASE("expected<T&>: operator-> returns T*", "[expected_ref]") {
    struct S { int v; };
    S s{7};
    expected<S&, int> e(s);
    CHECK(e->v == 7);
    e->v = 99;
    CHECK(s.v == 99);
}

TEST_CASE("expected<T&>: value() returns T& or throws", "[expected_ref]") {
    int x = 1;
    expected<int&, int> e(x);
    static_assert(std::is_same_v<decltype(e.value()), int&>);
    CHECK(e.value() == 1);
    e.value() = 2;
    CHECK(x == 2);
}

TEST_CASE("expected<T&>: value() throws bad_expected_access on error", "[expected_ref]") {
    expected<int&, int> e = unexpected(5);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<T&>: error() returns error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    CHECK(e.error() == 42);
}

TEST_CASE("expected<T&>: value_or returns T value when has error", "[expected_ref]") {
    expected<int&, int> e = unexpected(0);
    // value_or must provide a fallback — returns T by value
    // (the reference's value type, not T& itself)
    int fallback = 99;
    int result = e.value_or(fallback);
    CHECK(result == 99);
}

TEST_CASE("expected<T&>: value_or returns referred value when has value", "[expected_ref]") {
    int x = 42;
    expected<int&, int> e(x);
    int fallback = 0;
    int result = e.value_or(fallback);
    CHECK(result == 42);
}

// ---------------------------------------------------------------------------
// Swap
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: swap value-value rebinds pointers", "[expected_ref]") {
    int x = 1, y = 2;
    expected<int&, int> a(x), b(y);
    a.swap(b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
    // Values unchanged
    CHECK(x == 1);
    CHECK(y == 2);
}

TEST_CASE("expected<T&>: swap value-error", "[expected_ref]") {
    int x = 1;
    expected<int&, int> a(x), b(unexpect, 99);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 99);
    CHECK(&*b == &x);
}

// ---------------------------------------------------------------------------
// Equality
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: equality with expected<T2&, E2>", "[expected_ref]") {
    int x = 5, y = 5, z = 6;
    expected<int&, int> a(x), b(y), c(z);
    CHECK(a == b);      // values equal (5 == 5)
    CHECK(!(a == c));   // values unequal
}

TEST_CASE("expected<T&>: equality with value type", "[expected_ref]") {
    int x = 42;
    expected<int&, int> e(x);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T&>: equality with unexpected", "[expected_ref]") {
    expected<int&, int> e = unexpected(7);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

// ---------------------------------------------------------------------------
// Monadic operations
// ---------------------------------------------------------------------------

TEST_CASE("expected<T&>: and_then passes T& to callable", "[expected_ref]") {
    int x = 5;
    expected<int&, int> e(x);
    auto r = e.and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&>: transform passes T& to callable", "[expected_ref]") {
    int x = 3;
    expected<int&, int> e(x);
    auto r = e.transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 4);
}

TEST_CASE("expected<T&>: or_else passes error to callable", "[expected_ref]") {
    expected<int&, int> e = unexpected(99);
    int x = 0;
    auto r = e.or_else([&](int v) -> expected<int&, int> {
        x = v;
        return unexpected(v);
    });
    CHECK(x == 99);
}

TEST_CASE("expected<T&>: transform_error transforms error", "[expected_ref]") {
    expected<int&, int> e = unexpected(5);
    auto r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}
```

---

## Negative Compile Tests

### `expected_ref_temporary_fail.cpp`
```cpp
// NEGATIVE: constructing expected<int&, E> from a temporary binds a dangling reference
// Must be deleted by reference_constructs_from_temporary_v check
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int> e(42);  // 42 is a temporary — must not compile
}
```

### `expected_ref_no_default_fail.cpp`
```cpp
// NEGATIVE: expected<T&, E> has no default constructor (T& cannot be null)
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int&, int> e;  // must not compile
}
```

### `expected_ref_inplace_value_fail.cpp`
```cpp
// NEGATIVE: expected<T&, E> has no in_place_t value constructor
// (makes no sense for references; just bind the lvalue directly)
#include <beman/expected/expected.hpp>
void test() {
    int x = 5;
    beman::expected::expected<int&, int> e(std::in_place, x);  // must not compile
}
```

---

## CMakeLists additions

Each `_fail.cpp` follows the transcode pattern:
```cmake
# example for one fail file
add_library(beman.expected.tests.expected_ref_temporary_fail OBJECT)
target_sources(... PRIVATE expected_ref_temporary_fail.cpp)
target_link_libraries(... PRIVATE beman::expected)
set_target_properties(... PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true)
add_test(NAME expected_ref_temporary_fail
    COMMAND ${CMAKE_COMMAND} --build ... --target ...)
set_tests_properties(expected_ref_temporary_fail PROPERTIES WILL_FAIL TRUE)
```
