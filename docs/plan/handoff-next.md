# Handoff: After Step 1

## What Was Done

Step 1 is complete. `unexpected<E>` is fully implemented and tested on branch
`step1-unexpected`.

### Files changed

- `include/beman/expected/unexpected.hpp` — replaced the empty namespace with:
  - `unexpect_t` struct and `unexpect` inline constexpr instance
  - `unexpected<E>` class template with all standard members:
    - Copy/move constructors (= default)
    - Converting constructor with `requires` constraints (excludes `unexpected`
      and `in_place_t` as `Err`, mandates `is_constructible_v<E, Err>`)
    - Two in-place constructors (`in_place_t, Args...` and
      `in_place_t, initializer_list<U>, Args...`)
    - Copy/move assignment (= default)
    - Four ref-qualified `error()` observers
    - `swap()` member (noexcept conditional on `is_nothrow_swappable_v<E>`)
    - `operator==` hidden friend (cross-type `E2`)
    - `swap()` hidden friend (ADL)
  - CTAD deduction guide: `template<class E> unexpected(E) -> unexpected<E>`
  - All function bodies defined out-of-line after the class

- `tests/beman/expected/unexpected.test.cpp` — 22 tests covering all of the above

- `examples/CMakeLists.txt` — extra blank line removed by gersemi (CMake
  formatter), no semantic change

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError` in the tool itself
(`can only concatenate list (not "NoneType") to list` in `config.py`).
This is pre-existing on `main` and unrelated to our changes.
All other linters (clang-format, gersemi, codespell) pass.

## Next Step

Step 2: Implement `bad_expected_access<E>` — see
`docs/plan/step2-bad-expected-access.md`.

Steps 1 and 2 were independent; Step 2 branches from `main` (not from
`step1-unexpected`). After Step 2 is done, both Step 1 and Step 2 must be
merged (no-ff) to `main` before Step 3 can start.

## Key context for Step 2

- The header `include/beman/expected/bad_expected_access.hpp` exists with the
  full specification in a comment block — same skeleton pattern as Step 1.
- The exception types are needed so that `expected::value()` can throw when
  there's no value.
- `bad_expected_access<void>` is the base (protected ctors, public `what()`).
- `bad_expected_access<E>` derives from it and stores the error value.
- `what()` should return `"bad expected access"` (libstdc++/libc++ convention).
- Use `<exception>` and `<utility>`; no other new includes should be needed.
- Follow the same conventions: `constexpr` everywhere, out-of-line definitions,
  angle-bracket includes, `#ifndef`/`#define`/`#endif` guards.
