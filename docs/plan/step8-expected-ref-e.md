# Step 8: Implement expected<T, E&> Error-Reference Specialization

**Branch:** `step8-expected-ref-e`
**Depends on:** Steps 3, 5 (primary template with monadic ops)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement `expected<T, E&>` — a specialization where the error type is a
reference. This allows an expected to hold a non-owning reference to an error
object, with rebind semantics on error assignment.

## Context for Executing Agent

This is the mirror image of Step 7. Where Step 7 has a reference for the value
and a value for the error, this step has a value for the value and a reference
for the error.

### Storage model

```cpp
template <class T, class E>
class expected<T, E&> {
private:
    bool has_val_;
    union {
        T val_;
        E* unex_ptr_;
    };
};
```

When `has_val_` is true, `val_` is active (value type T, same as primary).
When `has_val_` is false, `unex_ptr_` is active (pointer to the error object).

### Design principles

Same rebind semantics as Step 7, applied to the error side:

1. **Error rebind**: Assignment from `unexpected<G>` rebinds the error pointer
   (changes what error the expected refers to), does not assign through
2. **Value is owned**: The value type T is stored by value, same as primary
3. **Error observers**: `error()` returns `E&` (dereferencing the pointer)
4. **unexpected_type**: `unexpected_type` is `unexpected<E>` (not `unexpected<E&>`)

### Constructors

- **Default**: value-initializes T (same as primary)
- **From value**: same as primary (`U&&` → constructs T)
- **From unexpected**: binds error reference:
  ```cpp
  template<class G>
  constexpr expected(const unexpected<G>& e) {
      // e.error() is a const G& — need to bind E& to it
      // This only works if E is constructible from const G&
      // Actually, for E& as error type, we store a pointer
      E& r(e.error());
      unex_ptr_ = std::addressof(r);
      has_val_ = false;
  }
  ```
  But wait — `unexpected` stores by value, so `e.error()` returns `const G&`.
  For `E& = G&`, we'd need the unexpected to provide a non-const reference.
  This means construction from `unexpected<G>&` (non-const) works when
  `E` is non-const, and from `const unexpected<G>&` when `E` is const.

  Alternative: the `expected<T, E&>` can also be constructed directly with
  `(unexpect_t, E& ref)` to bind the error reference.

- **unexpect_t constructors**: `(unexpect_t, Args&&...)` where the args
  construct/bind `E&` — practically `(unexpect_t, E& ref)`
- **Dangling prevention**: delete constructors that would bind temporaries
  to `E&`

### Assignment (rebind on error side)

```cpp
// From unexpected — rebind error reference
template<class G>
constexpr expected& operator=(const unexpected<G>& e) {
    if (has_val_) {
        std::destroy_at(&val_);
        bind_error_ref(e.error());
        has_val_ = false;
    } else {
        // Rebind error pointer
        bind_error_ref(e.error());
    }
    return *this;
}
```

### Observers

- `operator*()`, `operator->()`, `value()`, `value_or()` — same as primary
- `error()` returns `E&` (dereferences `unex_ptr_`)
- `error_or(G&&)` returns `E` by value (common type resolution)

### Monadic operations

Same as primary but `error()` returns `E&`:
- `or_else(F)`: passes `E&` to F
- `transform_error(F)`: passes `E&` to F, wraps result
- `and_then(F)`, `transform(F)`: same as primary (works on value)

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add `expected<T, E&>` partial
   specialization

2. **New test file: `tests/beman/expected/expected_ref_e.test.cpp`** — tests:
   - Default construction (has_value == true)
   - Construction from value (same as primary)
   - Construction from error reference via unexpect_t
   - **Error rebind**: assign new error reference, verify old error unchanged
   - `error()` returns `E&`, mutation through reference works
   - `value()` works normally (owned T)
   - Copy/move construction
   - `swap()`
   - Equality operators
   - Monadic operations: `or_else` and `transform_error` pass E& to callable
   - **Dangling prevention**: binding temporary to E& should not compile
   - **Shallow const on error**: `const expected<T, int&>` allows mutation
     of error through `.error()`

3. **Update `tests/beman/expected/CMakeLists.txt`**

## Procedure

1. Create branch from `main` (with Steps 1-5 merged)
2. Add `expected<T, E&>` partial specialization
3. Implement storage (value union member + error pointer + bool)
4. Implement constructors with dangling prevention on error side
5. Implement rebind assignment for error
6. Implement observers (value same as primary, error returns reference)
7. Implement monadic operations
8. Implement swap and equality
9. Write comprehensive tests
10. Run `make test` and `make lint`
11. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 9

Step 8 done, next read `docs/plan/step9-expected-ref-both.md`.
`expected<T, E&>` is complete. Step 9 combines both reference types.
