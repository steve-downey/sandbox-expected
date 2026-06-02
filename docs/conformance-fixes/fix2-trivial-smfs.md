# Fix 2: Trivial Special Member Functions

**Branch:** `fix2-trivial-smfs`
**Depends on:** —
**Read first:** `docs/conformance-audit.md` (sections 4.3, 4.5, 5.4)

---

## Goal

Add trivial paths for copy/move constructors and copy/move assignment
operators on the primary template. The void specialization already has
trivial copy/move constructors but needs trivial assignment operators.

These are audit items 8-12.

## Background

The standard requires these operations to be trivial when T and E (or
just E for void) are trivially copy/move constructible/assignable/destructible.
The current implementation always uses `construct_at`/`destroy_at`, which
prevents the compiler from recognizing triviality.

The pattern is: provide a `= default` overload guarded by a triviality
requires clause, and a non-trivial overload guarded by the complement.
The void specialization already demonstrates this for constructors:

```cpp
constexpr expected(const expected&)
    requires std::is_trivially_copy_constructible_v<E>
= default;

constexpr expected(const expected& rhs)
    requires(std::is_copy_constructible_v<E> && !std::is_trivially_copy_constructible_v<E>);
```

## Changes

### Primary template: trivial copy constructor

```cpp
constexpr expected(const expected&)
    requires(std::is_trivially_copy_constructible_v<T> &&
             std::is_trivially_copy_constructible_v<E>)
= default;

constexpr expected(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
             !(std::is_trivially_copy_constructible_v<T> &&
               std::is_trivially_copy_constructible_v<E>));
```

### Primary template: trivial move constructor

Same pattern with `move` variants.

### Primary template: trivial copy assignment

```cpp
constexpr expected& operator=(const expected&)
    requires(std::is_trivially_copy_constructible_v<T> &&
             std::is_trivially_copy_assignable_v<T> &&
             std::is_trivially_destructible_v<T> &&
             std::is_trivially_copy_constructible_v<E> &&
             std::is_trivially_copy_assignable_v<E> &&
             std::is_trivially_destructible_v<E>)
= default;
```

The non-trivial overload keeps all the existing constraints from
[expected.object.assign] para 4.

### Primary template: trivial move assignment

Same pattern with `move` variants plus the para 10 conditions.

### Void specialization: trivial copy assignment

```cpp
constexpr expected& operator=(const expected&)
    requires(std::is_trivially_copy_constructible_v<E> &&
             std::is_trivially_copy_assignable_v<E> &&
             std::is_trivially_destructible_v<E>)
= default;
```

### Void specialization: trivial move assignment

Same pattern.

## Tests

Add to a new file `tests/beman/expected/expected_trivial.test.cpp`
(beman-only — triviality is implementation quality, libstdc++ may differ):

```cpp
// Primary template
static_assert(std::is_trivially_copy_constructible_v<expt::expected<int, int>>);
static_assert(std::is_trivially_move_constructible_v<expt::expected<int, int>>);
static_assert(std::is_trivially_copy_assignable_v<expt::expected<int, int>>);
static_assert(std::is_trivially_move_assignable_v<expt::expected<int, int>>);
static_assert(std::is_trivially_destructible_v<expt::expected<int, int>>);

// Void specialization
static_assert(std::is_trivially_copy_constructible_v<expt::expected<void, int>>);
static_assert(std::is_trivially_move_constructible_v<expt::expected<void, int>>);
static_assert(std::is_trivially_copy_assignable_v<expt::expected<void, int>>);
static_assert(std::is_trivially_move_assignable_v<expt::expected<void, int>>);
static_assert(std::is_trivially_destructible_v<expt::expected<void, int>>);

// Non-trivial when type is non-trivial
static_assert(!std::is_trivially_copy_constructible_v<expt::expected<std::string, int>>);
```

All existing tests must still pass.

## Verification

```bash
make TOOLCHAIN=gcc-16 test
make lint
```

## Handoff

Merge (--no-ff) into `expected-over-references`.
