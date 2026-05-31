# Handoff: After Step 6

## What Was Done

Step 6 is complete. Monadic operations for `expected<void, E>` partial
specialization are implemented and tested on branch `step6-expected-void-monadic`,
then merged (--no-ff) into `expected-over-references`.

### Files changed

- `include/beman/expected/expected.hpp`
  - Added 16 monadic method declarations inside the void specialization class
    body (after `error_or`, before `[expected.void.eq]`)
  - Added 16 out-of-line implementations after the void `error_or`
    implementations, before the closing `} // namespace expected`
  - Key difference from primary: `and_then`/`transform` call `invoke(f)` with
    **no arguments** (void has no stored value). `or_else`/`transform_error`
    call `invoke(f, error())` as in the primary template.
  - `or_else` has value path returns `G()` (not `G(in_place, val_)` — T is void)
  - `transform_error` has value path returns `expected<void, G>()` (not
    `expected<T, G>(in_place, val_)`)
  - `or_else` static_assert checks `is_void_v<typename G::value_type>` (not
    `is_same_v<typename G::value_type, T>`) — the void-specialization mandates
    that G's value_type is void, and `is_void_v` is the cleaner check

- `tests/beman/expected/expected_void_monadic.test.cpp` — 15 Catch2 test cases:
  - `and_then`: value (F called with no args), error short-circuit, rvalue, void
    result, chaining void→value
  - `or_else`: error recovery, value short-circuit, error propagation through lambda
  - `transform`: value→int, error propagation, void-returning F, rvalue overload
  - `transform_error`: error transform, value pass-through
  - Chaining: and_then → transform_error, error-path end-to-end
  - All 4 ref-qualifications compile test for `and_then`

- Negative compile test files (2 new):
  - `void_and_then_wrong_error_type_fail.cpp` — F returns wrong E type
  - `void_or_else_wrong_value_type_fail.cpp` — F returns non-void value_type

- `tests/beman/expected/CMakeLists.txt` — added `expected_void_monadic.test.cpp`
  to the main test executable; registered 2 negative compile tests

### Test count

231 tests total, all passing (was 212 before this step; 19 new tests including
the 2 negative compile tests).

### Known pre-existing issue

`beman-tidy` crashes with a Python `TypeError`. Pre-existing on `main` and
unrelated to our changes. All other linters pass.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 231 tests, all passing
make lint                    # all linters (beman-tidy crash is pre-existing)
```

## Current Branch State

- Feature branch: `expected-over-references`
- Worktree `../step6-expected-void-monadic/` may be deleted:
  `git worktree remove ../step6-expected-void-monadic`
- All work accumulates on `expected-over-references`; this branch will be
  merged to `main` when all steps complete

## Step Checklist

- [x] Step 1: `unexpected<E>`
- [x] Step 2: `bad_expected_access<E>`
- [x] Step 3: `expected<T, E>` primary template
- [x] Step 4: `expected<void, E>` specialization
- [x] Step 5: `expected<T, E>` monadic ops
- [x] Step 6: `expected<void, E>` monadic ops  ← just done
- [ ] Step 7: `expected<T&, E>` reference specialization
- [ ] Step 8: `expected<T, E&>` error-reference specialization
- [ ] Step 9: `expected<T&, E&>` both-reference specialization
- [ ] Step 10: `expected<void, E&>` void+error-ref specialization

## Next Step: Step 7

**Step 7**: `expected<T&, E>` — a new partial specialization where the value
type is a reference. This is the core novel work of the proposal (P2988 design).
See `docs/plan/step7-expected-ref-t.md` and `docs/plan/tests-step7.md`.

### Critical design decisions for Step 7

**Storage** — mirror the primary template's union pattern but store a pointer:
```cpp
private:
    bool has_val_;
    union {
        T* val_;   // pointer to referred object when has_val_ == true
        E unex_;   // error when has_val_ == false
    };
```

**Rebind semantics** — assignment changes what `T*` points to, never assigns
through the reference. This is the key difference from a plain reference member:
```cpp
// Assign from lvalue: rebind (do not assign-through)
template <class U>
constexpr expected& operator=(U&& u) {
    T& r(std::forward<U>(u));      // form the reference
    val_ = std::addressof(r);       // store the address
    ...
}
```

**Shallow const** — `const expected<T&, E>` still allows mutation of T.
`operator*()` on const returns `T&` (not `const T&`).

**No default constructor** — T& cannot be null; there is no "empty" state.

**Dangling prevention** — delete constructors that would bind temporaries:
```cpp
template <class U>
    requires reference_constructs_from_temporary_v<T&, U&&>
constexpr expected(U&&) = delete;
```

**`reference_constructs_from_temporary_v`** — use the compiler built-in if
available (GCC 13+, Clang 16+), else the portable fallback from
`~/src/steve-downey/optional/main/include/beman/optional/optional.hpp`
lines ~1480-1511. See that file — it uses `is_convertible_v` checks.

### Reference implementation to study

`~/src/steve-downey/optional/main/include/beman/optional/optional.hpp`
lines 1515-2119 implements `optional<T&>` with identical design. Read it before
writing `expected<T&, E>`. The class structure, storage, assignment, and
observer patterns transfer directly — just replace the `optional` aspects with
`expected` (add error storage/handling, no `nullopt` constructor, has `or_else`
and `transform_error` instead of just `and_then` / `transform`).

### What does NOT exist yet in the codebase

- `reference_constructs_from_temporary_v` — needs to be added in `detail` namespace
- `expected<T&, E>` specialization — does not exist; the primary template has a
  `static_assert(!is_reference_v<T>)` that would fire if attempted
- Any reference-specialization tests

### Monadic ops for expected<T&, E>

Same 16 declarations + implementations as primary template, but:
- `and_then(F)`: pass `*(*this)` which is `T&` (dereference the pointer via operator*)
- `transform(F)`: same — `invoke(f, **this)` 
- `or_else(F)` / `transform_error(F)`: unchanged (operate on error)

The `or_else` value-path returns `G(std::in_place, **this)` — but wait, T is a
reference type. `G` is `expected<T&, G_err>` or similar. Check what makes sense:
the value path of `or_else` just returns the current object wrapped in G. Since
G::value_type must equal T (which is T&), `G(std::in_place, *this->val_)` or
just `G(**this)` works.

### Negative compile tests for Step 7

- `expected_ref_temporary_fail.cpp` — construct from rvalue/temporary
- `expected_ref_no_default_fail.cpp` — default construction
- `expected_ref_inplace_value_fail.cpp` — `in_place_t` value construction

### After step is done

Merge (--no-ff) the step branch into `expected-over-references`.
Update the checklist in `docs/plan/index.md`.
Write a new `docs/plan/handoff-next.md`.
