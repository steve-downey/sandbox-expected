# Fix 1: Constructor, Assignment, and Equality Constraints

**Branch:** `fix1-constraints`
**Depends on:** —
**Read first:** `docs/conformance-audit.md` (sections 4.3, 4.5, 4.9),
`docs/standard/expected.txt`

---

## Goal

Fix constraint bugs in the primary template that affect overload resolution
or reject valid programs. These are the audit's "Critical" items 1-7.

## Changes

All changes are in `include/beman/expected/expected.hpp`, primary template
only. The void specialization's corresponding constructors are already
correct or do not apply.

### 1. Add `converts_from_any_cvref` helper

Add to `namespace detail`:

```cpp
template <class T, class W>
constexpr bool converts_from_any_cvref =
    std::disjunction_v<
        std::is_constructible<T, W&>,       std::is_convertible<W&, T>,
        std::is_constructible<T, W>,        std::is_convertible<W, T>,
        std::is_constructible<T, const W&>, std::is_convertible<const W&, T>,
        std::is_constructible<T, const W>,  std::is_convertible<const W, T>>;
```

### 2. Fix converting constructors from expected<U, G>

In the `requires` clause of both `expected(const expected<U, G>&)` and
`expected(expected<U, G>&&)`, replace the 8-line
`!is_constructible/!is_convertible` block with:

```
(!std::is_same_v<bool, std::remove_cv_t<T>> ||
 !detail::converts_from_any_cvref<T, expected<U, G>>)
```

This is constraint (18.3): "if T is not cv bool, converts-from-any-cvref
is false". When T IS cv bool, the constraint is skipped (the `||` makes
the whole sub-expression true).

Keep the four `!is_constructible_v<unexpected<E>, ...>` lines unchanged
(constraints 18.4-18.7).

### 3. Fix value constructor `expected(U&&)` constraints

Add these two constraints to the `requires` clause:

```
&& !detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value  // (23.4)
&& (!std::is_same_v<bool, std::remove_cv_t<T>> ||                       // (23.6)
    !detail::is_expected_specialization<std::remove_cvref_t<U>>::value)
```

### 4. Fix value assignment `operator=(U&&)`

a) Change default template argument from `U = T` to
   `U = std::remove_cv_t<T>`.

b) Replace `!std::is_same_v<std::remove_cvref_t<U>, unexpect_t>` with
   `!detail::is_unexpected_specialization<std::remove_cvref_t<U>>::value`.

### 5. Fix move assignment constraint

Add to the `requires` clause:

```
&& (std::is_nothrow_move_constructible_v<T> || std::is_nothrow_move_constructible_v<E>)
```

This is constraint (6.5).

### 6. Fix `operator==(const expected&, const T2&)`

Add constraints to the hidden friend:

```cpp
template <class T2>
    requires(!detail::is_expected_specialization<T2>::value)
friend constexpr bool operator==(const expected& x, const T2& val) {
    ...
}
```

The standard also says "`*x == v` is well-formed and its result is
convertible to bool" is a Constraint. This requires a more elaborate
requires-expression. At minimum, add the specialization guard.

## Tests

### New tests (beman-only target — these test constraint behavior)

Add to a new file `tests/beman/expected/expected_constraints.test.cpp`:

- `expected<bool, int>` can be constructed from `expected<int, int>` (the
  bool exemption works — converting ctor selected, not value ctor)
- `expected<int, int>` cannot be implicitly constructed from
  `unexpected<int>` via the value constructor (unexpected specialization
  guard)
- Value assignment from `unexpected<int>` goes through the unexpected
  overload, not the value overload
- Move assignment is deleted when neither T nor E is nothrow move
  constructible (use `static_assert(!is_move_assignable_v<...>)`)
- `expected<int, int> == expected<int, int>` uses the expected-expected
  overload, not the value overload

### Negative compile tests

- `expected_bool_value_ctor_from_expected_fail.cpp` — when T=bool,
  `expected<bool, E>(expected<U, G>{})` must use the converting ctor,
  not the value ctor

### Existing tests

Run the full suite. No existing test should break.

## Verification

```bash
make TOOLCHAIN=gcc-16 test
make lint
```

## Handoff

Merge (--no-ff) into `expected-over-references`.
