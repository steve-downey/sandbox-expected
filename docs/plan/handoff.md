# Handoff: Current State

## Repository

`beman.expected` — a Beman C++ Standards track reference implementation for
`std::expected` with reference support (P2988 / targeting C++29).

## Working Branch

All step branches are created from and merged back into **`expected-over-references`**
(not `main`). The `main` branch tracks upstream; `expected-over-references` is
the integration branch for this work.

## Current State

Steps 1–7 are complete and merged into `expected-over-references`.
The implementation includes the full conformant `expected<T,E>` primary template,
`expected<void,E>`, monadic operations for both, and `expected<T&,E>` (P2988
reference-value specialization). 313 tests pass.

### Key Files

- `include/beman/expected/expected.hpp` — full implementation:
  `unexpected<E>`, `bad_expected_access`, `expected<T,E>`, `expected<void,E>`,
  `expected<T&,E>` (with monadic ops for all three)
- `tests/beman/expected/` — comprehensive test suite (313 tests)

### Build System

- CMake 3.30+, Ninja Multi-Config
- Catch2 via FetchContent
- `make test` to build and run, `make lint` for pre-commit hooks
- Header-only INTERFACE library (unless modules enabled)
- Default config: Asan

### Coding Rules

- Namespace: `beman::expected` (nested, not `beman::expected::inline_namespace`)
- Include guards: `#ifndef BEMAN_EXPECTED_<FILE>_HPP` / `#define` / `#endif`
- License: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- Angle-bracket includes with full paths: `<beman/expected/unexpected.hpp>`
- Functions defined out-of-line within the header (body after class)
- `constexpr` everything
- Test includes: header under test twice (idempotence), then Catch2, then std

### Reference Implementation

For `optional<T&>` patterns, see `~/src/steve-downey/optional/main`:
- `include/beman/optional/optional.hpp` lines 1515-2119 for the reference
  specialization
- Key pattern: `T* value_ = nullptr` storage, `convert_ref_init_val()` for
  binding, rebind semantics on assignment
- `reference_constructs_from_temporary_v` concept for dangling prevention

## Plan

See `docs/plan/index.md` for the full plan with step index, checklist,
and links to individual step files.

## Test Plan Documents

Each implementation step has a corresponding test plan:

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

Read `tests-overview.md` first, then the `tests-stepN.md` for the step being
worked before writing any tests.

## What Comes Next

Step 8: `expected<T, E&>` error-reference specialization — see `docs/plan/step8-expected-ref-e.md`.
Create branch `step8-expected-ref-e` from `expected-over-references`.
