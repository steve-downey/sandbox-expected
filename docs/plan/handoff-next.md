# Handoff: After Step 5

## What Was Done

Step 5 is complete. Monadic operations for `expected<T, E>` primary template
are implemented and tested on branch `step5-expected-primary-monadic`, then
merged (--no-ff) into `expected-over-references`.

### Files changed

- `include/beman/expected/expected.hpp`
  - Added `#include <functional>` for `std::invoke`
  - Added `detail::is_expected_specialization<T>` trait (primary false_type +
    specialization for `expected<T,E>` : true_type) in the detail namespace
  - Added 16 monadic method declarations inside the primary template class body:
    `and_then`, `or_else`, `transform`, `transform_error`, each with 4
    ref-qualified overloads (`&`, `&&`, `const&`, `const&&`)
  - Implemented all 16 out-of-line after the `error_or` implementations, before
    the void specialization
  - Mandates (callable return type requirements) enforced via `static_assert`,
    not SFINAE — these are "ill-formed, no diagnostic required" per the standard

- `tests/beman/expected/expected_monadic.test.cpp` — 27 new Catch2 test cases:
  - All four monadic operations, each with all 4 ref-qualified overloads
  - Value-passes and error-propagation paths
  - `transform` with void return type (returns `expected<void, E>`)
  - Type changes (transform to different type)
  - Chaining combinations (and_then → transform, or_else recovery, etc.)

- Negative compile test files (4 new):
  - `and_then_wrong_error_type_fail.cpp` — F returns wrong E type
  - `and_then_not_expected_fail.cpp` — F returns non-expected type
  - `or_else_wrong_value_type_fail.cpp` — F returns wrong T type
  - `transform_error_ref_result_fail.cpp` — F returns reference (triggers
    expected<T,int&> static_assert)

- `tests/beman/expected/CMakeLists.txt` — added `expected_monadic.test.cpp` to
  the main test executable; registered all 4 negative compile tests

### Implementation notes

- `transform` with void-returning F: the `if constexpr (std::is_void_v<U>)`
  branch calls `invoke(f, val_)` and then returns `expected<void, E>()` (or
  `expected<void, E>(unexpect, ...)` on error). The invoke call must appear
  before the return branch check to guarantee evaluation order.
- `is_expected_specialization` is declared as primary false_type in the detail
  namespace, then specialized after the `expected<T,E>` forward declaration. The
  partial specialization must come after the forward declaration but before any
  use (the monadic implementations come later, so this is satisfied).
- All 16 monadic operations access private `val_` and `unex_` directly (they
  are member functions of expected<T,E>).

### Test count

- 212 tests total, all passing (was 175 before this step; 37 new tests including
  the 4 negative compile tests)

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError`. Pre-existing on `main` and
unrelated to our changes. All other linters pass (clang-format was applied by
the pre-commit hook and its changes are included in the commit).

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 212 tests, all passing
make lint                    # all linters (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktree `../step5-expected-primary-monadic/` may be deleted:
  `git worktree remove ../step5-expected-primary-monadic`
- All work accumulates on `expected-over-references`; this branch will be
  merged to `main` when all steps complete

## Step Checklist

- [x] Step 1: `unexpected<E>`
- [x] Step 2: `bad_expected_access<E>`
- [x] Step 3: `expected<T, E>` primary template
- [x] Step 4: `expected<void, E>` specialization
- [x] Step 5: `expected<T, E>` monadic ops  ← just done
- [ ] Step 6: `expected<void, E>` monadic ops
- [ ] Step 7: `expected<T&, E>` reference specialization
- [ ] Step 8: `expected<T, E&>` error-reference specialization
- [ ] Step 9: `expected<T&, E&>` both-reference specialization
- [ ] Step 10: `expected<void, E&>` void+error-ref specialization

## Next Step: Step 6

**Step 6**: Monadic operations for `expected<void, E>`:
Same four operations but adapted for void value semantics.
See `docs/plan/step6-expected-void-monadic.md` and `docs/plan/tests-step6.md`.

### Key differences from Step 5 (critical to get right)

The void specialization differs because T is void — there is no stored value
to pass to the callable:

| Operation | Primary `expected<T,E>` | Void `expected<void,E>` |
|-----------|-------------------------|-------------------------|
| `and_then` has value | `invoke(f, val_)` | `invoke(f)` — no arg |
| `or_else` has value | `G(in_place, val_)` | `G()` — no value to copy |
| `transform` has value | `invoke(f, val_)` | `invoke(f)` — no arg |
| `transform_error` has value | `expected<T,G>(in_place, val_)` | `expected<void,G>()` |

The `or_else` constraint on the void specialization is also different: no
`is_constructible_v<T, ...>` check is needed (T is void, nothing to construct).

### Where to add the code

The void specialization is the second class body in `expected.hpp`, after the
line ~760 (`// [expected.void] Partial specialization for void value type`).
Add the 16 monadic declarations inside the class body (after `error_or`),
and the out-of-line implementations after the void `error_or` implementations.
The void out-of-line functions require `requires std::is_void_v<T>` on each
template prefix, as all other void-specialization out-of-line functions do.

### Reuse from Step 5

The `detail::is_expected_specialization` trait already exists (added in Step 5).
The same static_assert pattern for Mandates applies. The ref-qualification
structure is identical to Step 5.

### Test file for Step 6

Create `tests/beman/expected/expected_void_monadic.test.cpp`.
The `tests-step6.md` file has a complete test outline and negative compile
test templates. Follow Catch2 + double-include pattern from existing tests.

### Negative compile tests for Step 6

- `void_and_then_wrong_error_type_fail.cpp`
- `void_or_else_wrong_value_type_fail.cpp`

(Templates in `tests-step6.md`)

### After step is done

Merge (--no-ff) the step branch into `expected-over-references`.
Update the checklist in `docs/plan/index.md`.
Write a new `docs/plan/handoff-next.md`.
