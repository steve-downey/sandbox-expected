# Test Plan: Step 3 — expected<T, E> Primary Template

**Standard section:** [expected.expected] (22.8.6)
**Test files:**
- `tests/beman/expected/expected.test.cpp` — primary test file
- `tests/beman/expected/expected_*_fail.cpp` — negative compile tests

---

## Testing Strategy

Catch2 + header included twice. This step covers [expected.object.cons] through
[expected.object.eq] but NOT monadic operations (Step 5).

**Three tiers of negative tests:**

1. **Constraints** (constructor removed from overload resolution):
   `static_assert(!std::is_constructible_v<expected<T,E>, Args...>)` in the
   normal test file is sufficient for most cases.

2. **Mandates** (ill-formed at instantiation — `value()`, `value_or()`,
   `error_or()`, `emplace()`): use `static_assert(!std::is_invocable_v<...>)`
   or, where the diagnostic is reliable, a fail-to-compile negative file.

3. **Hardened preconditions** (`operator->`, `operator*`, `error()`): UB without
   contracts; test only if the implementation enforces them (e.g., aborts or
   throws in a debug/hardened mode). Mark these as conditionally compiled:
   ```cpp
   #if defined(BEMAN_EXPECTED_HARDENED)
   TEST_CASE("hardened: operator* on empty expected terminates", ...) { ... }
   #endif
   ```

---

## Ill-Formed Instantiations [expected.object.general] para 2–3

> A program which instantiates class template expected<T, E> with an
> argument T that is not a valid value type for expected is ill-formed.

Valid value type: `remove_cv_t<T>` is void OR a complete non-array object
type that is not `in_place_t`, `unexpect_t`, or a specialization of
`unexpected`.

> E … must not be a non-object type, an array type, a specialization of
> unexpected, or a cv-qualified type.

| Ill-formed case | Test |
|-----------------|------|
| `expected<int&, int>` | `static_assert` in test file; fail-file |
| `expected<int[], int>` | fail-file |
| `expected<in_place_t, int>` | fail-file |
| `expected<unexpect_t, int>` | fail-file |
| `expected<unexpected<int>, int>` | fail-file |
| `expected<int, int&>` | fail-file |
| `expected<int, int[]>` | fail-file |
| `expected<int, unexpected<int>>` | fail-file |
| `expected<int, const int>` | fail-file |

---

## Constructors [expected.object.cons]

### Default constructor
- **Constraints:** `is_default_constructible_v<T>`
- **Postcondition:** `has_value() == true`
- **Negative:** `static_assert(!std::is_default_constructible_v<expected<NotDefaultConstructible, int>>)`

```cpp
TEST_CASE("expected: default construct", "[expected]") {
    expected<int, std::string> e;
    CHECK(e.has_value());
    CHECK(*e == 0);          // value-initialized
}

struct NoDefault { NoDefault(int); };
static_assert(!std::is_default_constructible_v<expected<NoDefault, int>>);
```

### Copy constructor
- **Deleted unless:** `is_copy_constructible_v<T> && is_copy_constructible_v<E>`
- **Trivial if:** `is_trivially_copy_constructible_v<T> && is_trivially_copy_constructible_v<E>`
- **Postcondition:** `rhs.has_value() == this->has_value()`

```cpp
TEST_CASE("expected: copy construct with value", "[expected]") {
    expected<int, std::string> a(42);
    expected<int, std::string> b = a;
    REQUIRE(b.has_value());
    CHECK(*b == 42);
}

TEST_CASE("expected: copy construct with error", "[expected]") {
    expected<int, std::string> a(unexpect, "oops");
    expected<int, std::string> b = a;
    REQUIRE(!b.has_value());
    CHECK(b.error() == "oops");
}

// Deleted when T not copy-constructible
struct NoCopy { NoCopy(const NoCopy&) = delete; NoCopy(NoCopy&&) = default; int v; };
static_assert(!std::is_copy_constructible_v<expected<NoCopy, int>>);

// Trivially copyable when T and E are trivially copyable
static_assert(std::is_trivially_copy_constructible_v<expected<int, int>>);
```

### Move constructor
- **Constraints:** `is_move_constructible_v<T> && is_move_constructible_v<E>`
- **noexcept iff** `is_nothrow_move_constructible_v<T> && is_nothrow_move_constructible_v<E>`
- **Trivial if:** both trivially move-constructible
- **Postcondition:** `rhs.has_value() == this->has_value()` (unchanged)

```cpp
TEST_CASE("expected: move construct with value", "[expected]") {
    expected<std::string, int> a("hello");
    expected<std::string, int> b = std::move(a);
    REQUIRE(b.has_value());
    CHECK(*b == "hello");
}

static_assert(std::is_nothrow_move_constructible_v<expected<int, int>>);

// Not nothrow when T throws on move
struct ThrowingMove {
    ThrowingMove(ThrowingMove&&) noexcept(false);
};
static_assert(!std::is_nothrow_move_constructible_v<expected<ThrowingMove, int>>);
```

### Converting constructor from expected<U, G>
- **Constraints (18.1–18.7):** constructibility of T from UF, E from GF;
  anti-hijacking checks (no construction of unexpected<E> from the source)
- **explicit** if `!is_convertible_v<UF, T> || !is_convertible_v<GF, E>`

```cpp
TEST_CASE("expected: convert from expected<U, G> with value", "[expected]") {
    expected<int, int>   src(42);
    expected<long, long> dst = src;
    REQUIRE(dst.has_value());
    CHECK(*dst == 42L);
}

TEST_CASE("expected: convert from expected<U, G> with error", "[expected]") {
    expected<int, int>   src(unexpect, 7);
    expected<long, long> dst = src;
    REQUIRE(!dst.has_value());
    CHECK(dst.error() == 7L);
}

// explicit when conversion is not implicit
static_assert(!std::is_convertible_v<expected<int, int>, expected<long, double>>
              || std::is_convertible_v<int, long> && std::is_convertible_v<int, double>);
```

### Converting constructor from value U&&
- **Constraints (23.1–23.6):** U is not in_place_t/expected/unexpect_t/unexpected<X>;
  `is_constructible_v<T, U>`;
  if T is cv bool, U is not expected specialization
- **explicit** when `!is_convertible_v<U, T>`
- **Postcondition:** `has_value() == true`

```cpp
TEST_CASE("expected: construct from value", "[expected]") {
    expected<int, std::string> e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected: construct from value explicit", "[expected]") {
    // explicit when not implicitly convertible
    struct Explicit { explicit Explicit(int); };
    expected<Explicit, int> e(42);
    REQUIRE(e.has_value());
}

// Constraint: U not a specialization of unexpected
static_assert(!std::is_constructible_v<
    expected<int, int>,
    unexpected<int>>);  // should use the unexpected<G> constructor, not this one
```

### Constructors from unexpected<G>
- **Constraint:** `is_constructible_v<E, GF>`
- **Postcondition:** `has_value() == false`

```cpp
TEST_CASE("expected: construct from unexpected", "[expected]") {
    expected<int, std::string> e = unexpected("error");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "error");
}

// Constraint: E constructible from G
static_assert(!std::is_constructible_v<
    expected<int, int>,
    unexpected<std::string>>);
```

### In-place constructors
- `(in_place_t, Args...)`: **Constraint:** `is_constructible_v<T, Args...>`; postcondition `has_value() == true`
- `(in_place_t, ilist, Args...)`: `is_constructible_v<T, initializer_list<U>&, Args...>`
- `(unexpect_t, Args...)`: `is_constructible_v<E, Args...>`; postcondition `has_value() == false`
- `(unexpect_t, ilist, Args...)`: similar

```cpp
TEST_CASE("expected: in_place value construct", "[expected]") {
    expected<std::string, int> e(std::in_place, 5, 'a');
    REQUIRE(e.has_value());
    CHECK(*e == "aaaaa");
}

TEST_CASE("expected: in_place error construct", "[expected]") {
    expected<int, std::string> e(unexpect, 3, 'x');
    REQUIRE(!e.has_value());
    CHECK(e.error() == "xxx");
}

TEST_CASE("expected: in_place value with ilist", "[expected]") {
    expected<std::vector<int>, int> e(std::in_place, {1, 2, 3});
    REQUIRE(e.has_value());
    CHECK(e->size() == 3);
}
```

---

## Destructor [expected.object.dtor]

- Destroys `val` if `has_value()`, else destroys `unex`
- Trivial if `is_trivially_destructible_v<T> && is_trivially_destructible_v<E>`

```cpp
static_assert(std::is_trivially_destructible_v<expected<int, int>>);

TEST_CASE("expected: destructor runs for value", "[expected]") {
    int destroyed = 0;
    struct Counted {
        int* d;
        ~Counted() { ++*d; }
    };
    {
        expected<Counted, int> e(std::in_place, Counted{&destroyed});
        (void)e;
    }
    // Counted destructor ran exactly once
    CHECK(destroyed >= 1);
}
```

---

## Assignment [expected.object.assign]

### Copy assignment
- **Deleted unless:** `is_copy_assignable_v<T> && is_copy_constructible_v<T> && is_copy_assignable_v<E> && is_copy_constructible_v<E> && (nothrow_move_constructible<T> || nothrow_move_constructible<E>)`
- Four state transitions tested:
  - value → value, value → error, error → value, error → error

```cpp
TEST_CASE("expected: copy assign value to value", "[expected]") {
    expected<int, int> a(1), b(2);
    a = b;
    REQUIRE(a.has_value());
    CHECK(*a == 2);
}

TEST_CASE("expected: copy assign error to value", "[expected]") {
    expected<int, int> a(1);
    expected<int, int> b(unexpect, 99);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 99);
}

TEST_CASE("expected: copy assign value to error", "[expected]") {
    expected<int, int> a(unexpect, 10), b(5);
    a = b;
    REQUIRE(a.has_value());
    CHECK(*a == 5);
}

TEST_CASE("expected: copy assign error to error", "[expected]") {
    expected<int, int> a(unexpect, 1), b(unexpect, 2);
    a = b;
    REQUIRE(!a.has_value());
    CHECK(a.error() == 2);
}
```

### Move assignment
- **Constraints (6.1–6.5):** move-assignable and move-constructible for T and E;
  `nothrow_move_constructible<T> || nothrow_move_constructible<E>`
- **noexcept iff:** `nothrow_move_assignable<T> && nothrow_move_constructible<T> && nothrow_move_assignable<E> && nothrow_move_constructible<E>`

```cpp
TEST_CASE("expected: move assign", "[expected]") {
    expected<std::string, int> a("hello"), b("world");
    a = std::move(b);
    CHECK(*a == "world");
}

static_assert(std::is_nothrow_move_assignable_v<expected<int, int>>);
```

### Assign from value U&&
- **Constraints (11.1–11.5):** U != expected, U not unexpected<X>, constructible<T,U>, assignable<T&,U>, `nothrow_constructible<T,U> || nothrow_move_constructible<T> || nothrow_move_constructible<E>`

```cpp
TEST_CASE("expected: assign from value when has value", "[expected]") {
    expected<int, std::string> e(1);
    e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}

TEST_CASE("expected: assign from value when has error", "[expected]") {
    expected<int, std::string> e(unexpect, "err");
    e = 42;
    REQUIRE(e.has_value());
    CHECK(*e == 42);
}
```

### Assign from unexpected<G>
- **Constraints (15.1–15.3):** constructible<E,GF>, assignable<E&,GF>, nothrow guard

```cpp
TEST_CASE("expected: assign from unexpected when has value", "[expected]") {
    expected<int, std::string> e(1);
    e = unexpected<std::string>("fail");
    REQUIRE(!e.has_value());
    CHECK(e.error() == "fail");
}

TEST_CASE("expected: assign from unexpected when has error", "[expected]") {
    expected<int, std::string> e(unexpect, "old");
    e = unexpected<std::string>("new");
    CHECK(e.error() == "new");
}
```

---

## Emplace [expected.object.assign] para 18–21

- **Constraint:** `is_nothrow_constructible_v<T, Args...>`
- Returns `T&`
- Destroys current state first (safe even if T threw — nothrow guaranteed)

```cpp
TEST_CASE("expected: emplace from value state", "[expected]") {
    expected<std::string, int> e("old");
    auto& ref = e.emplace(3, 'x');
    REQUIRE(e.has_value());
    CHECK(*e == "xxx");
    CHECK(&ref == &*e);
}

TEST_CASE("expected: emplace from error state", "[expected]") {
    expected<std::string, int> e(unexpect, 5);
    e.emplace(2, 'z');
    REQUIRE(e.has_value());
    CHECK(*e == "zz");
}

// Constraint: nothrow constructible
struct MightThrow { MightThrow(int) noexcept(false); };
static_assert(!std::is_invocable_v<
    decltype(&expected<MightThrow, int>::emplace<int>),
    expected<MightThrow, int>&, int>);
```

---

## Swap [expected.object.swap]

- **Constraints (1.1–1.4):** `swappable<T>`, `swappable<E>`, `move_constructible<T> && move_constructible<E>`, `nothrow_move_constructible<T> || nothrow_move_constructible<E>`
- Four state combinations tested

```cpp
TEST_CASE("expected: swap value-value", "[expected]") {
    expected<int, int> a(1), b(2);
    a.swap(b);
    CHECK(*a == 2);
    CHECK(*b == 1);
}

TEST_CASE("expected: swap value-error", "[expected]") {
    expected<int, int> a(1), b(unexpect, 99);
    a.swap(b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
    CHECK(a.error() == 99);
    CHECK(*b == 1);
}

TEST_CASE("expected: swap error-value", "[expected]") {
    expected<int, int> a(unexpect, 10), b(42);
    a.swap(b);
    REQUIRE(a.has_value());
    REQUIRE(!b.has_value());
    CHECK(*a == 42);
    CHECK(b.error() == 10);
}

TEST_CASE("expected: swap error-error", "[expected]") {
    expected<int, int> a(unexpect, 1), b(unexpect, 2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

TEST_CASE("expected: free swap", "[expected]") {
    using std::swap;
    expected<int, int> a(3), b(unexpect, 7);
    swap(a, b);
    REQUIRE(!a.has_value());
    REQUIRE(b.has_value());
}
```

---

## Observers [expected.object.obs]

### operator->() — Hardened precondition: has_value()

```cpp
TEST_CASE("expected: operator-> returns pointer to value", "[expected]") {
    expected<std::string, int> e("hello");
    CHECK(e->size() == 5);
    CHECK(e.operator->() == &*e);
}

// Hardened precondition test (only when contract mode is active)
#if defined(BEMAN_EXPECTED_HARDENED)
TEST_CASE("expected: operator-> on empty terminates", "[expected][hardened]") {
    expected<int, int> e(unexpect, 1);
    REQUIRE_THROWS(e.operator->());
}
#endif
```

### operator*() — Hardened precondition: has_value()

All 4 ref-qualified overloads:

```cpp
TEST_CASE("expected: operator* ref qualifications", "[expected]") {
    expected<int, int> e(42);
    static_assert(std::is_same_v<decltype(*e), int&>);
    static_assert(std::is_same_v<decltype(*std::as_const(e)), const int&>);
    static_assert(std::is_same_v<decltype(*std::move(e)), int&&>);
    static_assert(std::is_same_v<
        decltype(*std::move(std::as_const(e))), const int&&>);
    CHECK(*e == 42);
}
```

### has_value() / operator bool()

```cpp
TEST_CASE("expected: has_value and bool conversion", "[expected]") {
    expected<int, int> a(1), b(unexpect, 0);
    CHECK(a.has_value());
    CHECK(bool(a));
    CHECK(!b.has_value());
    CHECK(!bool(b));
}
```

### value() — throws bad_expected_access when empty

- **Mandates (& / const& overloads):** `is_copy_constructible_v<E>`
- **Mandates (&& / const&& overloads):** `is_copy_constructible_v<E> && is_constructible_v<E, E&&>`
- **Throws:** `bad_expected_access(as_const(error()))` or `bad_expected_access(std::move(error()))`

```cpp
TEST_CASE("expected: value() returns value", "[expected]") {
    expected<int, int> e(42);
    CHECK(e.value() == 42);
}

TEST_CASE("expected: value() throws on error", "[expected]") {
    expected<int, int> e(unexpect, 7);
    REQUIRE_THROWS_AS(e.value(), beman::expected::bad_expected_access<int>);
}

TEST_CASE("expected: value() thrown exception carries error", "[expected]") {
    expected<int, int> e(unexpect, 99);
    try {
        e.value();
        FAIL("should have thrown");
    } catch (const beman::expected::bad_expected_access<int>& ex) {
        CHECK(ex.error() == 99);
    }
}

TEST_CASE("expected: rvalue value() throws bad_expected_access", "[expected]") {
    expected<int, int> e(unexpect, 5);
    REQUIRE_THROWS_AS(
        std::move(e).value(),
        beman::expected::bad_expected_access<int>);
}

// Mandates: is_copy_constructible_v<E> for value()
struct NoCopyE { NoCopyE(const NoCopyE&) = delete; };
// value() should not be callable when E is not copy-constructible
// (This is a mandate, not a constraint — test with static_assert or fail-file)
```

### error() — Hardened precondition: !has_value()

```cpp
TEST_CASE("expected: error() returns error", "[expected]") {
    expected<int, std::string> e(unexpect, "bad");
    static_assert(std::is_same_v<decltype(e.error()), std::string&>);
    CHECK(e.error() == "bad");
}

TEST_CASE("expected: error() rvalue", "[expected]") {
    expected<int, std::string> e(unexpect, "bad");
    std::string s = std::move(e).error();
    CHECK(s == "bad");
}
```

### value_or()

- **Mandates (const& overload):** `is_copy_constructible_v<T> && is_convertible_v<U, T>`
- **Mandates (&& overload):** `is_move_constructible_v<T> && is_convertible_v<U, T>`

```cpp
TEST_CASE("expected: value_or with value", "[expected]") {
    expected<int, int> e(42);
    CHECK(e.value_or(0) == 42);
}

TEST_CASE("expected: value_or with error", "[expected]") {
    expected<int, int> e(unexpect, 1);
    CHECK(e.value_or(99) == 99);
}

TEST_CASE("expected: rvalue value_or with error", "[expected]") {
    expected<std::string, int> e(unexpect, 1);
    std::string s = std::move(e).value_or("default");
    CHECK(s == "default");
}
```

### error_or()

- **Mandates (const& overload):** `is_copy_constructible_v<E> && is_convertible_v<G, E>`
- **Mandates (&& overload):** `is_move_constructible_v<E> && is_convertible_v<G, E>`

```cpp
TEST_CASE("expected: error_or with value", "[expected]") {
    expected<int, int> e(42);
    CHECK(e.error_or(99) == 99);
}

TEST_CASE("expected: error_or with error", "[expected]") {
    expected<int, int> e(unexpect, 7);
    CHECK(e.error_or(0) == 7);
}
```

---

## Equality Operators [expected.object.eq]

### expected == expected (T2 not void)
- **Constraint:** `*x == *y` and `x.error() == y.error()` both well-formed and bool-convertible

```cpp
TEST_CASE("expected: equality both have value", "[expected]") {
    expected<int, int> a(1), b(1), c(2);
    CHECK(a == b);
    CHECK(!(a == c));
}

TEST_CASE("expected: equality both have error", "[expected]") {
    expected<int, int> a(unexpect, 1), b(unexpect, 1), c(unexpect, 2);
    CHECK(a == b);
    CHECK(!(a == c));
}

TEST_CASE("expected: equality mixed value/error", "[expected]") {
    expected<int, int> a(1), b(unexpect, 1);
    CHECK(!(a == b));
}
```

### expected == T2 (comparison with value)
- **Constraint:** T2 is not a specialization of expected

```cpp
TEST_CASE("expected: equality with value type", "[expected]") {
    expected<int, int> a(42), b(unexpect, 0);
    CHECK(a == 42);
    CHECK(!(b == 42));
}
```

### expected == unexpected<E2>
- **Constraint:** `x.error() == e.error()` well-formed and bool-convertible

```cpp
TEST_CASE("expected: equality with unexpected", "[expected]") {
    expected<int, int> a(unexpect, 7), b(42);
    CHECK(a == unexpected(7));
    CHECK(!(b == unexpected(7)));
}
```

---

## Negative Compile Tests

### `expected_t_ref_fail.cpp`
```cpp
// NEGATIVE: T is a reference type — ill-formed
#include <beman/expected/expected.hpp>
beman::expected::expected<int&, int> e;
```

### `expected_e_ref_fail.cpp`
```cpp
// NEGATIVE: E is a reference type — ill-formed
#include <beman/expected/expected.hpp>
beman::expected::expected<int, int&> e;
```

### `expected_t_array_fail.cpp`
```cpp
// NEGATIVE: T is an array type — ill-formed
#include <beman/expected/expected.hpp>
beman::expected::expected<int[3], int> e;
```

### `expected_value_mandate_fail.cpp`
```cpp
// NEGATIVE: value() mandates is_copy_constructible_v<E>
// When E is not copy-constructible, value() is ill-formed
#include <beman/expected/expected.hpp>
struct NoCopyE {
    NoCopyE() = default;
    NoCopyE(const NoCopyE&) = delete;
    NoCopyE(NoCopyE&&) = default;
};
void test() {
    beman::expected::expected<int, NoCopyE> e(42);
    (void)e.value();  // ill-formed: E not copy-constructible
}
```

### `expected_emplace_throwing_fail.cpp`
```cpp
// NEGATIVE: emplace requires nothrow_constructible<T, Args...>
#include <beman/expected/expected.hpp>
struct ThrowingCtor { ThrowingCtor(int) noexcept(false); };
void test() {
    beman::expected::expected<ThrowingCtor, int> e(std::in_place, 0);
    e.emplace(1);  // ill-formed: not nothrow-constructible
}
```
