# Plan: Conformance Fixes for expected<T, E> and expected<void, E>

## Motivation

A conformance audit (`docs/conformance-audit.md`) found gaps between the
beman::expected implementation (Steps 1-6) and the C++26 working draft
[expected.syn] through [expected.void.eq]. These must be fixed before
beginning the reference specializations (Steps 7-10) because the reference
specializations will clone many of the same constraint patterns, and we
don't want to propagate the gaps.

## Reference Materials

- **`docs/conformance-audit.md`** — the audit; lists every gap with standard
  section references
- **`docs/standard/expected.txt`** — C++26 standard wording (plain text)
- **`docs/plan/index.md`** — original 10-step plan (this plan is a side quest)

## Phase Overview

| Fix | Branch | Scope | Depends on |
|-----|--------|-------|-----------|
| F1 | `fix1-constraints` | Constructor/assignment/equality constraint bugs | — |
| F2 | `fix2-trivial-smfs` | Trivial special member functions (primary + void) | — |
| F3 | `fix3-monadic-constraints` | requires clauses on monadic operations | — |
| F4 | `fix4-mandates` | static_assert Mandates on observers/monadic | — |
| F5 | `fix5-preconditions-and-minor` | Hardened preconditions, unexpected swap, minor fixes | — |

F1-F3 are independent and may run in parallel.
F4 depends on F3 (monadic Mandates go on the same functions as monadic Constraints).
F5 is independent of the others.

## Standing Conventions

Same as `docs/plan/index.md`:
- Include guards: `#ifndef`/`#define`/`#endif`
- SPDX license: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- Functions defined out-of-line in headers
- `constexpr` everything
- Test framework: Catch2
- Namespace: `beman::expected`
- Test files include `tests/beman/expected/test_config.hpp` (aliases
  `namespace expt`); new tests use `expt::` so they run against both
  beman and std modes — except for tests that verify beman-specific
  diagnostics (static_assert messages, hardened preconditions)

### std cross-check rule

Constraint and Mandates fixes should not break the `beman.expected.tests.std`
target (tests compiled against `std::expected`). If a new test exercises a
constraint that libstdc++ doesn't enforce, put it in the beman-only target.

## Step Details

- [Fix 1: Constructor, assignment, and equality constraints](fix1-constraints.md)
- [Fix 2: Trivial special member functions](fix2-trivial-smfs.md)
- [Fix 3: Monadic operation constraints](fix3-monadic-constraints.md)
- [Fix 4: Mandates static_asserts](fix4-mandates.md)
- [Fix 5: Hardened preconditions and minor fixes](fix5-preconditions-and-minor.md)

## Checklist

- [x] Fix 1: Constructor/assignment/equality constraints
- [x] Fix 2: Trivial special member functions
- [x] Fix 3: Monadic operation constraints
- [x] Fix 4: Mandates static_asserts
- [ ] Fix 5: Hardened preconditions and minor fixes

## After All Fixes

Update `docs/conformance-audit.md` to mark resolved items.
Update `docs/plan/handoff-next.md` with the post-fix state.
Then proceed to Step 7 (`docs/plan/step7-expected-ref-t.md`).
