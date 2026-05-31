# Test Plan: Step 5 — Monadic Operations for expected<T, E>

**Standard section:** [expected.object.monadic] (22.8.6.7)
**Test file:** `tests/beman/expected/expected_monadic.test.cpp`

---

## Testing Strategy

Catch2. Include `<beman/expected/expected.hpp>` twice for idempotence.

Monadic operations have two categories of requirements:
- **Constraints** (function removed from overload resolution)
- **Mandates** (ill-formed at instantiation — program is ill-formed if violated)

The Mandates here are on the return type of the callable: e.g., for `and_then`,
the callable must return a specialization of `expected` with the same error
type. These are *not* constraints — they cannot be detected with
`!is_invocable_v` at overload resolution; the program is simply ill-formed.
Test them with negative compile files.

---

## and_then [expected.object.monadic] para 1–8

### `and_then(F)` — lvalue and const-lvalue overloads (para 1–4)

`U = remove_cvref_t<invoke_result_t<F, decltype((val))>>`

- **Constraint:** `is_constructible_v<E, decltype(error())>` is true
- **Mandates:** U is a specialization of expected; `U::error_type` is `E`
- **Effects:** if has_value(), return `invoke(f, val)`; else return `U(unexpect, error())`

### `and_then(F)` — rvalue and const-rvalue overloads (para 5–8)

`U = remove_cvref_t<invoke_result_t<F, decltype(std::move(val))>>`

- **Constraint:** `is_constructible_v<E, decltype(std::move(error()))>` is true
- **Mandates:** same as above
- **Effects:** if has_value(), return `invoke(f, std::move(val))`; else return `U(unexpect, std::move(error()))`

---

## or_else [expected.object.monadic] para 9–16

`G = remove_cvref_t<invoke_result_t<F, decltype(error())>>` (lvalue overloads)

- **Constraint:** `is_constructible_v<T, decltype((val))>` (lvalue overloads)
- **Constraint:** `is_constructible_v<T, decltype(std::move(val))>` (rvalue overloads)
- **Mandates:** G is a specialization of expected; `G::value_type` is `T`
- **Effects:** if has_value(), return `G(in_place, val)`; else return `invoke(f, error())`

---

## transform [expected.object.monadic] para 17–24

`U = remove_cv_t<invoke_result_t<F, decltype((val))>>`

- **Constraint:** `is_constructible_v<E, decltype(error())>`
- **Mandates:** U is a valid value type for expected; if !is_void_v<U>, `U u(invoke(f, val))` is well-formed
- **Effects (three branches):**
  - has_value() == false → return `expected<U, E>(unexpect, error())`
  - has_value() && !is_void_v<U> → return expected<U, E> with invoke result
  - has_value() && is_void_v<U> → invoke(f, val), return `expected<U, E>()`

---

## transform_error [expected.object.monadic] para 25–32

`G = remove_cv_t<invoke_result_t<F, decltype(error())>>`

- **Constraint:** `is_constructible_v<T, decltype((val))>`
- **Mandates:** G is a valid template arg for unexpected; `G g(invoke(f, error()))` well-formed
- **Effects:** if has_value(), return `expected<T, G>(in_place, val)`; else return with invoke on error

---

## Test Outline

```cpp
#include <beman/expected/expected.hpp>
#include <beman/expected/expected.hpp>

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <string>

using namespace beman::expected;

// ---------------------------------------------------------------------------
// and_then
// ---------------------------------------------------------------------------

TEST_CASE("and_then: has value — calls F", "[expected_monadic]") {
    expected<int, std::string> e(42);
    auto result = e.and_then([](int v) -> expected<int, std::string> {
        return v * 2;
    });
    REQUIRE(result.has_value());
    CHECK(*result == 84);
}

TEST_CASE("and_then: has error — short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "fail");
    bool called = false;
    auto result = e.and_then([&](int) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(!result.has_value());
    CHECK(result.error() == "fail");
}

TEST_CASE("and_then: chaining", "[expected_monadic]") {
    expected<int, std::string> e(1);
    auto r = e
        .and_then([](int v) -> expected<int, std::string> { return v + 1; })
        .and_then([](int v) -> expected<int, std::string> { return v * 10; });
    REQUIRE(r.has_value());
    CHECK(*r == 20);
}

TEST_CASE("and_then: rvalue overload moves value", "[expected_monadic]") {
    expected<std::string, int> e("hello");
    std::string moved_from;
    auto r = std::move(e).and_then(
        [&](std::string s) -> expected<std::string, int> {
            moved_from = std::move(s);
            return "done";
        });
    CHECK(moved_from == "hello");
    REQUIRE(r.has_value());
}

TEST_CASE("and_then: const lvalue overload", "[expected_monadic]") {
    const expected<int, int> e(5);
    auto r = e.and_then([](int v) -> expected<long, int> { return v; });
    REQUIRE(r.has_value());
    CHECK(*r == 5L);
}

// ---------------------------------------------------------------------------
// or_else
// ---------------------------------------------------------------------------

TEST_CASE("or_else: has error — calls F", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "problem");
    auto result = e.or_else([](std::string s) -> expected<int, std::string> {
        return s.size();
    });
    REQUIRE(result.has_value());
    CHECK(*result == 7);
}

TEST_CASE("or_else: has value — short-circuits", "[expected_monadic]") {
    expected<int, std::string> e(42);
    bool called = false;
    auto result = e.or_else([&](std::string) -> expected<int, std::string> {
        called = true;
        return 0;
    });
    CHECK(!called);
    REQUIRE(result.has_value());
    CHECK(*result == 42);
}

TEST_CASE("or_else: rvalue overload moves error", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    std::string got;
    auto r = std::move(e).or_else(
        [&](std::string s) -> expected<int, std::string> {
            got = std::move(s);
            return 0;
        });
    CHECK(got == "err");
}

// ---------------------------------------------------------------------------
// transform
// ---------------------------------------------------------------------------

TEST_CASE("transform: has value — transforms", "[expected_monadic]") {
    expected<int, std::string> e(6);
    auto r = e.transform([](int v) { return v * 7; });
    static_assert(std::is_same_v<decltype(r), expected<int, std::string>>);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform: has error — propagates", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "oops");
    bool called = false;
    auto r = e.transform([&](int) { called = true; return 0; });
    CHECK(!called);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "oops");
}

TEST_CASE("transform: void return type", "[expected_monadic]") {
    expected<int, std::string> e(1);
    int count = 0;
    auto r = e.transform([&](int) { ++count; });
    static_assert(std::is_same_v<decltype(r), expected<void, std::string>>);
    REQUIRE(r.has_value());
    CHECK(count == 1);
}

TEST_CASE("transform: type change", "[expected_monadic]") {
    expected<int, int> e(42);
    auto r = e.transform([](int v) -> std::string { return std::to_string(v); });
    static_assert(std::is_same_v<decltype(r), expected<std::string, int>>);
    REQUIRE(r.has_value());
    CHECK(*r == "42");
}

TEST_CASE("transform: rvalue overload", "[expected_monadic]") {
    expected<std::string, int> e("hello");
    auto r = std::move(e).transform([](std::string s) { return s.size(); });
    REQUIRE(r.has_value());
    CHECK(*r == 5u);
}

// ---------------------------------------------------------------------------
// transform_error
// ---------------------------------------------------------------------------

TEST_CASE("transform_error: has error — transforms", "[expected_monadic]") {
    expected<int, int> e(unexpect, 3);
    auto r = e.transform_error([](int v) -> std::string {
        return std::to_string(v);
    });
    static_assert(std::is_same_v<decltype(r), expected<int, std::string>>);
    REQUIRE(!r.has_value());
    CHECK(r.error() == "3");
}

TEST_CASE("transform_error: has value — preserves", "[expected_monadic]") {
    expected<int, int> e(42);
    bool called = false;
    auto r = e.transform_error([&](int) -> std::string { called = true; return ""; });
    CHECK(!called);
    REQUIRE(r.has_value());
    CHECK(*r == 42);
}

TEST_CASE("transform_error: rvalue overload moves error", "[expected_monadic]") {
    expected<int, std::string> e(unexpect, "err");
    std::string got;
    auto r = std::move(e).transform_error([&](std::string s) -> int {
        got = std::move(s);
        return 99;
    });
    CHECK(got == "err");
    REQUIRE(!r.has_value());
    CHECK(r.error() == 99);
}

// ---------------------------------------------------------------------------
// Mixed chaining
// ---------------------------------------------------------------------------

TEST_CASE("monadic chaining", "[expected_monadic]") {
    auto parse = [](const std::string& s) -> expected<int, std::string> {
        if (s.empty()) return unexpected<std::string>("empty");
        return std::stoi(s);
    };
    auto double_it = [](int v) -> expected<int, std::string> { return v * 2; };
    auto to_string = [](int v) { return std::to_string(v); };

    auto r = parse("21").and_then(double_it).transform(to_string);
    REQUIRE(r.has_value());
    CHECK(*r == "42");

    auto r2 = parse("").and_then(double_it).transform(to_string);
    REQUIRE(!r2.has_value());
}

// ---------------------------------------------------------------------------
// Constraint checks
// ---------------------------------------------------------------------------

// and_then constraint: is_constructible_v<E, decltype(error())>
// When E is not copy-constructible from error(), and_then is constrained out
// (hard to test without reflection — leave as documentation)

// Mandates: callable must return expected<?, E> with same E
// This is a compile-time mandate — test via negative compile file

// ---------------------------------------------------------------------------
// Ref-qualification tests
// ---------------------------------------------------------------------------

TEST_CASE("and_then: all 4 ref qualifications compile", "[expected_monadic]") {
    using E = expected<int, std::string>;
    auto f = [](int v) -> E { return v; };

    E e(1);
    (void)(e.and_then(f));
    (void)(std::as_const(e).and_then(f));
    (void)(std::move(e).and_then(f));
    // const&& — rarely useful but must compile
    (void)(std::move(std::as_const(e)).and_then(f));
}
```

---

## Negative Compile Tests

### `and_then_wrong_error_type_fail.cpp`
```cpp
// NEGATIVE: and_then Mandates U::error_type == E
// F returns expected<int, double> but E is std::string — ill-formed
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, std::string> e(1);
    e.and_then([](int v) -> beman::expected::expected<int, double> {
        return v;
    });
}
```

### `and_then_not_expected_fail.cpp`
```cpp
// NEGATIVE: and_then Mandates U is a specialization of expected
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, int> e(1);
    e.and_then([](int v) -> int { return v; });  // int is not expected
}
```

### `or_else_wrong_value_type_fail.cpp`
```cpp
// NEGATIVE: or_else Mandates G::value_type == T
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, int> e(beman::expected::unexpect, 1);
    e.or_else([](int v) -> beman::expected::expected<long, int> { return v; });
}
```

### `transform_error_not_valid_unexpected_arg_fail.cpp`
```cpp
// NEGATIVE: transform_error Mandates G is valid for unexpected
// (not a reference, not an array, etc.)
#include <beman/expected/expected.hpp>
void test() {
    beman::expected::expected<int, int> e(beman::expected::unexpect, 1);
    e.transform_error([](int) -> int& {
        static int x = 0;
        return x;
    });
}
```
