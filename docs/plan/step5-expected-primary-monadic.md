# Step 5: Monadic Operations for expected<T, E>

**Branch:** `step5-expected-primary-monadic`
**Depends on:** Step 3 (expected<T, E> primary template)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Add the four monadic operations (`and_then`, `or_else`, `transform`,
`transform_error`) to the primary `expected<T, E>` template. Each has 4
ref-qualified overloads (& , &&, const&, const&&) = 16 function templates total.

## Context for Executing Agent

The primary template from Step 3 already has all constructors, observers,
assignment, swap, and equality. This step adds the monadic operations defined
in [expected.object.monadic].

### Monadic operations overview

| Operation | Input state | Calls F with | Returns |
|-----------|-------------|-------------|---------|
| `and_then(F)` | has value | `value()` | `invoke(f, value())` — must return `expected<U, E>` |
| `and_then(F)` | has error | — | `expected<U, E>(unexpect, error())` |
| `or_else(F)` | has value | — | `expected<T, G>(in_place, value())` |
| `or_else(F)` | has error | `error()` | `invoke(f, error())` — must return `expected<T, G>` |
| `transform(F)` | has value | `value()` | `expected<U, E>(invoke(f, value()))` where U = invoke_result |
| `transform(F)` | has error | — | `expected<U, E>(unexpect, error())` |
| `transform_error(F)` | has value | — | `expected<T, G>(in_place, value())` |
| `transform_error(F)` | has error | `error()` | `expected<T, G>(unexpect, invoke(f, error()))` |

### Key constraints per [expected.object.monadic]

For `and_then`:
- Let `U = remove_cvref_t<invoke_result_t<F, decltype(value())>>`
- `U` must be a specialization of `expected`
- `U::error_type` must be `E` (same error type)

For `or_else`:
- Let `G = remove_cvref_t<invoke_result_t<F, decltype(error())>>`
- `G` must be a specialization of `expected`
- `G::value_type` must be `T` (same value type)

For `transform`:
- Let `U = remove_cv_t<invoke_result_t<F, decltype(value())>>`
- Returns `expected<U, E>`
- If `U` is `void`, calls `invoke(f, value())` then returns `expected<void, E>()`
- `U` must not be a specialization of `unexpected`, must not be a reference,
  and must not be `in_place_t` or `unexpect_t`

For `transform_error`:
- Let `G = remove_cv_t<invoke_result_t<F, decltype(error())>>`
- Returns `expected<T, G>`
- `G` must be a valid error type for `unexpected`

### Ref-qualification pattern

Each operation has 4 overloads matching the object's value category:
```cpp
template<class F> constexpr auto and_then(F&& f) &;
template<class F> constexpr auto and_then(F&& f) &&;
template<class F> constexpr auto and_then(F&& f) const &;
template<class F> constexpr auto and_then(F&& f) const &&;
```

The value/error passed to F matches the ref-qualification:
- `&` → passes `value()` (lvalue ref)
- `&&` → passes `std::move(value())` (rvalue ref)
- `const &` → passes `value()` (const lvalue ref)
- `const &&` → passes `std::move(value())` (const rvalue ref)

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add 16 monadic member functions
   to the primary template

2. **New test file: `tests/beman/expected/expected_monadic.test.cpp`** — tests:
   - `and_then`: value case chains, error case short-circuits
   - `or_else`: error case chains, value case short-circuits
   - `transform`: transforms value, preserves error
   - `transform_error`: preserves value, transforms error
   - `transform` with void return (F returns void → expected<void, E>)
   - Ref-qualification: verify move semantics on && overloads
   - Chaining: `e.and_then(f).transform(g).or_else(h)`

3. **Update `tests/beman/expected/CMakeLists.txt`** — add new test file

## Procedure

1. Create branch `step5-expected-primary-monadic` from `main` (with Step 3 merged)
2. Add `#include <functional>` if not already present (for `std::invoke`)
3. Add the 16 monadic member function declarations inside the class body
4. Implement each out-of-line after the class body
5. Create test file with comprehensive monadic tests
6. Register in CMakeLists
7. Run `make test` and `make lint`
8. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 6

Step 5 done, next read `docs/plan/step6-expected-void-monadic.md`.
The primary template now has full monadic support. Step 7 will specialize
it for references.
