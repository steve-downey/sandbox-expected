# Handoff: After Step 4

## What Was Done

Step 4 is complete. `expected<void, E>` partial specialization is implemented
and tested on branch `step4-expected-void`, then merged (--no-ff) into
`expected-over-references`.

### Files changed

- `include/beman/expected/expected.hpp` — added the `expected<T, E> requires
  is_void_v<T>` partial specialization after the primary template:
  - Class definition with `static_assert`s for ill-formed E (same constraints
    as primary template: no reference, array, cv-qualified, or unexpected<X>)
  - Trivial copy constructor (`= default`) constrained on
    `is_trivially_copy_constructible_v<E>`, plus non-trivial overload
  - Trivial move constructor (`= default`) constrained on
    `is_trivially_move_constructible_v<E>`, plus non-trivial overload
  - Trivial/non-trivial destructor pair (same pattern as primary)
  - Converting constructors from `expected<U, G>` with `is_void_v<U>` (const& and &&)
  - `unexpected<G>` constructors (const& and &&)
  - `in_place_t` constructor (noexcept, no args, sets has_val_ = true)
  - `unexpect_t` constructors (Args..., initializer_list+Args...)
  - Copy/move assignment, assign from `unexpected<G>` (const& and &&)
  - `emplace()` — no args, noexcept, transitions error→value
  - `swap()` — all four state combinations
  - Observers: `operator bool()`, `has_value()`, `operator*()` (void),
    `value()` (const& and &&), `error()` (4 ref-qualified), `error_or()`
  - Equality: `expected<void>==expected<void,E2>` (requires `is_void_v<T2>`),
    `expected<void>==unexpected<E2>`
  - Storage: `bool has_val_` + `union { E unex_; }` (no val_ member)

- `tests/beman/expected/expected_void.test.cpp` — 39 new Catch2 test cases:
  - All constructors, destructor, copy/move assignment, assign from unexpected
  - `emplace()` both states, `swap()` all four combinations
  - `operator*` void, `value()` success and throws, `error()` ref-quals,
    `error_or()` both states
  - Equality: both-value, both-error, mixed, with unexpected, cross-type
  - Type-trait static_asserts: nothrow default ctor, trivially copy constructible,
    nothrow move constructible, trivially destructible, nothrow move assignable
  - Constraint static_assert: `!is_copy_constructible_v<expected<void, NoCopyE>>`,
    `!is_constructible_v<expected<void,int>, expected<int,int>>`

- `tests/beman/expected/expected_void_ref_fail.cpp` — negative compile test:
  `expected<void, int&>` is ill-formed

- `tests/beman/expected/expected_void_array_fail.cpp` — negative compile test:
  `expected<void, int[]>` is ill-formed

- `tests/beman/expected/CMakeLists.txt` — added `expected_void.test.cpp` to
  the main test executable; registered both negative compile tests via
  `add_fail_test` macro

- Minor whitespace-only changes in `unexpected.hpp`, `bad_expected_access.test.cpp`,
  `expected.test.cpp`, `unexpected.test.cpp` — applied by clang-format

### GCC 16 / requires-expression caveat

GCC 16 treats `e.operator->()` and `e.member_name()` inside `requires`
expressions as hard errors (not SFINAE) when the member doesn't exist. This
is a known GCC limitation. The absence of `operator->` and `value_or` for
the void specialization is verified instead by negative compile tests.

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError` in the tool itself. Pre-existing
on `main` and unrelated to our changes. All other linters pass.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 175 tests, all passing
make lint                    # all linters (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktree: `../step4-expected-void/` (can be deleted: `git worktree remove`)
- All work accumulates on `expected-over-references`; this branch will be
  merged to `main` when all steps complete

## Step Checklist

- [x] Step 1: `unexpected<E>`
- [x] Step 2: `bad_expected_access<E>`
- [x] Step 3: `expected<T, E>` primary template
- [x] Step 4: `expected<void, E>` specialization  ← just done
- [ ] Step 5: `expected<T, E>` monadic ops
- [ ] Step 6: `expected<void, E>` monadic ops
- [ ] Step 7: `expected<T&, E>` reference specialization
- [ ] Step 8: `expected<T, E&>` error-reference specialization
- [ ] Step 9: `expected<T&, E&>` both-reference specialization
- [ ] Step 10: `expected<void, E&>` void+error-ref specialization

## Next Step: Step 5 or Step 6

Steps 5 and 6 are independent of each other; either can proceed next.

**Step 5**: Monadic operations for `expected<T, E>` (primary template):
`and_then`, `or_else`, `transform`, `transform_error` — each with 4
ref-qualified overloads. See `docs/plan/step5-expected-primary-monadic.md`.

**Step 6**: Monadic operations for `expected<void, E>`:
Same four operations but adapted for void value semantics.
See `docs/plan/step6-expected-void-monadic.md`.

The recommended order is Step 5 first (primary template monadic ops), then
Step 6 (void monadic ops), since Step 5 is larger and Step 6 can reuse
patterns established in Step 5.

### Key context for Step 5

- Create branch `step5-expected-primary-monadic` from `expected-over-references`
- Add monadic operations INSIDE the `expected<T, E>` class body (hidden
  friend / member function templates)
- Return type of `and_then(F&&)` is `invoke_result_t<F, T&>` — must be an
  expected specialization (Mandates constraint)
- The four operations each have four ref-qualified overloads (const&, &, const&&, &&)
- Tests go in the existing `expected.test.cpp` (or a new
  `expected_monadic.test.cpp` — either is fine)
- Read `docs/plan/tests-step5.md` before writing any tests

### After step is done

Merge (--no-ff) the step branch into `expected-over-references`.
Update the checklist in `docs/plan/index.md`.
Write a new `docs/plan/handoff-next.md`.
