# Step 3: Implement expected<T, E> Primary Template

**Branch:** `step3-expected-primary`
**Depends on:** Steps 1-2 (unexpected<E>, bad_expected_access<E>)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement the `expected<T, E>` primary template per [expected.object] — the
non-reference, non-void case. This is the largest single step in the plan.
Monadic operations (and_then, or_else, transform, transform_error) are
deferred to Step 5.

## Context for Executing Agent

The file `include/beman/expected/expected.hpp` has the full specification as
comments (lines 37-153). Steps 1-2 have already implemented `unexpected<E>`
and `bad_expected_access<E>`, so those types are available.

### Key files

- `include/beman/expected/expected.hpp` — implement here (keep the spec
  comments, add real code in the namespace)
- `include/beman/expected/unexpected.hpp` — provides `unexpected<E>`, `unexpect_t`
- `include/beman/expected/bad_expected_access.hpp` — provides exception types
- `tests/beman/expected/expected.test.cpp` — expand from breathing test

### Standard reference (non-monadic subset)

The class needs:

**Types:**
- `value_type = T`, `error_type = E`, `unexpected_type = unexpected<E>`
- `template<class U> using rebind = expected<U, error_type>`

**Constructors** [expected.object.cons]:
- Default: value-initializes `T`
- Copy, move
- Converting from `expected<U, G>` (const& and &&)
- From value `U&&` (implicit/explicit based on convertibility)
- From `unexpected<G>` (const& and &&)
- In-place for value: `(in_place_t, Args...)`
- In-place for value with init-list: `(in_place_t, initializer_list<U>, Args...)`
- In-place for error: `(unexpect_t, Args...)`
- In-place for error with init-list: `(unexpect_t, initializer_list<U>, Args...)`

**Destructor** [expected.object.dtor]:
- Destroys contained value or error

**Assignment** [expected.object.assign]:
- Copy, move
- From value `U&&`
- From `unexpected<G>` (const& and &&)

**Emplace** [expected.object.assign]:
- `emplace(Args&&...)` — destroys current, constructs value in-place
- `emplace(initializer_list<U>, Args&&...)` — same with init-list

**Swap** [expected.object.swap]:
- Member `swap(expected&)`
- Friend `swap(expected&, expected&)`

**Observers** [expected.object.obs]:
- `operator->()` — const and non-const
- `operator*()` — 4 ref-qualified overloads
- `operator bool()` / `has_value()`
- `value()` — 4 ref-qualified, throws `bad_expected_access<E>` if no value
- `error()` — 4 ref-qualified
- `value_or(U&&)` — const& and &&
- `error_or(G&&)` — const& and &&

**Equality** [expected.object.eq]:
- `operator==(const expected<T2, E2>&)` (when T2 is not void)
- `operator==(const T2&)` (comparison with value)
- `operator==(const unexpected<E2>&)` (comparison with unexpected)

### Storage

Use a discriminated union:
```cpp
bool has_val_;
union {
    T val_;
    E unex_;
};
```

### Constraints

Many constructors are constrained with `requires` clauses and use
`explicit(see below)` — the explicit-ness depends on whether conversions
are implicit. Follow the standard wording carefully. Key constraints:

- `T` must not be a reference, array, function, `in_place_t`, `unexpect_t`,
  or a specialization of `unexpected`
- `E` must not be a reference, array, function, void, or a specialization
  of `unexpected`
- Converting constructors are explicit if the source types are not
  implicitly convertible

### Exception safety in assignment

Assignment to `expected` is complex because the old value/error must be
destroyed and the new one constructed. The standard specifies a careful
protocol using `reinit_expected` helper logic:
- When transitioning between value and error states, if the new construction
  can throw, store the old value temporarily, destroy it, try to construct
  the new one, and if that throws, reconstruct the old value from the backup.

For this step, you may use a simpler approach if the full exception-safety
protocol is too complex: use `std::construct_at` / `std::destroy_at` with
basic try/catch. The key invariant is: after assignment, the expected is in a
valid state (either has value or has error, never empty).

## Deliverables

1. **`include/beman/expected/expected.hpp`** — full primary template:
   - All constructors, destructor, assignment operators
   - emplace, swap, all observers
   - Equality operators as hidden friends
   - No monadic operations yet (Step 5)

2. **`tests/beman/expected/expected.test.cpp`** — comprehensive tests:
   - Default construction (has_value() == true)
   - Construction from value
   - Construction from unexpected
   - In-place construction (value and error)
   - Copy and move construction
   - Converting construction from expected<U, G>
   - Copy and move assignment
   - Assignment from value
   - Assignment from unexpected
   - emplace()
   - swap()
   - All observer overloads (operator*, operator->, value(), error())
   - value() throws bad_expected_access when no value
   - value_or() and error_or()
   - Equality operators (expected==expected, expected==value, expected==unexpected)
   - Verify static_assert prevents reference types for T and E

## Procedure

1. Create branch `step3-expected-primary` from `main` (with Steps 1-2 merged)
2. Add includes to `expected.hpp`: `<type_traits>`, `<utility>`, `<memory>`,
   `<initializer_list>`, `<functional>`
3. Define the primary template class declaration with all members
4. Implement constructors out-of-line after the class body
5. Implement destructor (destroy val_ or unex_ based on has_val_)
6. Implement assignment operators with state transition logic
7. Implement emplace
8. Implement swap (handle all 4 state combinations)
9. Implement observers
10. Implement equality operators as hidden friends
11. Add static_asserts at top of class body:
    - `static_assert(!std::is_reference_v<T>)` (for now — Step 7 lifts this)
    - `static_assert(!std::is_reference_v<E>)` (for now — Step 8 lifts this)
12. Write comprehensive tests
13. Run `make test` and `make lint`
14. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 4

Step 3 done, next read `docs/plan/step4-expected-void.md`.
The primary template is now available. Step 4 adds the void specialization,
Step 5 adds monadic operations to this template.
