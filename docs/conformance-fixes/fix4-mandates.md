# Fix 4: Mandates static_asserts

**Branch:** `fix4-mandates`
**Depends on:** Fix 3 (monadic operations should have their requires clauses first)
**Read first:** `docs/conformance-audit.md` (sections 4.2, 4.7, 4.8, 5.6, 5.7)

---

## Goal

Add `static_assert` checks for all standard *Mandates* that are currently
missing. Mandates are unconditional compile-time requirements — they
produce hard errors with clear diagnostics, not SFINAE.

These are audit items 17-22.

## Changes

All changes are in `include/beman/expected/expected.hpp`.

### 1. Primary template static_assert: T not a specialization of unexpected

Add to the existing block of static_asserts at the top of the class:

```cpp
static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<T>>::value,
              "T must not be a specialization of unexpected");
```

### 2. value() Mandates — primary template

**`value() const&` and `value() &`** — add inside the function body:
```cpp
static_assert(std::is_copy_constructible_v<E>,
              "value() requires is_copy_constructible_v<E>");
```

**`value() &&`** — add:
```cpp
static_assert(std::is_copy_constructible_v<E> &&
              std::is_constructible_v<E, decltype(std::move(error()))>,
              "value() && requires E be copy-constructible and constructible from move(error())");
```

**`value() const&&`** — same as `&&` variant.

### 3. value() Mandates — void specialization

**`value() const&`** — add:
```cpp
static_assert(std::is_copy_constructible_v<E>,
              "value() requires is_copy_constructible_v<E>");
```

**`value() &&`** — add:
```cpp
static_assert(std::is_copy_constructible_v<E> && std::is_move_constructible_v<E>,
              "value() && requires E be copy-constructible and move-constructible");
```

### 4. value_or() Mandates — primary template

**`value_or(U&&) const&`:**
```cpp
static_assert(std::is_copy_constructible_v<T>, "value_or requires is_copy_constructible_v<T>");
static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
```

**`value_or(U&&) &&`:**
```cpp
static_assert(std::is_move_constructible_v<T>, "value_or requires is_move_constructible_v<T>");
static_assert(std::is_convertible_v<U, T>, "value_or requires is_convertible_v<U, T>");
```

### 5. error_or() Mandates — both primary and void

**`error_or(G&&) const&`:**
```cpp
static_assert(std::is_copy_constructible_v<E>, "error_or requires is_copy_constructible_v<E>");
static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
```

**`error_or(G&&) &&`:**
```cpp
static_assert(std::is_move_constructible_v<E>, "error_or requires is_move_constructible_v<E>");
static_assert(std::is_convertible_v<G, E>, "error_or requires is_convertible_v<G, E>");
```

### 6. transform() Mandates — both primary and void

In each transform overload, after computing `U`, add:

```cpp
if constexpr (!std::is_void_v<U>) {
    static_assert(!std::is_array_v<U>, "transform: U must not be an array type");
    static_assert(!std::is_same_v<std::remove_cv_t<U>, std::in_place_t>,
                  "transform: U must not be in_place_t");
    static_assert(!std::is_same_v<std::remove_cv_t<U>, unexpect_t>,
                  "transform: U must not be unexpect_t");
    static_assert(!detail::is_unexpected_specialization<std::remove_cv_t<U>>::value,
                  "transform: U must not be a specialization of unexpected");
}
```

The "well-formed declaration" Mandates (`U u(invoke(...))`) will naturally
produce a diagnostic if violated, so an explicit static_assert is optional.

### 7. transform_error() Mandates — both primary and void

After computing `G`, add:

```cpp
static_assert(std::is_object_v<G>, "transform_error: G must be an object type");
static_assert(!std::is_array_v<G>, "transform_error: G must not be an array type");
static_assert(std::is_same_v<G, std::remove_cv_t<G>>,
              "transform_error: G must not be cv-qualified");
static_assert(!detail::is_unexpected_specialization<G>::value,
              "transform_error: G must not be a specialization of unexpected");
```

## Tests

Add to a new file `tests/beman/expected/expected_mandates.test.cpp`
(beman-only target):

- Verify `static_assert` fires for `expected<unexpected<int>, int>` (T is
  unexpected specialization) — this is a negative compile test
  (`expected_unexpected_value_type_fail.cpp`)

Existing tests already exercise value_or/error_or/transform/transform_error
with valid types, so they verify the static_asserts don't fire for good inputs.

## Verification

```bash
make TOOLCHAIN=gcc-16 test
make lint
```

## Handoff

Merge (--no-ff) into `expected-over-references`.
