# Step 10: Implement expected<void, E&> Specialization

**Branch:** `step10-expected-void-ref-e`
**Depends on:** Steps 4, 6 (expected<void, E> with monadic ops)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement `expected<void, E&>` — the final specialization combining void
value semantics (from Step 4/6) with error-reference semantics (from Step 8).

## Context for Executing Agent

This is the intersection of the void specialization and the error-reference
specialization. There is no value storage (void), and the error is a
non-owning reference with rebind semantics.

### Storage model

```cpp
template <class T, class E>
    requires std::is_void_v<T>
class expected<T, E&> {
private:
    E* unex_ptr_ = nullptr;  // null when has_val_ == true
    bool has_val_ = true;
};
```

Or using a union:
```cpp
private:
    bool has_val_ = true;
    union {
        char dummy_;
        E* unex_ptr_;
    };
```

Actually, since there's no value to store in the union (void), and the error
is just a pointer, we can simplify:

```cpp
private:
    E* unex_ptr_ = nullptr;
    bool has_val_ = true;
```

When `has_val_` is true, `unex_ptr_` is irrelevant (void/success state).
When `has_val_` is false, `unex_ptr_` points to the error object.

### Key properties

- **Default constructible**: void state is valid, sets `has_val_ = true`
- **No value storage**: `operator*()` returns void, no `operator->()`,
  no `value_or()`
- **Error is reference**: `error()` returns `E&`, rebind on assignment
- **Fully trivial operations**: no non-trivial members to destroy

### Constructors

- Default: `has_val_ = true`
- Copy, move (trivial)
- From `unexpected<G>` — binds E& (with dangling prevention)
- `(in_place_t)` — explicit, sets `has_val_ = true`
- `(unexpect_t, Args...)` — binds E&
- Converting from `expected<void, G&>`

### Assignment

- Copy/move (trivial)
- From `unexpected<G>` — rebinds error pointer
- `emplace()` — transitions from error to void state (just set `has_val_ = true`)

### Observers

- `operator*()` — void, no-op
- `value()` — void return, throws if no value
- `error()` → `E&` (dereference pointer)
- `error_or(G&&)` → `E` by value
- No `operator->()`, no `value_or()`

### Monadic operations

Combine void-value patterns (Step 6) with error-reference patterns (Step 8):
- `and_then(F)` — invoke F with no args (void value), propagate E&
- `or_else(F)` — invoke F with E& (error reference)
- `transform(F)` — invoke F with no args
- `transform_error(F)` — invoke F with E&

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add `expected<void, E&>`
   specialization (or handle via requires clause combining is_void and
   is_lvalue_reference)

   Note on template matching: you may need to handle this as
   `template<class T, class E> requires is_void_v<T>` on `expected<T, E&>`
   or as a direct `expected<void, E&>` specialization. Choose whichever
   approach avoids ambiguity with the existing void and E& specializations.

2. **New test file: `tests/beman/expected/expected_void_ref_e.test.cpp`**:
   - Default construction (has_value == true)
   - Construction from error reference via unexpected
   - **Error rebind**: assign new error reference
   - Copy/move construction (trivial)
   - `operator*()` returns void
   - `value()` returns void on success, throws on error
   - `error()` returns E&, mutation through reference works
   - `emplace()` transitions from error to void
   - `swap()`
   - Equality
   - Monadic: `and_then` calls F(), `or_else` calls F(E&)
   - Dangling prevention on error
   - Shallow const on error

3. **Update `tests/beman/expected/CMakeLists.txt`**

## Procedure

1. Create branch from `main` (with Steps 4, 6, 8 merged)
2. Add `expected<void, E&>` specialization (or T, E& with void constraint)
3. Implement with pointer-only error storage
4. Implement constructors with dangling prevention
5. Implement observers (void value, reference error)
6. Implement monadic operations
7. Implement swap and equality
8. Write comprehensive tests
9. Run `make test` and `make lint`
10. Write final `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Completion

Step 10 is the final step. After completion, all specializations of
`expected` are implemented:

| Specialization | Value storage | Error storage | Step |
|----------------|---------------|---------------|------|
| `expected<T, E>` | T (owned) | E (owned) | 3, 5 |
| `expected<void, E>` | none | E (owned) | 4, 6 |
| `expected<T&, E>` | T* (pointer) | E (owned) | 7 |
| `expected<T, E&>` | T (owned) | E* (pointer) | 8 |
| `expected<T&, E&>` | T* (pointer) | E* (pointer) | 9 |
| `expected<void, E&>` | none | E* (pointer) | 10 |

Write a final `docs/plan/handoff-next.md` summarizing the complete state and
listing what work remains beyond the core implementation (paper writing,
additional tests, performance benchmarks, etc.).
