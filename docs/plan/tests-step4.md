# Test Plan: Step 4 — expected<void, E> Specialization

**Standard section:** [expected.void] (22.8.7)
**Test file:** `tests/beman/expected/expected_void.test.cpp`
**Negative-compile files:** `tests/beman/expected/expected_void_*_fail.cpp`

---

## Testing Strategy

Catch2. Include header twice. This step tests the partial specialization
`expected<T, E> requires is_void_v<T>` (i.e., `expected<void, E>`).

Key differences from the primary template that need targeted tests:
- No value storage, no `operator->`, no `value_or`
- `operator*()` returns `void`
- `value()` returns `void` (or throws)
- `emplace()` takes no args
- Default constructor is `noexcept` (no T to initialize)
- Converting ctors require `is_void_v<U>` (constraint 13.1)

---

## Ill-Formed Instantiations [expected.void.general] para 2

> A program that instantiates the definition of the template expected<T, E>
> with a type for the E parameter that is not a valid template argument for
> unexpected is ill-formed.

Same E constraints as primary template: E must not be a reference, array,
`unexpected<X>` specialization, or cv-qualified.

```cpp
// These should all be caught by static_assert in the implementation:
static_assert(!std::is_constructible_v<expected<void, int&>>);
static_assert(!std::is_constructible_v<expected<void, int[]>>);
```

---

## Constructors [expected.void.cons]

### Default constructor
- **noexcept** (no T initialization can throw)
- **Postcondition:** `has_value() == true`

```cpp
TEST_CASE("expected<void>: default construct", "[expected_void]") {
    expected<void, int> e;
    CHECK(e.has_value());
    static_assert(std::is_nothrow_default_constructible_v<expected<void, int>>);
}
```

### Copy constructor
- **Deleted unless** `is_copy_constructible_v<E>`
- **Trivial if** `is_trivially_copy_constructible_v<E>`

```cpp
TEST_CASE("expected<void>: copy construct with value", "[expected_void]") {
    expected<void, int> a;
    expected<void, int> b = a;
    CHECK(b.has_value());
}

TEST_CASE("expected<void>: copy construct with error", "[expected_void]") {
    expected<void, int> a(unexpect, 42);
    expected<void, int> b = a;
    REQUIRE(!b.has_value());
    CHECK(b.error() == 42);
}

struct NoCopyE { NoCopyE(const NoCopyE&) = delete; NoCopyE() = default; };
static_assert(!std::is_copy_constructible_v<expected<void, NoCopyE>>);

static_assert(std::is_trivially_copy_constructible_v<expected<void, int>>);
```

### Move constructor
- **Constraint:** `is_move_constructible_v<E>`
- **noexcept iff** `is_nothrow_move_constructible_v<E>`
- **Trivial if** `is_trivially_move_constructible_v<E>`

```cpp
TEST_CASE("expected<void>: move construct with error", "[expected_void]") {
    expected<void, std::string> a(unexpect, "err");
    expected<void, std::string> b = std::move(a);
    REQUIRE(!b.has_value());
    CHECK(b.error() == "err");
}

static_assert(std::is_nothrow_move_constructible_v<expected<void, int>>);
```

### Converting constructor from expected<U, G> (void case)
- **Constraint 13.1:** `is_void_v<U>` — only converts from another void expected
- **Constraints 13.2–13.6:** constructibility and anti-hijacking

```cpp
TEST_CASE("expected<void>: convert from expected<void, G>", "[expected_void]") {
    expected<void, int> src;
    expected<void, long> dst = src;
    CHECK(dst.has_value());
}

TEST_CASE("expected<void>: convert from expected<void, G> with error", "[expected_void]") {
    expected<void, int> src(unexpect, 7);
    expected<void, long> dst = src;
    REQUIRE(!dst.has_value());
    CHECK(dst.error() == 7L);
}

// Constraint 13.1: U must be void — cannot convert from expected<int, G>
static_assert(!std::is_constructible_v<
    expected<void, int>,
    expected<int, int>>);
```

### Constructors from unexpected<G>
- **Constraint:** `is_constructible_v<E, GF>`
- **Postcondition:** `has_value() == false`

```cpp
TEST_CASE("expected<void>: construct from unexpected", "[expected_void]") {
    expected<void, std::string> e = unexpected<std::string>("fail");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "fail");
}
```

### in_place_t constructor
- **No arguments** (void, so nothing to initialize)
- **noexcept**
- **Postcondition:** `has_value() == true`

```cpp
TEST_CASE("expected<void>: in_place_t constructor", "[expected_void]") {
    expected<void, int> e(std::in_place);
    CHECK(e.has_value());
    static_assert(noexcept(expected<void, int>(std::in_place)));
}
```

### unexpect_t constructors
- `(unexpect_t, Args...)`: `is_constructible_v<E, Args...>`
- `(unexpect_t, ilist, Args...)`: `is_constructible_v<E, ilist<U>&, Args...>`

```cpp
TEST_CASE("expected<void>: unexpect_t constructor", "[expected_void]") {
    expected<void, std::string> e(unexpect, 3, 'x');
    REQUIRE(!e.has_value());
    CHECK(e.error() == "xxx");
}

TEST_CASE("expected<void>: unexpect_t ilist constructor", "[expected_void]") {
    expected<void, std::vector<int>> e(unexpect, {1, 2, 3});
    REQUIRE(!e.has_value());
    CHECK(e->error().size() == 3);  // NB: e.error() not e->error()
}
```

---

## Destructor [expected.void.dtor]

- Effects: if `has_value() == false`, destroys `unex`
- **Trivial if** `is_trivially_destructible_v<E>`

```cpp
static_assert(std::is_trivially_destructible_v<expected<void, int>>);

TEST_CASE("expected<void>: destructor destroys error", "[expected_void]") {
    int destroyed = 0;
    struct Counted { int* d; ~Counted() { ++*d; } };
    {
        expected<void, Counted> e(unexpect, Counted{&destroyed});
        (void)e;
    }
    CHECK(destroyed >= 1);
}
```

---

## Assignment [expected.void.assign]

### Copy assignment
- **Deleted unless** `is_copy_assignable_v<E> && is_copy_constructible_v<E>`
- **Trivial if** trivially copy-constructible, copy-assignable, destructible E
- Four state transitions: value→value (no-op), value→error, error→value, error→error

```cpp
TEST_CASE("expected<void>: copy assign value-to-value (no-op)", "[expected_void]") {
    expected<void, int> a, b;
    a = b;
    CHECK(a.has_value());
}

TEST_CASE("expected<void>: copy assign error-to-value", "[expected_void]") {
    expected<void, int> a;
    expected<void, int> b(unexpect, 5);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 5);
}

TEST_CASE("expected<void>: copy assign value-to-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b;
    a = b;
    CHECK(a.has_value());
}

TEST_CASE("expected<void>: copy assign error-to-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 2);
    a = b;
    CHECK(a.error() == 2);
}
```

### Move assignment
- **Constraint:** `is_move_constructible_v<E> && is_move_assignable_v<E>`
- **noexcept iff** `is_nothrow_move_constructible_v<E> && is_nothrow_move_assignable_v<E>`

```cpp
TEST_CASE("expected<void>: move assign", "[expected_void]") {
    expected<void, std::string> a, b(unexpect, "err");
    a = std::move(b);
    REQUIRE(!a.has_value());
    CHECK(a.error() == "err");
}

static_assert(std::is_nothrow_move_assignable_v<expected<void, int>>);
```

### Assign from unexpected<G>
- **Constraint:** `is_constructible_v<E, GF> && is_assignable_v<E&, GF>`
- Two states: has-value (constructs error), has-error (assigns error)

```cpp
TEST_CASE("expected<void>: assign from unexpected", "[expected_void]") {
    expected<void, int> e;
    e = unexpected(42);
    REQUIRE(!e.has_value());
    CHECK(e.error() == 42);
}

TEST_CASE("expected<void>: assign from unexpected when already error", "[expected_void]") {
    expected<void, int> e(unexpect, 1);
    e = unexpected(2);
    CHECK(e.error() == 2);
}
```

### emplace()
- No arguments, **noexcept**
- Effects: if `has_value() == false`, destroys unex and sets `has_val_ = true`

```cpp
TEST_CASE("expected<void>: emplace from error state", "[expected_void]") {
    expected<void, int> e(unexpect, 5);
    e.emplace();
    CHECK(e.has_value());
    static_assert(noexcept(e.emplace()));
}

TEST_CASE("expected<void>: emplace from value state (no-op)", "[expected_void]") {
    expected<void, int> e;
    e.emplace();
    CHECK(e.has_value());
}
```

---

## Swap [expected.void.swap]

- **Constraints:** `is_swappable_v<E> && is_move_constructible_v<E>`
- **noexcept iff** `is_nothrow_move_constructible_v<E> && is_nothrow_swappable_v<E>`
- Four state combinations

```cpp
TEST_CASE("expected<void>: swap value-value (no-op)", "[expected_void]") {
    expected<void, int> a, b;
    a.swap(b);
    CHECK(a.has_value());
    CHECK(b.has_value());
}

TEST_CASE("expected<void>: swap value-error", "[expected_void]") {
    expected<void, int> a, b(unexpect, 7);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 7);
}

TEST_CASE("expected<void>: swap error-error", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}
```

---

## Observers [expected.void.obs]

### operator bool() / has_value()

```cpp
TEST_CASE("expected<void>: has_value and bool", "[expected_void]") {
    expected<void, int> a, b(unexpect, 0);
    CHECK(a.has_value());
    CHECK(bool(a));
    CHECK(!b.has_value());
    CHECK(!bool(b));
}
```

### operator*() — void, noexcept, Hardened precondition: has_value()

```cpp
TEST_CASE("expected<void>: operator* is void", "[expected_void]") {
    expected<void, int> e;
    static_assert(std::is_same_v<decltype(*e), void>);
    *e;  // compiles and does nothing
}

// No operator-> for void
static_assert(!requires(expected<void, int> e) { e.operator->(); });
```

### value() — returns void, throws when empty
- **Mandates (const& overload):** `is_copy_constructible_v<E>`
- **Mandates (&& overload):** `is_copy_constructible_v<E> && is_move_constructible_v<E>`
- **Throws:** `bad_expected_access(error())` or `bad_expected_access(std::move(error()))`

```cpp
TEST_CASE("expected<void>: value() on success is no-op", "[expected_void]") {
    expected<void, int> e;
    e.value();  // should not throw
}

TEST_CASE("expected<void>: value() throws on error", "[expected_void]") {
    expected<void, int> e(unexpect, 7);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected<void>: rvalue value() throws on error", "[expected_void]") {
    expected<void, int> e(unexpect, 5);
    REQUIRE_THROWS_AS(
        std::move(e).value(),
        beman::expected::bad_expected_access<int>);
}
```

### error() — Hardened precondition: !has_value()

```cpp
TEST_CASE("expected<void>: error() all ref qualifications", "[expected_void]") {
    expected<void, int> e(unexpect, 99);
    static_assert(std::is_same_v<decltype(e.error()), int&>);
    static_assert(std::is_same_v<decltype(std::as_const(e).error()), const int&>);
    static_assert(std::is_same_v<decltype(std::move(e).error()), int&&>);
    CHECK(e.error() == 99);
}
```

### error_or()
- **Mandates (const& overload):** `is_copy_constructible_v<E> && is_convertible_v<G, E>`
- **Mandates (&& overload):** `is_move_constructible_v<E> && is_convertible_v<G, E>`

```cpp
TEST_CASE("expected<void>: error_or with value", "[expected_void]") {
    expected<void, int> e;
    CHECK(e.error_or(99) == 99);
}

TEST_CASE("expected<void>: error_or with error", "[expected_void]") {
    expected<void, int> e(unexpect, 7);
    CHECK(e.error_or(0) == 7);
}
```

---

## Equality Operators [expected.void.eq]

### expected<void> == expected<void, E2>
- **Requires** `is_void_v<T2>`
- If both have values → `true`; if mixed → `false`; if both error → `x.error() == y.error()`

```cpp
TEST_CASE("expected<void>: equality both value", "[expected_void]") {
    expected<void, int> a, b;
    CHECK(a == b);
}

TEST_CASE("expected<void>: equality both error same", "[expected_void]") {
    expected<void, int> a(unexpect, 1), b(unexpect, 1);
    CHECK(a == b);
}

TEST_CASE("expected<void>: equality mixed", "[expected_void]") {
    expected<void, int> a, b(unexpect, 0);
    CHECK(!(a == b));
}
```

### expected<void> == unexpected<E2>

```cpp
TEST_CASE("expected<void>: equality with unexpected", "[expected_void]") {
    expected<void, int> a(unexpect, 3), b;
    CHECK(a == unexpected(3));
    CHECK(!(b == unexpected(3)));
}
```

---

## No-value members (confirm absence)

```cpp
// expected<void, E> has no operator->, value_or
static_assert(!requires(expected<void, int> e) { e.operator->(); });
static_assert(!requires(expected<void, int> e) { e.value_or(0); });
```
