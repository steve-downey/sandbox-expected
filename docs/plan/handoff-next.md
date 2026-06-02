# Handoff: After Step 7 (expected<T&, E> Complete)

## What Was Done

Step 7 is complete. Branch `step7-expected-ref-t` implements
`expected<T&, E>` — the reference-value partial specialization (P2988).
313 tests pass, lint clean.

### Changes in Step 7

**`include/beman/expected/expected.hpp`:**
- Added `detail::reference_constructs_from_temporary_v` (portable fallback
  using `__cpp_lib_reference_from_temporary` when available, otherwise
  approximation via `is_convertible_v`)
- Added `expected<T&, E>` partial specialization (~670 lines) after the void
  specialization, with:
  - Storage: `union { T* val_; E unex_; }` + `bool has_val_`
  - No default constructor (`= delete`)
  - Copy/move constructors (trivial + non-trivial paths)
  - Value constructor with dangling-prevention delete overload
  - Converting constructors from `expected<U&, G>` (copy and move)
  - Error constructors from `unexpected<G>` (copy and move)
  - `unexpect_t` in-place error constructors
  - Rebind assignment (`operator=`) — rebinds pointer, never assigns through
  - Assignment from `unexpected<G>` (rebinds error)
  - Observers: `operator*()` → `T&`, `operator->()` → `T*`, `value()` → `T&`
  - Shallow const: `const expected<T&, E>` still gives `T*`/`T&` (not const)
  - `value_or()`, `error_or()`, `error()`
  - `swap()`, equality operators
  - Monadic ops: `and_then`, `or_else`, `transform`, `transform_error`
    (2 ref-qualified overloads each — `&` and `&&`)
  - Mandate static_asserts: E must not be reference, void, array, or cv-qual

**New test files:**
- `tests/beman/expected/expected_ref.test.cpp` — 477 lines of runtime tests
- `tests/beman/expected/expected_ref_constraints.test.cpp` — 259 lines of
  static_assert / type-trait checks
- Negative compile tests:
  - `expected_ref_temporary_fail.cpp` — binding temporary to T& is deleted
  - `expected_ref_no_default_fail.cpp` — no default constructor
  - `expected_ref_inplace_value_fail.cpp` — no in_place_t value constructor
  - `expected_ref_e_ref_fail.cpp` — E must not be reference
  - `expected_ref_e_void_fail.cpp` — E must not be void
  - `expected_ref_e_array_fail.cpp` — E must not be array
  - `expected_ref_e_cv_fail.cpp` — E must not be cv-qualified

**`tests/beman/expected/CMakeLists.txt`:**
- Added `beman.expected.tests.expected_ref` target
- Added `beman.expected.tests.expected_ref_constraints` target
- Added all 7 negative-compile fail targets

### Test count

313 tests total, all passing.

## Build Commands

```bash
make TOOLCHAIN=gcc-16 test   # 313 tests, all passing
make lint                    # all linters pass
```

## Step 7 Checklist

- [x] Step 7: `expected<T&, E>` — pointer storage, rebind assignment,
  observers returning T&, value_or, monadic ops, dangling prevention

## What Comes Next

**Step 8: `expected<T, E&>` error-reference specialization.**

Read `docs/plan/step8-expected-ref-e.md` for the full specification.

### Key differences from Step 7

Step 8 is the mirror image: value is owned (same as primary template), error
is a reference (pointer to error object). The union is:

```cpp
union {
    T val_;       // active when has_val_ == true
    E* unex_ptr_; // active when has_val_ == false
};
bool has_val_;
```

### What to reuse from Step 7

- `detail::reference_constructs_from_temporary_v` — already exists, reuse for
  preventing temporaries binding to `E&`
- Mandate static_asserts pattern for T (T must not be reference, void, array,
  cv-qual — same as primary template); add E constraints for reference case
- Same monadic operation structure (2 ref-qual overloads each)
- Same negative-compile test pattern (CMakeLists, WILL_FAIL)

### New test file

`tests/beman/expected/expected_ref_e.test.cpp` — analogous to expected_ref.test.cpp
but testing the error-reference behavior:
- Error rebind: assigning `unexpected(new_err)` changes what error is pointed
  to, does not assign through the error reference
- `error()` returns `E&` (dereferencing `unex_ptr_`)
- Shallow const on error: `const expected<T, int&>` still lets you mutate
  through `.error()`
- Default constructor: works (has_value == true, value-inits T)
- Dangling: construction from temporary via `unexpect_t` must be deleted
