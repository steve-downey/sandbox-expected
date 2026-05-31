# Handoff: After Step 3 + Test Backfill

## What Was Done

Step 3 is complete. `expected<T, E>` primary template is fully implemented
and tested on branch `step3-expected-primary`, then merged (--no-ff) into
`expected-over-references`.

After Step 3, all missing tests from the test plan (`docs/plan/tests-step1.md`,
`tests-step2.md`, `tests-step3.md`) were backfilled on `expected-over-references`
directly. See **Additional Test Plan Documents** below.

### What step3 included (beyond step3 itself)

Steps 1 and 2 were NOT previously merged into `expected-over-references`.
They existed only on separate sibling branches (`step1-unexpected`,
`step2-bad-expected-access`). Step 3 merged them in with conflict resolution:
the test framework changed from GTest (steps 1/2) to Catch2 (feature branch),
so those test files were converted to Catch2 format during the merge.

### Files changed

- `include/beman/expected/unexpected.hpp` — full `unexpected<E>` implementation
  (from step1: constructors, error() observers, swap, equality, CTAD)

- `include/beman/expected/bad_expected_access.hpp` — full implementation
  (from step2: `bad_expected_access<void>` base + `bad_expected_access<E>` with
  four ref-qualified error() overloads)

- `include/beman/expected/expected.hpp` — replaced the empty namespace with
  the full `expected<T, E>` primary template:
  - `detail::reinit_expected` helper for exception-safe state transitions
  - All constructors (default, copy, move, converting from expected<U,G>,
    from U&&, from unexpected<G>, in-place for value/error with initializer_list)
  - Destructor (trivial when T+E both trivially destructible)
  - Copy/move assignment, assign from U&&, assign from unexpected<G>
  - `emplace()` (two overloads: Args... and initializer_list + Args...)
  - `swap()` with all four state combinations, exception-safe
  - All observers: operator->, operator* (4 ref-qual), has_value(), operator
    bool(), value() (4 ref-qual, throws bad_expected_access<E>), error()
    (4 ref-qual), value_or(), error_or()
  - Hidden-friend equality operators (expected==expected, expected==value,
    expected==unexpected)
  - static_assert(!is_reference_v<T>) and static_assert(!is_reference_v<E>)
    (placeholder for Steps 7/8 which will lift these)

- `CMakeLists.txt` — added FetchContent_GetProperties fix for Catch2's
  `include(Catch)` / `catch_discover_tests`: when Catch2 is fetched via the
  lockfile FetchContent provider, its `extras/` dir was not on CMAKE_MODULE_PATH.
  Fixed by calling `FetchContent_GetProperties(catch2)` after `find_package`
  and appending to CMAKE_MODULE_PATH.

- `tests/beman/expected/unexpected.test.cpp` — full Catch2 tests (from step1,
  converted from GTest): 17 test cases covering all constructors, error()
  observers, swap, equality, CTAD, constexpr

- `tests/beman/expected/bad_expected_access.test.cpp` — full Catch2 tests
  (from step2, converted from GTest): 11 test cases

- `tests/beman/expected/expected.test.cpp` — comprehensive Catch2 tests:
  117 total test cases covering all of the above

### Test backfill (committed on expected-over-references after Step 3)

- `include/beman/expected/unexpected.hpp` — added static_asserts inside `unexpected<E>`
  for [expected.un.general] para 2 ill-formed instantiation constraints:
  `is_object_v<E>`, `!is_array_v<E>`, `is_same_v<E, remove_cv_t<E>>`,
  `!is_unexpected_specialization<E>` (uses a `detail::is_unexpected_specialization`
  trait forward-declared before the class). These make the negative compile tests work.

- `tests/beman/expected/unexpected.test.cpp` — added: constructibility static_asserts,
  `error()` ref-qualification static_asserts, `!=` operator test, ilist constraint check.

- `tests/beman/expected/bad_expected_access.test.cpp` — added: inheritance chain
  static_asserts (`exception → bad_expected_access<void> → bad_expected_access<E>`),
  `error()` ref-qualification static_asserts, move-only E test, base-ref access test.

- `tests/beman/expected/expected.test.cpp` — added: namespace-scope helper types
  (`NoDefault`, `NoCopy`, `ThrowingMove`, `MightThrow`), type-trait static_asserts
  (default ctor constraint, copy ctor constraint, trivially destructible, nothrow move
  constructible/assignable, `operator*` and `error()` ref-qual return types), destructor
  tests (value and error state), `emplace` with initializer_list, `operator->` address
  equality, `value()` ref-qual static_asserts.

- `tests/beman/expected/expected_*_fail.cpp` and `unexpected_*_fail.cpp` (7 new files):
  negative compile tests for ill-formed `unexpected<E>` and `expected<T,E>` instantiations
  and the `emplace` nothrow Mandates.

- `tests/beman/expected/CMakeLists.txt` — registered all 7 negative compile targets as
  WILL_FAIL ctest entries using a `add_fail_test` macro.

Total tests now: 134 (was 117: 17 new Catch2 cases + 7 negative compile tests).

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError` in the tool itself. This is
pre-existing on `main` and unrelated to our changes. All other linters
(clang-format, gersemi/cmake-lint, codespell, whitespace) pass.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # build + run all 117 tests
make lint                    # all linters (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktrees: `step3-expected-primary` still exists at
  `../step3-expected-primary/` but can be deleted
- All work accumulates on `expected-over-references`; this branch will be
  merged to `main` when all steps complete

## Additional Test Plan Documents

The full test plan lives alongside this handoff under `docs/plan/`:

| File | Covers |
|------|--------|
| `tests-overview.md` | Framework, conventions, negative compile pattern, CMakeLists structure |
| `tests-step1.md` | `unexpected<E>` — all testable statements from [expected.un.*] |
| `tests-step2.md` | `bad_expected_access<E>` — [expected.bad] and [expected.bad.void] |
| `tests-step3.md` | `expected<T,E>` primary template — [expected.object.*] excluding monadic |
| `tests-step4.md` | `expected<void,E>` partial specialization |
| `tests-step5.md` | `expected<T,E>` monadic operations |
| `tests-step6.md` | `expected<void,E>` monadic operations |
| `tests-step7.md` | `expected<T&,E>` reference-value specialization (P2988) |
| `tests-step8.md` | `expected<T,E&>` reference-error specialization (P2988) |
| `tests-step9.md` | `expected<T&,E&>` both-reference specialization (P2988) |
| `tests-step10.md`| `expected<void,E&>` void+reference-error specialization (P2988) |

**When starting a new step**, read `tests-overview.md` and the corresponding
`tests-stepN.md` before writing tests. The test plan is the authoritative
description of what needs to be tested; not everything in it may have been
implemented yet in earlier steps.

## Next Step: Step 4

**Step 4**: `expected<void, E>` partial specialization.
See `docs/plan/step4-expected-void.md` for the full plan.

### Key context for Step 4

- Create branch `step4-expected-void` from `expected-over-references` (NOT from main)
- The void specialization is added to `expected.hpp` AFTER the primary template
- Template signature: `template <class T, class E> requires std::is_void_v<T> class expected<T, E>`
- Storage is simpler: only `bool has_val_` + `union { E unex_; }` (no val_ member)
- No `operator->`, no `value_or()`, no from-value constructor, no from-value assignment
- `operator*()` returns `void` (compiles but does nothing)
- `value()` throws if no value, returns void otherwise
- `emplace()` takes no arguments (just resets to has-value state)
- Converting constructors from `expected<U,G>` only when `is_void_v<U>`
- A NEW test file: `tests/beman/expected/expected_void.test.cpp`
- Register the new test file in `tests/beman/expected/CMakeLists.txt`
- The commented-out specification for the void specialization is in
  `expected.hpp` at lines 156-249 (the `/***/` block after the primary template)
- The `emplace` noexcept constraint (`is_nothrow_constructible_v`) does NOT
  apply to the void specialization (it's unconditionally noexcept with no args)
- Step 4 does NOT include monadic operations (Step 6 adds those)

### Emplace for emplace<Args...> constraint reminder

In the primary template, `emplace` requires `is_nothrow_constructible_v<T, Args...>`
as a Mandates clause (expressed as `requires` in our code). This means you
CANNOT test emplace with types whose constructor might throw (e.g.,
`std::string(size_t, char)` is NOT nothrow). Use `int` or other trivially
constructible types for emplace tests.

### After Step 4 is done

Merge (--no-ff) `step4-expected-void` into `expected-over-references`.
Update the checklist in `docs/plan/index.md`.
Write a new `docs/plan/handoff-next.md`.
Step 5 (monadic ops for primary template) and Step 6 (monadic ops for void)
can proceed in either order.
