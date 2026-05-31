# Handoff: After Fix 3

## What Was Done

Fix 3 is complete. SFINAE-friendly `requires` clauses added to all monadic
operations per standard Constraints. Branch `fix3-monadic-constraints` merged
(--no-ff) into `expected-over-references`.

### Changes in `include/beman/expected/expected.hpp`

**Primary template (16 overloads — 4 operations × 4 ref-qualifiers):**
- `and_then` / `transform`: `requires std::is_constructible_v<E, E-ref>`
  (E& for &, E&& for &&, const E& for const&, const E&& for const&&)
- `or_else` / `transform_error`: `requires std::is_constructible_v<T, T-ref>`
  (same ref-qualifier pattern)

**Void specialization (8 of 16 overloads):**
- `and_then` / `transform`: same E-constructibility constraints as primary
- `or_else` / `transform_error`: NO constraints (standard specifies none)

Both in-class declarations and out-of-line definitions updated.

### Tests added

- `tests/beman/expected/expected_monadic_constraints.test.cpp` — beman-only:
  - Concept detectors using `std::declval<X>()` to preserve value category
  - MoveOnly type (deleted copy ctor) as E: verifies `and_then`/`transform`
    lvalue overloads are SFINAE-removed, rvalue overloads remain available
  - MoveOnly type as T: verifies `or_else`/`transform_error` same pattern
  - Void specialization: same E-constructibility tests for `and_then`/`transform`
  - Void specialization: `or_else`/`transform_error` unconstrained (always available)
  - Normal types: all operations remain available on all overloads
  - 6 Catch2 runtime tests exercising rvalue monadic chains with move-only types

### Test count

247 tests total, all passing.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 247 tests, all passing
make lint                    # all linters pass (beman-tidy crash is pre-existing)
```

## Conformance Fix Checklist

- [x] Fix 1: Constructor/assignment/equality constraints
- [x] Fix 2: Trivial special member functions
- [x] Fix 3: Monadic operation constraints  ← just done
- [ ] Fix 4: Mandates static_asserts
- [ ] Fix 5: Hardened preconditions and minor fixes

## Next Step: Fix 4

Fix 4 adds `static_assert` Mandates to observers and monadic operations.
These go on the same functions that just received requires clauses (Fix 3),
plus `value()`, `value_or()`, and `error_or()` observers.

The Mandates are compile-time checks that produce clear diagnostics when
violated, as opposed to Constraints which affect overload resolution.
See `docs/conformance-fixes/fix4-mandates.md`.
