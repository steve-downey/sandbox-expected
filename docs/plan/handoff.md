# Handoff: Starting State

## Repository

`beman.expected` — a Beman C++ Standards track reference implementation for
`std::expected` with reference support (P2988 / targeting C++29).

## Current State

The repository has a complete skeleton: all standard interface declarations
exist as comments in the header files, but no actual C++ class definitions
have been written. The headers compile (they're empty namespaces), and the
tests are breathing tests only (`EXPECT_EQ(true, true)`).

### Files

- `include/beman/expected/expected.hpp` — commented-out specification for
  `expected<T,E>` and `expected<void,E>`, empty `beman::expected` namespace
- `include/beman/expected/unexpected.hpp` — commented-out specification for
  `unexpected<E>`, empty namespace
- `include/beman/expected/bad_expected_access.hpp` — commented-out specification
  for `bad_expected_access<E>` and `bad_expected_access<void>`, empty namespace
- `tests/beman/expected/expected.test.cpp` — breathing test only
- `tests/beman/expected/unexpected.test.cpp` — breathing test only
- `tests/beman/expected/bad_expected_access.test.cpp` — breathing test

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

Step 1: Implement `unexpected<E>` — see `docs/plan/step1-unexpected.md`.
