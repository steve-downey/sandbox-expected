# Step 9: Implement expected<T&, E&> Both-Reference Specialization

**Branch:** `step9-expected-ref-both`
**Depends on:** Steps 7, 8 (expected<T&, E> and expected<T, E&>)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement `expected<T&, E&>` — a specialization where both the value and
error types are references. Both sides use rebind semantics and pointer
storage.

## Context for Executing Agent

This combines the patterns from Steps 7 and 8. With both sides being
references, the storage is maximally simple — just two pointers and a bool.

### Storage model

```cpp
template <class T, class E>
class expected<T&, E&> {
private:
    bool has_val_;
    union {
        T* val_;
        E* unex_;
    };
};
```

Since both sides are pointers (same size, trivially constructible), the union
is simple. Alternatively, since pointers are trivial, you could even avoid the
union and just use two pointers, but the union approach is cleaner semantically
and matches the other specializations.

### Key properties

- **No default constructor**: `T&` cannot be default-constructed
- **Fully trivial**: copy, move, destruction are all trivial (just copying
  pointers and a bool)
- **Both rebind**: assignment from value rebinds value pointer, assignment
  from unexpected rebinds error pointer
- **Shallow const on both sides**: `const expected<int&, err&>` allows
  mutation through both `*e` and `e.error()`
- **Dangling prevention on both sides**: constructors that would bind
  temporaries to either T& or E& are deleted

### Constructors

Combine the value-side constructors from Step 7 with the error-side
constructors from Step 8:

- From value `U&&` — binds T& (with dangling prevention)
- From `unexpected<G>` — binds E& (with dangling prevention)
- `(unexpect_t, Args...)` — binds E&
- Copy/move — copies pointers
- Converting from `expected<U&, G&>`

### Assignment

- From value `U&&` — rebinds T* (destroying E if transitioning from error)
- From `unexpected<G>` — rebinds E* (destroying T* conceptually if transitioning)
- Copy/move — standard semantics

Since both sides are pointers, state transitions are trivial:
```cpp
// Transition from error to value: just set the pointer and flip the bool
// No destroy_at needed since pointers have trivial destruction
```

### Observers

- `operator*()` → `T&`
- `operator->()` → `T*`
- `value()` → `T&` (throws on error)
- `error()` → `E&` (UB if has value)
- `value_or(U&&)` → `T` by value
- `error_or(G&&)` → `E` by value

### Monadic operations

Same structure as primary but with reference semantics on both sides:
- `and_then(F)` passes `T&` to F, propagates `E&` on error
- `or_else(F)` passes `E&` to F, propagates `T&` on value
- `transform(F)` passes `T&` to F
- `transform_error(F)` passes `E&` to F

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add `expected<T&, E&>` partial
   specialization

2. **New test file: `tests/beman/expected/expected_ref_both.test.cpp`** — tests:
   - Construction from lvalue reference (value)
   - Construction from error reference via unexpected
   - **Value rebind**: assign new value reference
   - **Error rebind**: assign new error reference
   - Both rebind in sequence
   - Copy/move construction (trivial)
   - `operator*()` returns T&
   - `error()` returns E&
   - Mutation through both references
   - `value()` throws on error
   - `swap()`
   - Equality
   - Monadic operations with reference semantics on both sides
   - Dangling prevention: both T and E temporaries rejected
   - Shallow const on both sides
   - No default constructor
   - Triviality: verify trivially copyable/movable

3. **Update `tests/beman/expected/CMakeLists.txt`**

## Procedure

1. Create branch from `main` (with Steps 7-8 merged)
2. Add `expected<T&, E&>` partial specialization
3. Implement with dual-pointer storage
4. Implement constructors (combine patterns from Steps 7 and 8)
5. Implement trivial assignment with rebind on both sides
6. Implement observers with reference semantics
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

## Handoff to Step 10

Step 9 done, next read `docs/plan/step10-expected-void-ref-e.md`.
`expected<T&, E&>` is complete. Step 10 is the final specialization:
`expected<void, E&>`.
