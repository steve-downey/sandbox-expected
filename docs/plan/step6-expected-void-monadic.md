# Step 6: Monadic Operations for expected<void, E>

**Branch:** `step6-expected-void-monadic`
**Depends on:** Step 4 (expected<void, E> specialization)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Add the four monadic operations to the `expected<void, E>` partial
specialization per [expected.void.monadic].

## Context for Executing Agent

Step 4 implemented the void specialization without monadic ops. The void case
differs from the primary template because there is no value to pass to the
callable — `and_then` and `transform` call F with no arguments.

### Differences from primary template monadic ops

| Operation | Primary `expected<T, E>` | Void `expected<void, E>` |
|-----------|--------------------------|--------------------------|
| `and_then(F)` has value | `invoke(f, value())` | `invoke(f)` (no arg) |
| `or_else(F)` has value | return `expected<T,G>(in_place, value())` | return `expected<void,G>()` |
| `transform(F)` has value | `invoke(f, value())` | `invoke(f)` (no arg) |
| `transform_error(F)` has value | return `expected<T,G>(in_place, value())` | return `expected<void,G>()` |

### Constraints per [expected.void.monadic]

For `and_then`:
- `F` is invocable with no arguments: `invoke(f)`
- Return type `U` must be `expected<U_val, E>` where `U::error_type` is `E`

For `or_else`:
- Same as primary (calls `f(error())`)
- Return type `G` must be `expected<void, G_err>`

For `transform`:
- `F` is invocable with no arguments
- If return type is void: `invoke(f)` then return `expected<void, E>()`
- Otherwise: return `expected<U, E>(invoke(f))`

For `transform_error`:
- Same as primary (calls `f(error())`)
- Returns `expected<void, G>`

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add 16 monadic member functions
   to the void specialization

2. **New test file: `tests/beman/expected/expected_void_monadic.test.cpp`** — tests:
   - `and_then`: value state invokes F with no args
   - `and_then`: error state short-circuits
   - `or_else`: error state invokes F with error
   - `or_else`: value state short-circuits
   - `transform`: value state invokes F with no args, wraps result
   - `transform`: value state with void-returning F
   - `transform_error`: preserves void value, transforms error
   - Chaining combinations

3. **Update `tests/beman/expected/CMakeLists.txt`**

## Procedure

1. Create branch `step6-expected-void-monadic` from `main` (with Step 4 merged)
2. Add 16 monadic declarations to the void specialization class body
3. Implement each out-of-line
4. Create test file
5. Register in CMakeLists
6. Run `make test` and `make lint`
7. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 7

Step 6 done, next read `docs/plan/step7-expected-ref-t.md`.
Both the primary and void templates now have full monadic support.
The reference specializations build on these foundations.
