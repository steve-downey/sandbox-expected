# Handoff: Current State

## Repository

`beman.expected` — a Beman C++ Standards track reference implementation for
`std::expected` with reference support (P2988 / targeting C++29).

## Working Branch

All step branches are created from and merged back into **`expected-over-references`**
(not `main`). The `main` branch tracks upstream; `expected-over-references` is
the integration branch for this work.

## Current State

All 10 steps are complete. Steps 1–9 are merged into `expected-over-references`.
Step 10 is on branch `step10-expected-void-ref-e`, ready to merge.
The implementation includes the full conformant `expected<T,E>` primary template,
`expected<void,E>`, monadic operations for both, `expected<T&,E>` (P2988
reference-value specialization), `expected<T,E&>` (P2988 reference-error
specialization), `expected<T&,E&>` (P2988 both-reference specialization), and
`expected<void,E&>` (P2988 void+reference-error specialization).
470 tests pass.

### Key Files

- `include/beman/expected/expected.hpp` — full implementation:
  `unexpected<E>`, `bad_expected_access`, `expected<T,E>`, `expected<void,E>`,
  `expected<T&,E>`, `expected<T,E&>`, `expected<T&,E&>`, `expected<void,E&>`
  (with monadic ops for all)
- `tests/beman/expected/` — comprehensive test suite (470 tests)

### Step 10 Design Notes

- `expected<void, E&>` is a partial specialization `template<class E> class expected<void, E&>`
  This is more specific than both `expected<T,E> requires is_void_v<T>` and
  `expected<T,E&>`, so it is correctly selected for `expected<void, int&>`.
- Storage: `E* unex_ptr_` + `bool has_val_` (no union — void has no value)
- **Trivially copyable/destructible**: pointer + bool, all operations default
- **Default constructible**: sets `has_val_ = true` (void success state)
- **Shallow const on error**: `error()` always returns `E&` regardless of
  `const` on the `expected` object
- **Dangling prevention**: `expected(unexpect_t, G&&)` is deleted when
  `reference_constructs_from_temporary_v<E&, G>` is true
- **No `operator=` from `unexpected<G>`**: `unexpected<G>` requires G to be
  an object type (reference types are ill-formed). Rebind is done via copy
  assignment from another `expected<void, E&>`, same as in `expected<T,E&>`.
- **`emplace()` returns void**: sets `has_val_ = true`, no construction needed
- **`transform_error(F)`** returns `expected<void, G>` — the plain void
  specialization (non-reference error, since F transforms E& → G by value)
- **Removed `expected_void_ref_fail` negative compile test**: this test
  asserted `expected<void, int&>` was ill-formed in `expected<T,E&>`. Now that
  Step 10 adds `expected<void, E&>`, this is valid and the test was removed.

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
- Angle-bracket includes with full paths: `<beman/expected/expected.hpp>`
- Functions defined inline within the class for the reference specializations
- `constexpr` everything
- Test includes: header under test twice (idempotence), then Catch2, then std

### Reference Implementation

For `optional<T&>` patterns, see `~/src/steve-downey/optional/main`:
- `include/beman/optional/optional.hpp` lines 1515-2119 for the reference
  specialization

## Plan

See `docs/plan/index.md` for the full plan with step index, checklist,
and links to individual step files. **All 10 steps are now complete.**

## What Comes Next

The core implementation is complete. The full set of specializations:

| Specialization | Value storage | Error storage | Step |
|----------------|---------------|---------------|------|
| `expected<T, E>` | T (owned) | E (owned) | 3, 5 |
| `expected<void, E>` | none | E (owned) | 4, 6 |
| `expected<T&, E>` | T* (pointer) | E (owned) | 7 |
| `expected<T, E&>` | T (owned) | E* (pointer) | 8 |
| `expected<T&, E&>` | T* (pointer) | E* (pointer) | 9 |
| `expected<void, E&>` | none | E* (pointer) | 10 |

Remaining work beyond the core implementation:

1. **Merge Step 10** into `expected-over-references`
2. **Paper/proposal writing** — document the API surface for the C++ committee
3. **Additional test coverage** — edge cases, more type combinations
4. **Performance benchmarks** — compare with std::expected on supported compilers
5. **CI/CD setup** — ensure all specializations are tested on multiple compilers
6. **Upstream PR** — submit to beman project upstream
