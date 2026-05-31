# Handoff: After Fix 2

## What Was Done

Fix 2 is complete. Trivial special member functions were added to the primary
template `expected<T, E>` and the void specialization `expected<void, E>`.
Branch `fix2-trivial-smfs` was merged (--no-ff) into `expected-over-references`.

### Changes in `include/beman/expected/expected.hpp`

1. **Primary template: trivial copy constructor** — new `= default` overload
   constrained by `is_trivially_copy_constructible_v<T> && is_trivially_copy_constructible_v<E>`.
   Non-trivial overload gains negated triviality guard.

2. **Primary template: trivial move constructor** — same pattern with move traits.

3. **Primary template: trivial copy assignment** — `= default` overload
   constrained by trivially copy constructible/assignable/destructible for both T and E.
   Non-trivial overload gains negated guard.

4. **Primary template: trivial move assignment** — same pattern with move traits.

5. **Void specialization: trivial copy assignment** — `= default` overload
   constrained by trivially copy constructible/assignable/destructible for E.
   Non-trivial overload gains negated guard.

6. **Void specialization: trivial move assignment** — same pattern.

The void specialization already had trivial copy/move constructors (from Step 4).

### Tests added

- `tests/beman/expected/expected_trivial.test.cpp` — beman-only:
  - `static_assert` checks for `expected<int, int>` (trivially copy/move constructible,
    copy/move assignable, destructible)
  - `static_assert` checks for `expected<void, int>` (same)
  - Negative `static_assert` checks for `expected<std::string, int>` (not trivial)
  - Two Catch2 runtime tests verifying copy/move construction and assignment

### Test count

241 tests total, all passing (was 241 before; 2 new runtime tests + static_asserts
at compile time).

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 241 tests, all passing
make lint                    # all linters pass (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktree `../fix2-trivial-smfs/` contains the fix branch

## Conformance Fix Checklist

- [x] Fix 1: Constructor/assignment/equality constraints
- [x] Fix 2: Trivial special member functions  ← just done
- [ ] Fix 3: Monadic operation constraints
- [ ] Fix 4: Mandates static_asserts
- [ ] Fix 5: Hardened preconditions and minor fixes

## Next Step: Fix 3

Fix 3 adds `requires` clauses to all monadic operations. The standard specifies
these as *Constraints* (SFINAE), but the implementation currently has no
constraints — calling a monadic operation when the constraint isn't met produces
a hard error instead of graceful overload failure.

### Primary template (16 overloads: 4 operations × 4 ref-qualifiers)

- `and_then`: needs `is_constructible_v<E, decltype(error())>` (& and const& overloads)
  or `is_constructible_v<E, decltype(std::move(error()))>` (&& and const&& overloads)
- `or_else`: needs `is_constructible_v<T, decltype(*this)>` variant per ref-qualifier
- `transform`: needs `is_constructible_v<E, decltype(error())>` variant
- `transform_error`: needs `is_constructible_v<T, decltype(*this)>` variant

### Void specialization (8 overloads: and_then + transform × 4)

- `and_then`: same E-constructibility constraints
- `transform`: same E-constructibility constraints
- `or_else` and `transform_error` have no Constraints in the standard for the void
  specialization (only Mandates)

See `docs/conformance-fixes/fix3-monadic-constraints.md` for full details.
