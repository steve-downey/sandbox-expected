# Test Plan: Step 2 — bad_expected_access<E>

**Standard sections:** [expected.bad] (22.8.4), [expected.bad.void] (22.8.5)
**Test file:** `tests/beman/expected/bad_expected_access.test.cpp`

---

## Testing Strategy

Same framework as Step 1: Catch2, include header twice for idempotence.
`bad_expected_access` has no Constraints or Hardened preconditions — only
Effects and Returns. There are no negative compile tests for this step.

---

## Testable Statements from the Standard

### [expected.bad.void] — base specialization

```cpp
template<> class bad_expected_access<void> : public exception { ... };
```

- Inherits from `std::exception`
- Protected constructors (default, copy, move) and assignments
- `what()` returns an implementation-defined ntbs

```cpp
static_assert(std::is_base_of_v<std::exception, bad_expected_access<void>>);
```

### [expected.bad] — primary template

```cpp
template<class E>
class bad_expected_access : public bad_expected_access<void> { ... };
```

- Inherits from `bad_expected_access<void>` (hence from `std::exception`)
- Constructor `explicit bad_expected_access(E e)` — initializes `unex` with `std::move(e)`
- `error()` — 4 ref-qualified overloads returning `unex`
- `what()` — returns an implementation-defined ntbs

---

## Test Outline

```cpp
#include <beman/expected/bad_expected_access.hpp>
#include <beman/expected/bad_expected_access.hpp>

#include <catch2/catch_test_macros.hpp>
#include <exception>
#include <string>
#include <utility>

using beman::expected::bad_expected_access;

// Inheritance
static_assert(std::is_base_of_v<std::exception, bad_expected_access<void>>);
static_assert(std::is_base_of_v<bad_expected_access<void>, bad_expected_access<int>>);
static_assert(std::is_base_of_v<std::exception, bad_expected_access<int>>);

TEST_CASE("bad_expected_access: construction stores error", "[bad_expected_access]") {
    bad_expected_access<int> ex(42);
    CHECK(ex.error() == 42);
}

TEST_CASE("bad_expected_access: constructor moves from E", "[bad_expected_access]") {
    // Effects: initializes unex with std::move(e)
    std::string s = "error msg";
    bad_expected_access<std::string> ex(std::move(s));
    CHECK(ex.error() == "error msg");
}

TEST_CASE("bad_expected_access: what() returns non-null", "[bad_expected_access]") {
    bad_expected_access<int> ex(0);
    const char* msg = ex.what();
    CHECK(msg != nullptr);
    CHECK(msg[0] != '\0');
}

TEST_CASE("bad_expected_access: catchable as std::exception", "[bad_expected_access]") {
    bool caught = false;
    try {
        throw bad_expected_access<int>(99);
    } catch (const std::exception& e) {
        caught = true;
        CHECK(e.what() != nullptr);
    }
    CHECK(caught);
}

TEST_CASE("bad_expected_access: error() lvalue ref", "[bad_expected_access]") {
    bad_expected_access<int> ex(10);
    static_assert(std::is_same_v<decltype(ex.error()), int&>);
    ex.error() = 20;          // error() & returns non-const ref
    CHECK(ex.error() == 20);
}

TEST_CASE("bad_expected_access: error() const lvalue ref", "[bad_expected_access]") {
    const bad_expected_access<int> ex(10);
    static_assert(std::is_same_v<decltype(ex.error()), const int&>);
    CHECK(ex.error() == 10);
}

TEST_CASE("bad_expected_access: error() rvalue ref", "[bad_expected_access]") {
    bad_expected_access<int> ex(10);
    static_assert(std::is_same_v<decltype(std::move(ex).error()), int&&>);
    int v = std::move(ex).error();
    CHECK(v == 10);
}

TEST_CASE("bad_expected_access: error() const rvalue ref", "[bad_expected_access]") {
    const bad_expected_access<int> ex(10);
    static_assert(
        std::is_same_v<decltype(std::move(ex).error()), const int&&>);
    const int&& r = std::move(ex).error();
    CHECK(r == 10);
}

TEST_CASE("bad_expected_access: move-only error type", "[bad_expected_access]") {
    // E is move-constructible: bad_expected_access(E) uses std::move
    struct MoveOnly {
        int v;
        MoveOnly(int x) : v(x) {}
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly(MoveOnly&&) = default;
    };
    bad_expected_access<MoveOnly> ex(MoveOnly{7});
    CHECK(ex.error().v == 7);
}

TEST_CASE("bad_expected_access<void>: what() non-null", "[bad_expected_access]") {
    // bad_expected_access<void> can be constructed only by derived classes
    // (protected ctor), but we can test via a derived instance
    bad_expected_access<int> ex(0);
    const bad_expected_access<void>& base = ex;
    CHECK(base.what() != nullptr);
}
```

---

## Return value conventions

The standard says `what()` returns "an implementation-defined ntbs". The de
facto convention (libstdc++, libc++) is `"bad expected access"`. It is valid
to test `std::string_view(ex.what()).contains("bad")` but not mandatory.

---

## No negative compile tests

`bad_expected_access` has no Constraints or Mandates on its API surface.
The only type requirement on E is that it be Cpp17Destructible, which is
handled at the `expected` template level, not here.
