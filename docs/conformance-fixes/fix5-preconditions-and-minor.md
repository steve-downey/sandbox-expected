# Fix 5: Hardened Preconditions and Minor Fixes

**Branch:** `fix5-preconditions-and-minor`
**Depends on:** —
**Read first:** `docs/conformance-audit.md` (sections 1.4, 4.7, 5.6, 5.7)

---

## Goal

Add hardened precondition checks to observers, fix the `unexpected` friend
swap constraint, and address the two minor deviations in the void
specialization's monadic operations.

These are audit items 23-28.

## Changes

### 1. Hardened preconditions (`include/beman/expected/expected.hpp`)

The standard specifies "Hardened preconditions" on several observers. Guard
these with the `BEMAN_EXPECTED_HARDENED` macro (matching the project's
existing convention from `docs/plan`).

For the primary template, add to each observer implementation:

```cpp
// operator->() — both const and non-const
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_) __builtin_trap();
#endif

// operator*() — all 4 overloads
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_) __builtin_trap();
#endif

// error() — all 4 overloads
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_) __builtin_trap();
#endif
```

For the void specialization:

```cpp
// operator*() const noexcept
#if defined(BEMAN_EXPECTED_HARDENED)
    if (!has_val_) __builtin_trap();
#endif

// error() — all 4 overloads
#if defined(BEMAN_EXPECTED_HARDENED)
    if (has_val_) __builtin_trap();
#endif
```

Use `__builtin_trap()` (available on GCC and Clang) for the trap. Do not
throw — these are precondition violations, not recoverable errors.

### 2. unexpected friend swap constraint (`include/beman/expected/unexpected.hpp`)

The standard says the friend swap has *Constraints* (SFINAE):
`is_swappable_v<E>` is true. Add a `requires` clause:

```cpp
friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)))
    requires std::is_swappable_v<E>
{ x.swap(y); }
```

### 3. Void specialization `or_else`: use `is_same_v` not `is_void_v`

In all 4 `or_else` overloads of the void specialization, change:

```cpp
static_assert(std::is_void_v<typename G::value_type>,
              "or_else: F must return expected with void value_type");
```

to:

```cpp
static_assert(std::is_same_v<typename G::value_type, T>,
              "or_else: F must return expected with the same value_type");
```

This matches the standard wording `is_same_v<typename G::value_type, T>`
and correctly handles `const void` and other cv-void types.

### 4. Void specialization `transform_error`: use `expected<T, G>` not `expected<void, G>`

In all 4 `transform_error` overloads of the void specialization, change
`expected<void, G>` to `expected<T, G>`:

```cpp
// Before:
return expected<void, G>();
return expected<void, G>(unexpect, ...);

// After:
return expected<T, G>();
return expected<T, G>(unexpect, ...);
```

## Tests

### Hardened precondition tests

Add a new test file `tests/beman/expected/expected_hardened.test.cpp`
(beman-only target). These tests must be compiled with
`-DBEMAN_EXPECTED_HARDENED`:

- `operator*()` on error-state expected traps (use death test or signal check)
- `operator->()` on error-state expected traps
- `error()` on value-state expected traps
- Same for void specialization `operator*()` and `error()`

If Catch2 doesn't support death tests easily, instead just verify the
precondition checks compile correctly and the happy paths work. The
important thing is the code compiles with `BEMAN_EXPECTED_HARDENED` defined.

Add a CMake target that compiles the test suite with
`-DBEMAN_EXPECTED_HARDENED` to ensure the precondition code is at least
compiled.

### unexpected swap constraint test

Add a `static_assert` (beman-only) verifying that `swap(x, y)` is not
well-formed for non-swappable E via a requires-expression check:

```cpp
struct NoSwap { NoSwap(NoSwap&&) = delete; };
static_assert(!requires(expt::unexpected<NoSwap>& a, expt::unexpected<NoSwap>& b) {
    swap(a, b);
});
```

### or_else / transform_error minor fix tests

The existing void monadic tests already exercise these paths. No new tests
required unless you want to explicitly test `expected<const void, E>` —
which is an extremely unlikely type but technically valid.

## Verification

```bash
make TOOLCHAIN=gcc-16 test
make lint
```

## Handoff

Merge (--no-ff) into `expected-over-references`.
