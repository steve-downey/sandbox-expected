# Test Plan: Step 1 — unexpected<E>

**Standard section:** [expected.unexpected] (22.8.3)
**Test file:** `tests/beman/expected/unexpected.test.cpp`
**Negative-compile files:** `tests/beman/expected/unexpected_*_fail.cpp`

---

## Testing Strategy

Use Catch2 (`catch2/catch_test_macros.hpp`). Include the header twice at the
top of every test file to check idempotence:

```cpp
#include <beman/expected/unexpected.hpp>
#include <beman/expected/unexpected.hpp>
```

**Constraint violations** (Constraints:/Mandates: wording) are ill-formed at
the point of instantiation. Test them two ways:

1. **`static_assert(!std::is_constructible_v<...>)`** inside the normal test
   file — proves the constructor does not participate in overload resolution.
2. **Separate `_fail.cpp` files** registered in CMakeLists as OBJECT targets
   with `EXCLUDE_FROM_ALL` + `add_test(WILL_FAIL TRUE)` — proves the
   restriction is enforced at instantiation, matching the transcode pattern.

**Mandates** differ from Constraints: the program is ill-formed, but the
compiler may or may not diagnose it. Use `static_assert` to confirm the
type-level invariant holds; write a fail-file only when the diagnostic is
reliable (e.g., an explicit `static_assert` in the implementation).

**Hardened preconditions** do not apply to `unexpected` (it has none).

---

## Testable Statements from the Standard

### [expected.un.general] para 2 — ill-formed instantiations

> A program that instantiates the definition of unexpected for a
> non-object type, an array type, a specialization of unexpected,
> or a cv-qualified type is ill-formed.

| Case | Test strategy |
|------|--------------|
| `unexpected<int&>` is ill-formed | `static_assert(!std::is_constructible_v<unexpected<int&>, int&>)` + fail-file |
| `unexpected<int[]>` is ill-formed | fail-file |
| `unexpected<unexpected<int>>` is ill-formed | fail-file |
| `unexpected<const int>` is ill-formed | `static_assert` + fail-file |
| `unexpected<void>` is ill-formed (non-object?) | `static_assert` |

### [expected.un.cons] — constructors

**Converting constructor** `template<class Err = E> constexpr explicit unexpected(Err&&)`:

Constraints (1.1–1.3):

| Constraint | Positive test | Negative test |
|-----------|--------------|--------------|
| `!is_same_v<remove_cvref_t<Err>, unexpected>` | `unexpected<int>(42)` compiles | `static_assert(!is_constructible_v<unexpected<int>, unexpected<int>>)` via this ctor (copy ctor should win) |
| `!is_same_v<remove_cvref_t<Err>, in_place_t>` | N/A (in_place ctors below) | `static_assert(!is_constructible_v<unexpected<int>, std::in_place_t>)` |
| `is_constructible_v<E, Err>` | `unexpected<std::string>("hello")` | `static_assert(!is_constructible_v<unexpected<int>, std::string>)` |

**In-place constructor** `unexpected(in_place_t, Args&&...)`:

Constraint: `is_constructible_v<E, Args...>`

| Case | Test |
|------|------|
| `unexpected<std::string>(std::in_place, "hello", 5)` | compiles and constructs |
| Args that don't construct E | `static_assert(!is_constructible_v<...>)` |

**In-place + initializer_list** `unexpected(in_place_t, initializer_list<U>, Args&&...)`:

Constraint: `is_constructible_v<E, initializer_list<U>&, Args...>`

| Case | Test |
|------|------|
| `unexpected<std::vector<int>>(std::in_place, {1,2,3})` | compiles |
| ilist element type incompatible | `static_assert(!is_constructible_v<...>)` |

### [expected.un.obs] — observers

All four `error()` overloads must return `unex`:

```cpp
// Postcondition: returns the stored error value
unexpected<int> u(42);
CHECK(u.error() == 42);
CHECK(std::as_const(u).error() == 42);
CHECK(std::move(u).error() == 42);
CHECK(std::move(std::as_const(u)).error() == 42);
```

Also verify reference category:
```cpp
static_assert(std::is_same_v<decltype(u.error()), int&>);
static_assert(std::is_same_v<decltype(std::as_const(u).error()), const int&>);
static_assert(std::is_same_v<decltype(std::move(u).error()), int&&>);
static_assert(std::is_same_v<decltype(std::move(std::as_const(u)).error()), const int&&>);
```

### [expected.un.swap] — swap

**Member `swap(unexpected&)`:**
- Mandates: `is_swappable_v<E>` is true → test with non-swappable E (fail-file or `static_assert`)
- Effects: swaps unex values

```cpp
unexpected<int> a(1), b(2);
a.swap(b);
CHECK(a.error() == 2);
CHECK(b.error() == 1);
```

**Free `swap(unexpected&, unexpected&)`:**
- Constraint: `is_swappable_v<E>` is true
- `static_assert(!std::is_swappable_v<unexpected<not_swappable>>)` where `not_swappable` has deleted `swap`

### [expected.un.eq] — equality

- Mandates: `x.error() == y.error()` is well-formed and bool-convertible

```cpp
unexpected<int> a(1), b(1), c(2);
CHECK(a == b);
CHECK(!(a == c));

// Cross-type comparison
unexpected<long> d(1L);
CHECK(a == d);
```

Verify `!=` works via synthesized operator:
```cpp
CHECK(a != c);
```

---

## Test Outline

### Normal (positive) tests

```cpp
TEST_CASE("unexpected: construct from value", "[unexpected]") {
    unexpected<int> u(42);
    CHECK(u.error() == 42);
    // has_value() does not apply to unexpected
}

TEST_CASE("unexpected: copy construct", "[unexpected]") {
    unexpected<int> a(7);
    unexpected<int> b = a;
    CHECK(b.error() == 7);
}

TEST_CASE("unexpected: move construct", "[unexpected]") {
    unexpected<std::string> a(std::in_place, "hello");
    unexpected<std::string> b = std::move(a);
    CHECK(b.error() == "hello");
}

TEST_CASE("unexpected: in_place construct", "[unexpected]") {
    unexpected<std::string> u(std::in_place, "world", 5);
    CHECK(u.error() == "world");
}

TEST_CASE("unexpected: in_place with initializer_list", "[unexpected]") {
    unexpected<std::vector<int>> u(std::in_place, {1, 2, 3});
    CHECK(u.error().size() == 3);
    CHECK(u.error()[0] == 1);
}

TEST_CASE("unexpected: error() ref-qualifications", "[unexpected]") {
    unexpected<int> u(42);
    static_assert(std::is_same_v<decltype(u.error()), int&>);
    static_assert(std::is_same_v<decltype(std::as_const(u).error()), const int&>);
    static_assert(std::is_same_v<decltype(std::move(u).error()), int&&>);
    static_assert(std::is_same_v<
        decltype(std::move(std::as_const(u)).error()), const int&&>);
    CHECK(u.error() == 42);
}

TEST_CASE("unexpected: swap member", "[unexpected]") {
    unexpected<int> a(1), b(2);
    a.swap(b);
    CHECK(a.error() == 2);
    CHECK(b.error() == 1);
}

TEST_CASE("unexpected: swap free function", "[unexpected]") {
    using std::swap;
    unexpected<int> a(3), b(4);
    swap(a, b);
    CHECK(a.error() == 4);
    CHECK(b.error() == 3);
}

TEST_CASE("unexpected: equality same type", "[unexpected]") {
    unexpected<int> a(1), b(1), c(2);
    CHECK(a == b);
    CHECK(!(a == c));
    CHECK(a != c);
}

TEST_CASE("unexpected: equality cross type", "[unexpected]") {
    unexpected<int>  a(42);
    unexpected<long> b(42L);
    CHECK(a == b);
}

TEST_CASE("unexpected: CTAD", "[unexpected]") {
    unexpected u(42);
    static_assert(std::is_same_v<decltype(u), unexpected<int>>);
    CHECK(u.error() == 42);
}
```

### Constraint / type-trait tests (in normal test file)

```cpp
// Constructible: basic conversion works
static_assert(std::is_constructible_v<unexpected<int>, int>);
static_assert(std::is_constructible_v<unexpected<std::string>, const char*>);

// Constraint 1.3: E not constructible from Err → not constructible
static_assert(!std::is_constructible_v<unexpected<int>, std::string>);

// Copy-constructible (via deleted converting ctor — uses copy ctor)
static_assert(std::is_copy_constructible_v<unexpected<int>>);
static_assert(std::is_move_constructible_v<unexpected<int>>);

// Swappable
static_assert(std::is_swappable_v<unexpected<int>>);
```

---

## Negative Compile Tests

Register each as an OBJECT library in CMakeLists with `EXCLUDE_FROM_ALL`,
then add a `WILL_FAIL` ctest. Pattern from transcode:

```cmake
add_library(beman.expected.tests.unexpected_void_fail OBJECT)
target_sources(... PRIVATE unexpected_void_fail.cpp)
target_link_libraries(... PRIVATE beman::expected)
set_target_properties(... PROPERTIES EXCLUDE_FROM_ALL true EXCLUDE_FROM_DEFAULT_BUILD true)
add_test(NAME unexpected_void_fail
    COMMAND ${CMAKE_COMMAND} --build ... --target ...)
set_tests_properties(unexpected_void_fail PROPERTIES WILL_FAIL TRUE)
```

### `unexpected_array_fail.cpp`
```cpp
// NEGATIVE COMPILE TEST: unexpected<int[]> is ill-formed [expected.un.general] para 2
#include <beman/expected/unexpected.hpp>
beman::expected::unexpected<int[]> u(std::in_place);
```

### `unexpected_cvref_fail.cpp`
```cpp
// NEGATIVE COMPILE TEST: unexpected<const int> is ill-formed [expected.un.general] para 2
#include <beman/expected/unexpected.hpp>
beman::expected::unexpected<const int> u(42);
```

### `unexpected_self_fail.cpp`
```cpp
// NEGATIVE COMPILE TEST: unexpected<unexpected<int>> is ill-formed
#include <beman/expected/unexpected.hpp>
using namespace beman::expected;
unexpected<unexpected<int>> u(unexpected<int>(42));
```

### `unexpected_swap_nonswappable_fail.cpp`
```cpp
// NEGATIVE COMPILE TEST: swap mandates is_swappable_v<E>
#include <beman/expected/unexpected.hpp>
struct no_swap {
    no_swap() = default;
    no_swap(const no_swap&) = default;
    no_swap& operator=(const no_swap&) = delete;
};
namespace std { template<> struct is_swappable<no_swap> : std::false_type {}; }
void test() {
    beman::expected::unexpected<no_swap> a, b;
    a.swap(b);  // must fail: Mandates is_swappable_v<E>
}
```

---

## CMakeLists additions

For the normal test file, add sources to the existing combined test target or
create a dedicated one:

```cmake
add_executable(beman.expected.tests.unexpected)
target_sources(beman.expected.tests.unexpected PRIVATE unexpected.test.cpp)
target_link_libraries(
    beman.expected.tests.unexpected
    PRIVATE beman::expected Catch2::Catch2WithMain
)
include(Catch)
catch_discover_tests(beman.expected.tests.unexpected)
```

For each `_fail.cpp`, follow the transcode pattern (see above).
