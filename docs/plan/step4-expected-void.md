# Step 4: Implement expected<void, E> Specialization

**Branch:** `step4-expected-void`
**Depends on:** Steps 1-2 (unexpected<E>, bad_expected_access)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement the partial specialization `expected<T, E> requires is_void_v<T>` per
[expected.void]. This handles the case where there is no value — only success
(void) or error.

## Context for Executing Agent

The specification is in `include/beman/expected/expected.hpp` lines 156-249 as
comments. This specialization differs from the primary template:

- No value storage, no `operator->`, no `value_or()`
- `operator*()` returns void
- `value()` just throws if no value (returns void otherwise)
- `emplace()` takes no arguments
- Still has error storage, `error()`, `error_or()`

### Key differences from primary template

| Aspect | Primary `expected<T, E>` | Void `expected<void, E>` |
|--------|--------------------------|--------------------------|
| Storage | `union { T val_; E unex_; }` | `union { E unex_; }` + `bool` |
| Default ctor | Value-initializes T | Sets has_val_ = true |
| `operator*()` | Returns T& | Returns void |
| `operator->()` | Returns T* | Not present |
| `value()` | Returns T& or throws | Returns void or throws |
| `value_or()` | Returns T | Not present |
| `emplace()` | Takes Args... | Takes no args |
| From-value ctor | `expected(U&&)` | Not present |
| Value assignment | `operator=(U&&)` | Not present |
| `in_place_t` ctor | `(in_place_t, Args...)` | `(in_place_t)` only |

### Constructors [expected.void.cons]

- Default: `has_val_ = true`
- Copy, move
- Converting from `expected<U, G>` (const& and &&) where `is_void_v<U>`
- From `unexpected<G>` (const& and &&)
- `(in_place_t)` — explicit, no args
- `(unexpect_t, Args...)`, `(unexpect_t, initializer_list<U>, Args...)`

### Storage

```cpp
bool has_val_;
union {
    E unex_;
};
```

When `has_val_` is true, the union member is not active (void state).

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add the void specialization
   after the primary template

2. **New test file: `tests/beman/expected/expected_void.test.cpp`** — tests:
   - Default construction (has_value() == true)
   - Construction from unexpected
   - In-place construction (`in_place_t`)
   - Copy/move construction
   - Converting from `expected<void, G>`
   - Assignment (copy, move, from unexpected)
   - `emplace()` — transitions from error to value state
   - `swap()`
   - `operator*()` — compiles (returns void)
   - `value()` — no-op when has value, throws when no value
   - `error()` — all ref-qualified overloads
   - `error_or()`
   - Equality: `expected<void,E> == expected<void,E2>`,
     `expected<void,E> == unexpected<E2>`

3. **Update `tests/beman/expected/CMakeLists.txt`** — add new test file

## Procedure

1. Create branch `step4-expected-void` from `main` (with Steps 1-2 merged)
2. In `expected.hpp`, after the primary template, add the partial specialization:
   ```cpp
   template <class T, class E>
       requires std::is_void_v<T>
   class expected<T, E> { ... };
   ```
3. Implement with simpler storage (no value union member)
4. Implement all constructors, destructor, assignment
5. Implement observers (void value semantics)
6. Implement equality
7. No monadic operations yet (Step 6)
8. Create `expected_void.test.cpp` and register in CMakeLists
9. Run `make test` and `make lint`
10. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 5

Step 4 done, next read `docs/plan/step5-expected-primary-monadic.md` or
`docs/plan/step6-expected-void-monadic.md` depending on which is next.
