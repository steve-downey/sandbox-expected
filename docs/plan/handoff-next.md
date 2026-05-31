# Handoff: After Fix 4

## What Was Done

Fix 4 is complete. `static_assert` Mandates added to observers and monadic
operations per standard wording. Branch `fix4-mandates` merged (--no-ff) into
`expected-over-references`.

### Changes in `include/beman/expected/expected.hpp`

**Primary template class-level:**
- `static_assert(!detail::is_unexpected_specialization<remove_cv_t<T>>::value, ...)`

**Primary template `value()` (4 overloads):**
- `const&` / `&`: `is_copy_constructible_v<E>`
- `&&` / `const&&`: `is_copy_constructible_v<E> && is_constructible_v<E, decltype(std::move(error()))>`

**Primary template `value_or()` (2 overloads):**
- `const&`: `is_copy_constructible_v<T>`, `is_convertible_v<U, T>`
- `&&`: `is_move_constructible_v<T>`, `is_convertible_v<U, T>`

**`error_or()` (4 overloads — both primary and void):**
- `const&`: `is_copy_constructible_v<E>`, `is_convertible_v<G, E>`
- `&&`: `is_move_constructible_v<E>`, `is_convertible_v<G, E>`

**Void specialization `value()` (2 overloads):**
- `const&`: `is_copy_constructible_v<E>`
- `&&`: `is_copy_constructible_v<E> && is_move_constructible_v<E>`

**`transform()` (8 overloads — both primary and void):**
- In `if constexpr (!is_void_v<U>)` block: U not array, not in_place_t, not unexpect_t, not unexpected<>

**`transform_error()` (8 overloads — both primary and void):**
- G is object, not array, not cv-qualified, not unexpected<>

### Tests added

- `tests/beman/expected/expected_unexpected_value_type_fail.cpp` — negative compile
  test verifying `expected<unexpected<int>, int>` is ill-formed

### Test count

248 tests total, all passing.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 248 tests, all passing
make lint                    # all linters pass (beman-tidy crash is pre-existing)
```

## Conformance Fix Checklist

- [x] Fix 1: Constructor/assignment/equality constraints
- [x] Fix 2: Trivial special member functions
- [x] Fix 3: Monadic operation constraints
- [x] Fix 4: Mandates static_asserts  ← just done
- [ ] Fix 5: Hardened preconditions and minor fixes

## Next Step: Fix 5

Fix 5 adds hardened precondition checks to observers (`operator->`, `operator*`,
`error()`) and miscellaneous minor fixes (friend swap constraint on
`unexpected<E>`, etc.).

See `docs/conformance-fixes/fix5-preconditions-and-minor.md`.
