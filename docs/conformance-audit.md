# Conformance Audit: beman::expected vs std::expected (C++26)

Audit of the implementation on expected-over-references against
[expected.syn] through [expected.void.eq] in the C++26 working draft.

**Status**: All gaps resolved (Fixes 1–5 merged).

Legend:
- PASS = conformant
- EXT = non-standard extension (may or may not be desirable)
- FIXED = was a gap, now resolved (fix noted)

---

## 1. unexpected<E> [expected.unexpected]

### 1.1 Static assertions [expected.un.general] para 2

| Check | Status |
|-------|--------|
| E is an object type | PASS |
| E is not an array | PASS |
| E is not cv-qualified | PASS |
| E is not a specialization of unexpected | PASS |

### 1.2 Constructors [expected.un.cons]

| Constructor | Status | Notes |
|-------------|--------|-------|
| copy/move defaulted | PASS | |
| `unexpected(Err&&)` | PASS | Constraints correct |
| `unexpected(in_place_t, Args&&...)` | PASS | |
| `unexpected(in_place_t, initializer_list<U>, Args&&...)` | PASS | |

**EXT**: All non-defaulted constructors have conditional `noexcept(...)` specifications.
The standard does not specify noexcept on these; it only says "Throws: Any exception
thrown by the initialization of unex." This is a conforming strengthening of the
exception specification (permitted by [res.on.exception.handling]).

### 1.3 Observers [expected.un.obs]

All four `error()` overloads: **PASS**

### 1.4 Swap [expected.un.swap]

| Item | Status | Notes |
|------|--------|-------|
| Member swap noexcept spec | PASS | `noexcept(is_nothrow_swappable_v<E>)` |
| Member swap Mandates: `is_swappable_v<E>` | PASS | Fails naturally via swap(unex_, other.unex_) |
| Friend swap Constraints: `is_swappable_v<E>` | FIXED (Fix 5) | `requires std::is_swappable_v<E>` added |

### 1.5 Equality operator [expected.un.eq]

**PASS** — hidden friend, correct signature.

### 1.6 CTAD

`template<class E> unexpected(E) -> unexpected<E>;` **PASS**

---

## 2. bad_expected_access<void> [expected.bad.void]

| Item | Status |
|------|--------|
| Inherits from `std::exception` | PASS |
| Protected special members | PASS (all defaulted) |
| `what()` returns implementation-defined NTBS | PASS |

---

## 3. bad_expected_access<E> [expected.bad]

| Item | Status |
|------|--------|
| Inherits from `bad_expected_access<void>` | PASS |
| Constructor `bad_expected_access(E)` with `std::move` | PASS |
| `what()` override | PASS |
| All 4 `error()` overloads | PASS |

---

## 4. expected<T, E> Primary Template [expected.expected]

### 4.1 Type aliases and rebind

**PASS** — `value_type`, `error_type`, `unexpected_type`, `rebind` all correct.

### 4.2 Static assertions [expected.object.general] para 2-3

| Check | Status | Notes |
|-------|--------|-------|
| T is not a reference | PASS | |
| T is not void (use specialization) | N/A | Handled by partial specialization |
| T is not in_place_t | PASS | |
| T is not unexpect_t | PASS | |
| T is not an array | PASS | |
| T is not a specialization of unexpected | FIXED (Fix 4) | static_assert added |
| E is not a reference | PASS | |
| E is not void | PASS | |
| E is not an array | PASS | |

### 4.3 Constructors [expected.object.cons]

#### Default constructor

| Item | Status | Notes |
|------|--------|-------|
| Constraint: `is_default_constructible_v<T>` | PASS | |
| Value-initializes val | PASS | |
| noexcept spec | EXT | Standard doesn't specify noexcept here; conforming extension |

#### Copy constructor

| Item | Status | Notes |
|------|--------|-------|
| Defined as deleted unless both T and E are copy-constructible | PASS | Uses requires |
| Trivial when both T and E are trivially copy-constructible | FIXED (Fix 2) | `= default` path added |

#### Move constructor

| Item | Status | Notes |
|------|--------|-------|
| Constraints (move-constructible T and E) | PASS | |
| noexcept specification | PASS | |
| Trivial when both T and E are trivially move-constructible | FIXED (Fix 2) | `= default` path added |

#### Converting constructors from expected<U, G>

| Item | Status | Notes |
|------|--------|-------|
| `is_constructible_v<T, UF>` | PASS | |
| `is_constructible_v<E, GF>` | PASS | |
| if T is not cv bool, converts-from-any-cvref is false | FIXED (Fix 1) | Bool exemption added |
| `unexpected<E>` not constructible from expected<U,G> | PASS | |
| explicit condition | PASS | |

#### Value constructor `expected(U&& v)`

| Constraint | Status | Notes |
|------------|--------|-------|
| (23.1) `remove_cvref_t<U>` is not `in_place_t` | PASS | |
| (23.2) `remove_cvref_t<U>` is not `expected` | PASS | |
| (23.3) `remove_cvref_t<U>` is not `unexpect_t` | PASS | |
| (23.4) `remove_cvref_t<U>` is not a specialization of unexpected | FIXED (Fix 1) | Constraint added |
| (23.5) `is_constructible_v<T, U>` | PASS | |
| (23.6) if T is cv bool, `remove_cvref_t<U>` is not a specialization of expected | FIXED (Fix 1) | Constraint added |

#### unexpected<G> constructors

**PASS** — both const& and && overloads have correct constraints and explicit conditions.

#### in_place_t constructors

**PASS** — both variadic and initializer_list versions, correct constraints.

#### unexpect_t constructors

**PASS** — both variadic and initializer_list versions, correct constraints.

### 4.4 Destructor [expected.object.dtor]

| Item | Status |
|------|--------|
| Destroys val or unex based on has_value() | PASS |
| Trivial when both T and E are trivially destructible | PASS |

### 4.5 Assignment [expected.object.assign]

#### Copy assignment

| Item | Status | Notes |
|------|--------|-------|
| Constraints (copy-constructible, copy-assignable, nothrow-move disjunction) | PASS | |
| Uses reinit-expected correctly | PASS | |
| Trivial when all trivially copy-constructible/assignable/destructible | FIXED (Fix 2) | `= default` path added |

#### Move assignment

| Item | Status | Notes |
|------|--------|-------|
| Constraints (6.1-6.4): move-constructible/assignable T and E | PASS | |
| (6.5): `is_nothrow_move_constructible_v<T> \|\| is_nothrow_move_constructible_v<E>` | FIXED (Fix 1) | Added to requires clause |
| noexcept specification | PASS | |
| Trivial when all trivially move-constructible/assignable/destructible | FIXED (Fix 2) | `= default` path added |

#### Value assignment `operator=(U&&)`

| Item | Status | Notes |
|------|--------|-------|
| Default template argument `U = remove_cv_t<T>` | FIXED (Fix 1) | Changed from `U = T` |
| (11.1) `remove_cvref_t<U>` is not `expected` | PASS | |
| (11.2) `remove_cvref_t<U>` is not a specialization of unexpected | FIXED (Fix 1) | Was checking `unexpect_t` instead |
| (11.3) `is_constructible_v<T, U>` | PASS | |
| (11.4) `is_assignable_v<T&, U>` | PASS | |
| (11.5) nothrow disjunction | PASS | |

#### unexpected<G> assignment

**PASS** — both const& and && overloads correct.

#### emplace

**PASS** — both overloads (variadic and initializer_list), correct constraints and implementation.

### 4.6 Swap [expected.object.swap]

| Item | Status |
|------|--------|
| Constraints | PASS |
| noexcept specification | PASS |
| All 4 cases handled correctly | PASS |
| Friend swap | PASS |

### 4.7 Observers [expected.object.obs]

#### operator->

| Item | Status | Notes |
|------|--------|-------|
| Returns `addressof(val)` | PASS | |
| Hardened preconditions: `has_value()` is true | FIXED (Fix 5) | `BEMAN_EXPECTED_HARDENED` guard added |

#### operator*

| Item | Status | Notes |
|------|--------|-------|
| All 4 overloads (const&, &, const&&, &&) | PASS | |
| Hardened preconditions: `has_value()` is true | FIXED (Fix 5) | `BEMAN_EXPECTED_HARDENED` guard added |

#### operator bool / has_value()

**PASS**

#### value()

| Item | Status | Notes |
|------|--------|-------|
| Returns val / throws bad_expected_access | PASS | |
| Mandates (const&, &): `is_copy_constructible_v<E>` | FIXED (Fix 4) | static_assert added |
| Mandates (&&, const&&): `is_copy_constructible_v<E>` and `is_constructible_v<E, decltype(std::move(error()))>` | FIXED (Fix 4) | static_assert added |

#### error()

| Item | Status | Notes |
|------|--------|-------|
| All 4 overloads correct | PASS | |
| Hardened preconditions: `has_value()` is false | FIXED (Fix 5) | `BEMAN_EXPECTED_HARDENED` guard added |

#### value_or()

| Item | Status | Notes |
|------|--------|-------|
| Correct behavior | PASS | |
| Mandates (const&): `is_copy_constructible_v<T>` and `is_convertible_v<U, T>` | FIXED (Fix 4) | static_assert added |
| Mandates (&&): `is_move_constructible_v<T>` and `is_convertible_v<U, T>` | FIXED (Fix 4) | static_assert added |

#### error_or()

| Item | Status | Notes |
|------|--------|-------|
| Correct behavior | PASS | |
| Mandates (const&): `is_copy_constructible_v<E>` and `is_convertible_v<G, E>` | FIXED (Fix 4) | static_assert added |
| Mandates (&&): `is_move_constructible_v<E>` and `is_convertible_v<G, E>` | FIXED (Fix 4) | static_assert added |

### 4.8 Monadic operations [expected.object.monadic]

#### and_then (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Mandates: U is specialization of expected | PASS | static_assert |
| Mandates: `U::error_type` is E | PASS | static_assert |
| Constraints (&, const&): `is_constructible_v<E, decltype(error())>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>` | FIXED (Fix 3) | requires clause added |

#### or_else (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Mandates: G is specialization of expected | PASS | |
| Mandates: `G::value_type` is T | PASS | |
| Constraints (&, const&): `is_constructible_v<T, decltype((val))>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<T, decltype(std::move(val))>` | FIXED (Fix 3) | requires clause added |

#### transform (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Handles void U case | PASS | |
| Handles non-void U case | PASS | |
| Constraints (&, const&): `is_constructible_v<E, decltype(error())>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>` | FIXED (Fix 3) | requires clause added |
| Mandates: U is a valid value type for expected | FIXED (Fix 4) | static_assert added |
| Mandates (non-void U): declaration `U u(invoke(...))` is well-formed | PASS | Checked by is_constructible in return |

#### transform_error (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Correct return types | PASS | |
| Constraints (&, const&): `is_constructible_v<T, decltype((val))>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<T, decltype(std::move(val))>` | FIXED (Fix 3) | requires clause added |
| Mandates: G is valid template argument for unexpected | FIXED (Fix 4) | static_assert added |

### 4.9 Equality operators [expected.object.eq]

#### operator==(expected, expected<T2, E2>)

| Item | Status | Notes |
|------|--------|-------|
| requires `!is_void_v<T2>` | PASS | |
| Correct semantics | PASS | |

#### operator==(expected, T2)

| Item | Status | Notes |
|------|--------|-------|
| Constraints: T2 is not a specialization of expected | FIXED (Fix 1) | Constraint added |
| Constraints: `*x == v` is well-formed and convertible to bool | PASS | Uses `static_cast<bool>` |

#### operator==(expected, unexpected<E2>)

**PASS**

---

## 5. expected<void, E> Specialization [expected.void]

### 5.1 Static assertions

**PASS** — Checks E is not reference, void, array, cv-qualified, or unexpected specialization.

### 5.2 Constructors [expected.void.cons]

| Constructor | Status | Notes |
|-------------|--------|-------|
| Default `noexcept` | PASS | |
| Copy (trivial path) | PASS | `= default` when trivially copy constructible |
| Copy (non-trivial path) | PASS | |
| Move (trivial path) | PASS | `= default` when trivially move constructible |
| Move (non-trivial path) | PASS | |
| Converting from `expected<U, G>` (copy) | PASS | Correct constraints |
| Converting from `expected<U, G>` (move) | PASS | |
| From `unexpected<G>` (copy/move) | PASS | |
| `expected(in_place_t)` | PASS | |
| `expected(unexpect_t, Args...)` | PASS | |
| `expected(unexpect_t, initializer_list, Args...)` | PASS | |

### 5.3 Destructor [expected.void.dtor]

**PASS** — Trivial when E is trivially destructible.

### 5.4 Assignment [expected.void.assign]

| Item | Status | Notes |
|------|--------|-------|
| Copy assignment | PASS | Correct constraints and effects |
| Move assignment | PASS | |
| unexpected<G> assignment (copy/move) | PASS | |
| emplace() | PASS | |
| Trivial copy assignment | FIXED (Fix 2) | `= default` path added |
| Trivial move assignment | FIXED (Fix 2) | `= default` path added |

### 5.5 Swap [expected.void.swap]

**PASS** — Constraints, noexcept spec, and all cases correct.

### 5.6 Observers [expected.void.obs]

| Item | Status | Notes |
|------|--------|-------|
| `operator bool` / `has_value()` | PASS | |
| `operator*() const noexcept` | PASS | |
| `operator*()` Hardened preconditions | FIXED (Fix 5) | `BEMAN_EXPECTED_HARDENED` guard added |
| `value() const&` behavior | PASS | |
| `value() &&` behavior | PASS | |
| `value() const&` Mandates: `is_copy_constructible_v<E>` | FIXED (Fix 4) | static_assert added |
| `value() &&` Mandates: `is_copy_constructible_v<E>` and `is_move_constructible_v<E>` | FIXED (Fix 4) | static_assert added |
| `error()` all overloads | PASS | |
| `error()` Hardened preconditions | FIXED (Fix 5) | `BEMAN_EXPECTED_HARDENED` guard added |
| `error_or()` behavior | PASS | |
| `error_or()` Mandates | FIXED (Fix 4) | static_assert added |

### 5.7 Monadic operations [expected.void.monadic]

#### and_then

| Item | Status | Notes |
|------|--------|-------|
| Mandates | PASS | static_asserts present |
| Constraints (&, const&): `is_constructible_v<E, decltype(error())>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>` | FIXED (Fix 3) | requires clause added |

#### or_else

| Item | Status | Notes |
|------|--------|-------|
| Mandates: G is specialization of expected | PASS | |
| Mandates: `is_same_v<G::value_type, T>` | FIXED (Fix 5) | Changed from `is_void_v` to `is_same_v<..., T>` |
| No Constraints in standard | PASS | |

#### transform

| Item | Status | Notes |
|------|--------|-------|
| Handles void U and non-void U | PASS | |
| Constraints (&, const&): `is_constructible_v<E, decltype(error())>` | FIXED (Fix 3) | requires clause added |
| Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>` | FIXED (Fix 3) | requires clause added |
| Mandates: U is a valid value type | FIXED (Fix 4) | static_assert added |

#### transform_error

| Item | Status | Notes |
|------|--------|-------|
| Correct return types | PASS | |
| No Constraints in standard | PASS | |
| Mandates: G is valid template argument for unexpected | FIXED (Fix 4) | static_assert added |
| Returns `expected<T, G>` not `expected<void, G>` | FIXED (Fix 5) | Changed to use T |

### 5.8 Equality operators [expected.void.eq]

**PASS** — Both `operator==(expected<T2,E2>)` and `operator==(unexpected<E2>)` correct.

---

## Summary

All gaps identified in the original audit have been resolved across five fix branches:

- **Fix 1** (constraints): Converting ctor bool exemption, value ctor constraints 23.4/23.6,
  value assignment default arg and constraint 11.2, move assignment constraint 6.5,
  equality operator T2 constraint
- **Fix 2** (trivial SMFs): Trivial copy/move constructors and copy/move assignment for
  both primary and void specializations
- **Fix 3** (monadic constraints): requires clauses on all monadic operations
  (and_then, or_else, transform, transform_error) for both primary and void
- **Fix 4** (Mandates): static_asserts on value(), value_or(), error_or(), transform(),
  transform_error(), and T-is-not-unexpected class-level check
- **Fix 5** (preconditions + minor): Hardened precondition guards, unexpected friend swap
  constraint, void or_else is_same_v fix, void transform_error return type fix

### Extensions (not in standard, kept as conforming)

- Conditional `noexcept` on unexpected constructors
- Conditional `noexcept` on expected default constructor
