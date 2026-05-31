# Handoff: After Step 2

## What Was Done

Step 2 is complete. `bad_expected_access<void>` and `bad_expected_access<E>` are
fully implemented and tested on branch `step2-bad-expected-access`.

### Files changed

- `cmake/gcc-flags.cmake` — raised C++ standard from C++20 to C++26 (`-std=gnu++26`,
  `CMAKE_CXX_STANDARD 26`). The build baseline is now **GCC-16 / C++26**.
  Use `make TOOLCHAIN=gcc-16 test` to build and test.

- `include/beman/expected/bad_expected_access.hpp` — replaced the empty namespace with:
  - Forward declaration `template<class E> class bad_expected_access;`
  - `bad_expected_access<void>` explicit specialization (base class):
    - Inherits from `std::exception`
    - Protected default/copy/move constructors and assignment operators (`= default`)
    - Protected `constexpr ~bad_expected_access() = default`
    - Public `constexpr const char* what() const noexcept override`
  - `bad_expected_access<E>` primary template (derived):
    - `constexpr explicit bad_expected_access(E e)` — stores via `std::move`
    - `constexpr const char* what() const noexcept override`
    - Four ref-qualified `error()` observers (`&`, `const&`, `&&`, `const&&`)
    - Private `E unex` member
  - All function bodies defined out-of-line after the classes
  - Include guard fixed to `BEMAN_EXPECTED_BAD_EXPECTED_ACCESS_HPP` (was missing `_HPP`)

- `tests/beman/expected/bad_expected_access.test.cpp` — 11 tests covering:
  - Basic construction and `error()` access
  - `what()` returns `"bad expected access"`
  - Inherits from `std::exception` (slice to reference)
  - All four ref-qualified `error()` overloads
  - `std::string` with move semantics
  - Catchable as `std::exception&` and `bad_expected_access<void>&`

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError` in the tool itself
(`can only concatenate list (not "NoneType") to list` in `config.py`).
This is pre-existing on `main` and unrelated to our changes.
All other linters (clang-format, gersemi, codespell) pass.

## Build Baseline Change

The project now requires **GCC-16** and **C++26**. Run all builds as:

```
make TOOLCHAIN=gcc-16 test
make lint
```

Plain `make` (system `cc`/`c++` = GCC 13) will fail because GCC 13 does not
support `-std=gnu++26`.

## Next Step

Steps 1 and 2 must both be merged (no-ff) to `main` before Step 3 can start.

- Step 1 is on branch `step1-unexpected`
- Step 2 is on branch `step2-bad-expected-access`

Once both are merged, proceed to **Step 3**: `expected<T, E>` primary template.
See `docs/plan/step3-expected-primary.md`.

## Key context for Step 3

- `unexpected<E>` (Step 1) and `bad_expected_access<E>` (Step 2) are now available
- `expected<T, E>` stores either a `T` value or an `unexpected<E>` error in a union
- The primary template is for non-reference, non-void `T` and `E`
- `expected::value()` must throw `bad_expected_access<E>` when there is no value
- The header `include/beman/expected/expected.hpp` exists with the full specification
  in a comment block — same skeleton pattern as Steps 1 and 2
- Key storage: `union { T val_; E unex_; }` with a `bool has_val_` flag
- The reference implementation in `~/src/steve-downey/optional/main` shows patterns
  for the union storage and special member function constraints
- Step 3 does NOT include monadic operations (and_then, or_else, transform,
  transform_error) — those come in Step 5
- Build with `make TOOLCHAIN=gcc-16 test` throughout
