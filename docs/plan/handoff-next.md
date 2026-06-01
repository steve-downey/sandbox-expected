# Handoff: After Fix 5 (All Conformance Fixes Complete)

## What Was Done

Fix 5 is complete. Branch `fix5-preconditions-and-minor` merged (--no-ff) into
`expected-over-references`. All conformance fixes (F1–F5) are now merged.

### Changes in Fix 5

**`include/beman/expected/expected.hpp`:**
- Added `BEMAN_EXPECTED_HARDENED` precondition guards (`__builtin_trap()`) to:
  - `operator->()` (both const and non-const) — checks `has_val_`
  - `operator*()` (all 4 overloads) — checks `has_val_`
  - `error()` (all 4 overloads, primary template) — checks `!has_val_`
  - `operator*()` (void specialization) — checks `has_val_`
  - `error()` (all 4 overloads, void specialization) — checks `!has_val_`
- Void `or_else` (4 overloads): changed `is_void_v<G::value_type>` to
  `is_same_v<G::value_type, T>` — correct for `const void` etc.
- Void `transform_error` (4 overloads): changed `expected<void, G>` to
  `expected<T, G>` — matches standard wording

**`include/beman/expected/unexpected.hpp`:**
- Added `requires std::is_swappable_v<E>` to friend swap

**Tests added:**
- `tests/beman/expected/expected_hardened.test.cpp` — compiled with
  `-DBEMAN_EXPECTED_HARDENED`, verifies happy paths and swap constraint
- New CMake target `beman.expected.tests.hardened`

### Test count

253 tests total, all passing.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 253 tests, all passing
make lint                    # all linters pass (beman-tidy crash is pre-existing)
```

## Conformance Fix Checklist

- [x] Fix 1: Constructor/assignment/equality constraints
- [x] Fix 2: Trivial special member functions
- [x] Fix 3: Monadic operation constraints
- [x] Fix 4: Mandates static_asserts
- [x] Fix 5: Hardened preconditions and minor fixes  ← just done

## All Conformance Fixes Complete

The conformance fixes phase is finished. Per `docs/conformance-fixes/index.md`,
the next actions are:

1. Update `docs/conformance-audit.md` to mark resolved items
2. Update this handoff for the post-fix state
3. Proceed to Step 7: `docs/plan/step7-expected-ref-t.md`

## State of the Implementation

The `expected-over-references` branch now has a fully conformant `expected<T,E>`
and `expected<void,E>` (modulo the extensions noted in the audit as conforming).
All constraint, Mandates, trivial SMF, monadic SFINAE, and precondition gaps
identified in the audit are resolved.
