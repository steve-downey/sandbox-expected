# Handoff: After Fix 1

## What Was Done

Fix 1 is complete. Critical constructor/assignment/equality constraint bugs in the
primary template (`expected<T, E>`) are fixed on branch `fix1-constraints`, then
merged (--no-ff) into `expected-over-references`.

### Changes in `include/beman/expected/expected.hpp`

1. **`detail::converts_from_any_cvref<T, W>`** — new variable template in the
   second `detail` namespace block. Checks all 8 constructibility/convertibility
   combinations of T from W (const&, &, const&&, &&).

2. **Converting copy/move ctors** — replaced the old 8-line block
   (`!is_constructible_v<T, expected<U,G>&> && ...`) with:
   `(is_same_v<bool, remove_cv_t<T>> || !converts_from_any_cvref<T, expected<U,G>>)`
   — constraint 18.3. When T=bool the first operand is true, skipping the check;
   for non-bool T the check applies. Both declaration and out-of-line definition
   updated for both copy and move ctors.

3. **Value ctor** — added two new constraints:
   - `!is_unexpected_specialization<remove_cvref_t<U>>::value` — constraint 23.4
   - `(!is_same_v<bool, remove_cv_t<T>> || !is_expected_specialization<remove_cvref_t<U>>::value)` — constraint 23.6

4. **Move assignment** — added `(is_nothrow_move_constructible_v<T> || is_nothrow_move_constructible_v<E>)`
   to the requires clause — constraint 6.5.

5. **Value assignment** — two fixes:
   - Default template parameter changed from `U = T` to `U = remove_cv_t<T>`
   - Constraint 11.2: replaced `!is_same_v<remove_cvref_t<U>, unexpect_t>` with
     `!is_unexpected_specialization<remove_cvref_t<U>>::value`

6. **`operator==(expected, T2)`** — added
   `requires(!is_expected_specialization<T2>::value)` to the hidden friend.

### Tests added

- `tests/beman/expected/expected_constraints.test.cpp` — 7 Catch2 test cases
  (positive) + 8 static_assert checks:
  - bool exemption value/error paths
  - unexpected<G> routing (simple + AcceptsUnexpected type)
  - value assignment unexpected routing
  - move assignment deleted/available permutations
  - equality operator overload resolution
- `tests/beman/expected/expected_bool_value_ctor_from_expected_fail.cpp` — negative
  compile test: `expected<bool, std::string>` from `expected<int, int>` must fail
  (value ctor blocked by 23.6, converting ctor not viable due to error type mismatch)

### Test count

239 tests total, all passing (was 231 before Fix 1; 8 new tests including the
negative compile test).

### Spec document correction

The `fix1-constraints.md` spec had a sign error in the bool exemption expression.
The spec wrote `(!is_same_v<bool, T> || !converts_from_any_cvref<...>)` but the
correct form is `(is_same_v<bool, T> || !converts_from_any_cvref<...>)` — without
the leading `!`. The former blocks converting ctors for T=bool (the old bug);
the latter allows them (the fix).

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 239 tests, all passing
make lint                    # all linters (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktree `../fix1-constraints/` may be left or removed as desired
- All work accumulates on `expected-over-references`

## Conformance Fix Checklist

- [x] Fix 1: Constructor/assignment/equality constraints  ← just done
- [ ] Fix 2: Trivial special member functions
- [ ] Fix 3: Monadic operation constraints
- [ ] Fix 4: Mandates static_asserts
- [ ] Fix 5: Hardened preconditions and minor fixes

## Next Step: Fix 2 or Fix 3 (parallel)

Fix 2 and Fix 3 are independent. Both can be started from `expected-over-references`.

### Fix 2: Trivial special member functions (`fix2-trivial-smfs`)

Add `= default` copy/move constructor and copy/move assignment paths to the primary
template when all relevant members are trivially copyable/movable/destructible. This
is purely structural (ABI/codegen), not a correctness fix. See `fix2-trivial-smfs.md`.

Key pattern (from the void specialization which already does this correctly):
```cpp
constexpr expected(const expected&)
    requires std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>
= default;

constexpr expected(const expected& rhs)
    requires(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<E> &&
             !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<E>));
```

The non-trivial path's `requires` clause needs the additional `!(...trivially...)` guard
to prevent ambiguity. The primary template currently has no trivial path at all.

### Fix 3: Monadic operation constraints (`fix3-monadic-constraints`)

Add `requires` clauses to all 16 monadic operations in the primary template
(and_then, or_else, transform, transform_error) and the 8 in the void specialization
(and_then, transform). The standard specifies these as *Constraints* (SFINAE), not
hard errors. See `fix3-monadic-constraints.md`.

Pattern for primary template `and_then`:
```cpp
template <class F>
    requires std::is_constructible_v<E, E&>  // & overload
constexpr auto and_then(F&& f) &;
```
