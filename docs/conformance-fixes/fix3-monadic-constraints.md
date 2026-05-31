# Fix 3: Monadic Operation Constraints

**Branch:** `fix3-monadic-constraints`
**Depends on:** —
**Read first:** `docs/conformance-audit.md` (sections 4.8, 5.7),
`docs/standard/expected.txt` [expected.object.monadic] and [expected.void.monadic]

---

## Goal

Add `requires` clauses to all monadic operations that the standard marks as
Constraints. Currently, all monadic operations only have `static_assert` for
their Mandates but no SFINAE-friendly constraints.

These are audit items 13-16.

## Which operations need constraints

### Primary template `expected<T, E>`

| Operation | Overload | Constraint |
|-----------|----------|-----------|
| `and_then` | `&`, `const&` | `is_constructible_v<E, decltype(error())>` (i.e. `const E&` for const, `E&` for non-const) |
| `and_then` | `&&`, `const&&` | `is_constructible_v<E, decltype(std::move(error()))>` |
| `or_else` | `&`, `const&` | `is_constructible_v<T, decltype(*(*this))>` (i.e. `T&`/`const T&`) |
| `or_else` | `&&`, `const&&` | `is_constructible_v<T, decltype(std::move(*(*this)))>` |
| `transform` | `&`, `const&` | `is_constructible_v<E, decltype(error())>` |
| `transform` | `&&`, `const&&` | `is_constructible_v<E, decltype(std::move(error()))>` |
| `transform_error` | `&`, `const&` | `is_constructible_v<T, decltype(*(*this))>` |
| `transform_error` | `&&`, `const&&` | `is_constructible_v<T, decltype(std::move(*(*this)))>` |

### Void specialization `expected<void, E>`

| Operation | Overload | Constraint |
|-----------|----------|-----------|
| `and_then` | `&`, `const&` | `is_constructible_v<E, decltype(error())>` |
| `and_then` | `&&`, `const&&` | `is_constructible_v<E, decltype(std::move(error()))>` |
| `or_else` | all | No constraints in standard (none needed) |
| `transform` | `&`, `const&` | `is_constructible_v<E, decltype(error())>` |
| `transform` | `&&`, `const&&` | `is_constructible_v<E, decltype(std::move(error()))>` |
| `transform_error` | all | No constraints in standard (none needed) |

## Implementation

For each affected declaration, add a `requires` clause. Example for
`and_then` lvalue overload on the primary template:

**Declaration (in class body):**
```cpp
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto and_then(F&& f) &;
```

**Out-of-line definition:**
```cpp
template <class T, class E>
template <class F>
    requires std::is_constructible_v<E, E&>
constexpr auto expected<T, E>::and_then(F&& f) & {
    // ... existing implementation unchanged ...
}
```

For `const&`: `requires std::is_constructible_v<E, const E&>`
For `&&`: `requires std::is_constructible_v<E, E&&>`  (equivalently `is_move_constructible_v<E>`)
For `const&&`: `requires std::is_constructible_v<E, const E&&>`

Note: `is_constructible_v<E, E&>` is equivalent to `is_constructible_v<E, decltype(error())>`
when called on a non-const lvalue expected. The standard uses `decltype(error())`; we
expand it to the concrete type for the requires clause.

For `or_else` and `transform_error` on the primary template:
- `&`: `requires std::is_constructible_v<T, T&>` (i.e. copy-constructible from lvalue)
- `const&`: `requires std::is_constructible_v<T, const T&>` (copy-constructible)
- `&&`: `requires std::is_constructible_v<T, T&&>` (move-constructible)
- `const&&`: `requires std::is_constructible_v<T, const T&&>`

## Tests

Add to a new file `tests/beman/expected/expected_monadic_constraints.test.cpp`
(beman-only target — constraint SFINAE behavior):

- Verify that `and_then` is SFINAE-removed (not a hard error) when E is
  not constructible from the relevant reference. Use a concept check:
  ```cpp
  template <class X, class F>
  concept has_and_then = requires(X x, F f) { x.and_then(f); };
  ```
  Then `static_assert(!has_and_then<...>)` for the failing case.

- Same pattern for `or_else`, `transform`, `transform_error`.

All existing monadic tests must still pass (the constraints should be
satisfied in all existing test cases).

## Verification

```bash
make TOOLCHAIN=gcc-16 test
make lint
```

## Handoff

Merge (--no-ff) into `expected-over-references`.
F4 (Mandates) adds `static_assert` to the same functions.
