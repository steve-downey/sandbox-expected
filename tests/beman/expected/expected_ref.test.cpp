// tests/beman/expected/expected_ref.test.cpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>

using namespace beman::expected;

// =============================================================================
// Type-level static assertions
// =============================================================================

static_assert(std::is_constructible_v<expected<int&, int>, int&>);
static_assert(!std::is_default_constructible_v<expected<int&, int>>);
static_assert(std::is_copy_constructible_v<expected<int&, int>>);
static_assert(std::is_move_constructible_v<expected<int&, int>>);

static_assert(std::is_same_v<decltype(std::declval<expected<int&, int>>().operator->()), int*>);
static_assert(std::is_same_v<decltype(*std::declval<expected<int&, int>>()), int&>);
static_assert(std::is_same_v<decltype(std::declval<expected<int&, int>>().value()), int&>);

// const expected<T&, E> still returns T* / T& (shallow const)
static_assert(std::is_same_v<decltype(std::declval<const expected<int&, int>>().operator->()), int*>);
static_assert(std::is_same_v<decltype(*std::declval<const expected<int&, int>>()), int&>);

// Triviality: when E is trivial, copy/move/assign/destroy should be trivial
static_assert(std::is_trivially_copy_constructible_v<expected<int&, int>>);
static_assert(std::is_trivially_move_constructible_v<expected<int&, int>>);
static_assert(std::is_trivially_copy_assignable_v<expected<int&, int>>);
static_assert(std::is_trivially_move_assignable_v<expected<int&, int>>);
static_assert(std::is_trivially_destructible_v<expected<int&, int>>);

// Non-trivial E: still constructible/assignable but not trivially
static_assert(std::is_copy_constructible_v<expected<int&, std::string>>);
static_assert(std::is_move_constructible_v<expected<int&, std::string>>);
static_assert(!std::is_trivially_copy_constructible_v<expected<int&, std::string>>);
static_assert(!std::is_trivially_move_constructible_v<expected<int&, std::string>>);
static_assert(!std::is_trivially_destructible_v<expected<int&, std::string>>);

// =============================================================================
// Construction
// =============================================================================

TEST_CASE("expected<T&>: construct from lvalue reference", "[expected_ref]") {
    int                   x = 42;
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
    int                   x = 1;
    expected<int&, int> a(x);
    expected<int&, int> b = a;
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: move construct (copies pointer)", "[expected_ref]") {
    int                   x = 2;
    expected<int&, int> a(x);
    expected<int&, int> b = std::move(a);
    REQUIRE(b.has_value());
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: copy construct error state", "[expected_ref]") {
    expected<int&, std::string> a(unexpect, "oops");
    expected<int&, std::string> b = a;
    REQUIRE(!b.has_value());
    CHECK(b.error() == "oops");
}

TEST_CASE("expected<T&>: construct from derived expected<U&, G>", "[expected_ref]") {
    struct Base {
        virtual ~Base() = default;
        int v;
    };
    struct Derived : Base {
        Derived(int i) { v = i; }
    };

    Derived                      d{99};
    expected<Derived&, int> src(d);
    expected<Base&, int>    dst = src;
    REQUIRE(dst.has_value());
    CHECK(dst->v == 99);
    CHECK(&*dst == static_cast<Base*>(&d));
}

// =============================================================================
// Rebind semantics on assignment
// =============================================================================

TEST_CASE("expected<T&>: rebind reference on assignment from lvalue", "[expected_ref]") {
    int                   x = 1, y = 2;
    expected<int&, int> e(x);
    e = y;
    CHECK(&*e == &y);
    CHECK(*e == 2);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: rebind does NOT assign through reference", "[expected_ref]") {
    int                   x = 100, y = 200;
    expected<int&, int> e(x);
    e = y;
    CHECK(x == 100);
    CHECK(*e == 200);
}

TEST_CASE("expected<T&>: assign from unexpected transitions to error state", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> e(x);
    e = unexpected(99);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 99);
    CHECK(x == 5);
}

TEST_CASE("expected<T&>: assign lvalue rebinds from error state", "[expected_ref]") {
    int                   x = 7;
    expected<int&, int> e = unexpected(1);
    e                       = x;
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

TEST_CASE("expected<T&>: copy assignment value-value rebinds", "[expected_ref]") {
    int                   x = 1, y = 2;
    expected<int&, int> a(x), b(y);
    a = b;
    CHECK(&*a == &y);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: copy assignment error-error copies error", "[expected_ref]") {
    expected<int&, int> a = unexpected(1);
    expected<int&, int> b = unexpected(2);
    a                       = b;
    CHECK(a.error() == 2);
}

TEST_CASE("expected<T&>: copy assignment value-to-error", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> a(x);
    expected<int&, int> b = unexpected(42);
    a                       = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 42);
}

TEST_CASE("expected<T&>: copy assignment error-to-value", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> a = unexpected(42);
    expected<int&, int> b(x);
    a                       = b;
    REQUIRE(a.has_value());
    CHECK(&*a == &x);
}

// =============================================================================
// Shallow const
// =============================================================================

TEST_CASE("expected<T&>: shallow const allows mutation of referent", "[expected_ref]") {
    int                         x = 10;
    const expected<int&, int> e(x);
    *e = 20;
    CHECK(x == 20);
}

TEST_CASE("expected<T&>: operator-> on const returns T*", "[expected_ref]") {
    int                         x = 5;
    const expected<int&, int> e(x);
    static_assert(std::is_same_v<decltype(e.operator->()), int*>);
    *e.operator->() = 99;
    CHECK(x == 99);
}

// =============================================================================
// Observers
// =============================================================================

TEST_CASE("expected<T&>: operator* returns T&", "[expected_ref]") {
    int                   x = 42;
    expected<int&, int> e(x);
    static_assert(std::is_same_v<decltype(*e), int&>);
    *e = 99;
    CHECK(x == 99);
}

TEST_CASE("expected<T&>: operator-> returns T*", "[expected_ref]") {
    struct S {
        int v;
    };
    S                   s{7};
    expected<S&, int> e(s);
    CHECK(e->v == 7);
    e->v = 99;
    CHECK(s.v == 99);
}

TEST_CASE("expected<T&>: value() returns T& or throws", "[expected_ref]") {
    int                   x = 1;
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

TEST_CASE("expected<T&>: value_or returns referred value when has value", "[expected_ref]") {
    int                   x = 42;
    expected<int&, int> e(x);
    int                   result = e.value_or(0);
    CHECK(result == 42);
}

TEST_CASE("expected<T&>: value_or returns fallback when has error", "[expected_ref]") {
    expected<int&, int> e = unexpected(0);
    int                   result = e.value_or(99);
    CHECK(result == 99);
}

TEST_CASE("expected<T&>: error_or returns error when has error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    CHECK(e.error_or(0) == 42);
}

TEST_CASE("expected<T&>: error_or returns default when has value", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> e(x);
    CHECK(e.error_or(99) == 99);
}

TEST_CASE("expected<T&>: bool conversion", "[expected_ref]") {
    int                   x = 1;
    expected<int&, int> val(x);
    expected<int&, int> err = unexpected(0);
    CHECK(static_cast<bool>(val));
    CHECK(!static_cast<bool>(err));
}

// =============================================================================
// emplace
// =============================================================================

TEST_CASE("expected<T&>: emplace rebinds from value", "[expected_ref]") {
    int                   x = 1, y = 2;
    expected<int&, int> e(x);
    e.emplace(y);
    CHECK(&*e == &y);
    CHECK(x == 1);
}

TEST_CASE("expected<T&>: emplace rebinds from error", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> e = unexpected(42);
    e.emplace(x);
    REQUIRE(e.has_value());
    CHECK(&*e == &x);
}

// =============================================================================
// Swap
// =============================================================================

TEST_CASE("expected<T&>: swap value-value rebinds pointers", "[expected_ref]") {
    int                   x = 1, y = 2;
    expected<int&, int> a(x), b(y);
    a.swap(b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
    CHECK(x == 1);
    CHECK(y == 2);
}

TEST_CASE("expected<T&>: swap value-error", "[expected_ref]") {
    int                   x = 1;
    expected<int&, int> a(x), b(unexpect, 99);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 99);
    CHECK(&*b == &x);
}

TEST_CASE("expected<T&>: swap error-error", "[expected_ref]") {
    expected<int&, int> a(unexpect, 1), b(unexpect, 2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

TEST_CASE("expected<T&>: friend swap", "[expected_ref]") {
    int                   x = 10, y = 20;
    expected<int&, int> a(x), b(y);
    swap(a, b);
    CHECK(&*a == &y);
    CHECK(&*b == &x);
}

// =============================================================================
// Equality
// =============================================================================

TEST_CASE("expected<T&>: equality with expected<T2&, E2>", "[expected_ref]") {
    int                   x = 5, y = 5, z = 6;
    expected<int&, int> a(x), b(y), c(z);
    CHECK(a == b);
    CHECK(!(a == c));
}

TEST_CASE("expected<T&>: equality with value type", "[expected_ref]") {
    int                   x = 42;
    expected<int&, int> e(x);
    CHECK(e == 42);
    CHECK(!(e == 99));
}

TEST_CASE("expected<T&>: equality with unexpected", "[expected_ref]") {
    expected<int&, int> e = unexpected(7);
    CHECK(e == unexpected(7));
    CHECK(!(e == unexpected(8)));
}

TEST_CASE("expected<T&>: inequality value vs error", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> val(x);
    expected<int&, int> err = unexpected(5);
    CHECK(!(val == err));
}

// =============================================================================
// Monadic operations
// =============================================================================

TEST_CASE("expected<T&>: and_then passes T& to callable", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> e(x);
    auto                  r = e.and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(r.has_value());
    CHECK(*r == 10);
}

TEST_CASE("expected<T&>: and_then on error forwards error", "[expected_ref]") {
    expected<int&, int> e = unexpected(42);
    auto                  r = e.and_then([](int& v) -> expected<int, int> { return v * 2; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 42);
}

TEST_CASE("expected<T&>: transform passes T& to callable", "[expected_ref]") {
    int                   x = 3;
    expected<int&, int> e(x);
    auto                  r = e.transform([](int& v) { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 4);
}

TEST_CASE("expected<T&>: transform on error forwards error", "[expected_ref]") {
    expected<int&, int> e = unexpected(99);
    auto                  r = e.transform([](int& v) { return v + 1; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == 99);
}

TEST_CASE("expected<T&>: or_else passes error to callable", "[expected_ref]") {
    expected<int&, int> e = unexpected(99);
    int                   x = 0;
    auto                  r = e.or_else([&](int v) -> expected<int&, int> {
        x = v;
        return unexpected(v);
    });
    CHECK(x == 99);
}

TEST_CASE("expected<T&>: or_else on value returns value", "[expected_ref]") {
    int                   x = 42;
    expected<int&, int> e(x);
    auto r = e.or_else([](int) -> expected<int&, int> { return unexpected(0); });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("expected<T&>: transform_error transforms error", "[expected_ref]") {
    expected<int&, int> e = unexpected(5);
    auto                  r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "5");
}

TEST_CASE("expected<T&>: transform_error on value preserves value", "[expected_ref]") {
    int                   x = 42;
    expected<int&, int> e(x);
    auto                  r = e.transform_error([](int v) -> std::string { return std::to_string(v); });
    REQUIRE(r.has_value());
    CHECK(*r == 42);
    CHECK(&*r == &x);
}

TEST_CASE("expected<T&>: transform to void", "[expected_ref]") {
    int                   x   = 5;
    int                   out = 0;
    expected<int&, int> e(x);
    auto                  r = e.transform([&](int& v) { out = v; });
    REQUIRE(r.has_value());
    CHECK(out == 5);
}

// =============================================================================
// const-qualified monadic operations
// =============================================================================

TEST_CASE("expected<T&>: const and_then", "[expected_ref]") {
    int                         x = 10;
    const expected<int&, int> e(x);
    auto                        r = e.and_then([](int& v) -> expected<int, int> { return v + 1; });
    REQUIRE(r.has_value());
    CHECK(*r == 11);
}

TEST_CASE("expected<T&>: const transform", "[expected_ref]") {
    int                         x = 3;
    const expected<int&, int> e(x);
    auto                        r = e.transform([](int& v) { return v * 3; });
    REQUIRE(r.has_value());
    CHECK(*r == 9);
}

// =============================================================================
// rvalue-qualified monadic operations
// =============================================================================

TEST_CASE("expected<T&>: rvalue and_then", "[expected_ref]") {
    int                   x = 5;
    expected<int&, int> e(x);
    auto                  r = std::move(e).and_then([](int& v) -> expected<int, int> { return v * 3; });
    REQUIRE(r.has_value());
    CHECK(*r == 15);
}

TEST_CASE("expected<T&>: rvalue transform_error moves error", "[expected_ref]") {
    expected<int&, std::string> e(unexpect, "hello");
    auto r = std::move(e).transform_error([](std::string&& s) -> std::string { return s + " world"; });
    REQUIRE(!r.has_value());
    CHECK(r.error() == "hello world");
}
