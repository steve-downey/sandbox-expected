# Conformance Audit: beman::expected vs std::expected (C++26)

Audit of the implementation on expected-over-references (after Step 6 merge)
against [expected.syn] through [expected.void.eq] in the C++26 working draft.

Legend:
- PASS = conformant
- GAP = missing or incorrect relative to standard
- EXT = non-standard extension (may or may not be desirable)
- MINOR = pedantic/low-impact deviation

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
| Member swap Mandates: `is_swappable_v<E>` | MINOR | No explicit `static_assert`; would fail naturally but with poor diagnostic |
| Friend swap Constraints: `is_swappable_v<E>` | **GAP** | Missing `requires is_swappable_v<E>`. Standard says *Constraints* (SFINAE), but implementation has no requires clause — participates in overload resolution unconditionally |

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
| **T is not a specialization of unexpected** | **GAP** | Standard says T must not be a specialization of unexpected; no static_assert |
| E is not a reference | PASS | |
| E is not void | PASS | |
| E is not an array | PASS | |
| E is not cv-qualified | MINOR | Not checked directly; `unexpected<E>` typedef would catch it |
| E is not a specialization of unexpected | MINOR | Not checked directly; `unexpected<E>` typedef would catch it |

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
| **Trivial when both T and E are trivially copy-constructible** | **GAP** | No `= default` path; always uses construct_at |

#### Move constructor

| Item | Status | Notes |
|------|--------|-------|
| Constraints (move-constructible T and E) | PASS | |
| noexcept specification | PASS | |
| **Trivial when both T and E are trivially move-constructible** | **GAP** | No `= default` path |

#### Converting constructors from expected<U, G>

| Item | Status | Notes |
|------|--------|-------|
| `is_constructible_v<T, UF>` | PASS | |
| `is_constructible_v<E, GF>` | PASS | |
| **if T is not cv bool, converts-from-any-cvref is false** | **GAP** | Always applied unconditionally. When T=bool, rejects valid conversions because `expected<U,G>` has `operator bool()` |
| `unexpected<E>` not constructible from expected<U,G> | PASS | |
| explicit condition | PASS | |

#### Value constructor `expected(U&& v)`

| Constraint | Status | Notes |
|------------|--------|-------|
| (23.1) `remove_cvref_t<U>` is not `in_place_t` | PASS | |
| (23.2) `remove_cvref_t<U>` is not `expected` | PASS | |
| (23.3) `remove_cvref_t<U>` is not `unexpect_t` | PASS | |
| **(23.4) `remove_cvref_t<U>` is not a specialization of unexpected** | **GAP** | Not checked |
| (23.5) `is_constructible_v<T, U>` | PASS | |
| **(23.6) if T is cv bool, `remove_cvref_t<U>` is not a specialization of expected** | **GAP** | Not checked |

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
| **Trivial when all trivially copy-constructible/assignable/destructible** | **GAP** | No trivial path |

#### Move assignment

| Item | Status | Notes |
|------|--------|-------|
| Constraints (6.1-6.4): move-constructible/assignable T and E | PASS | |
| **(6.5): `is_nothrow_move_constructible_v<T> \|\| is_nothrow_move_constructible_v<E>`** | **GAP** | Missing from requires clause |
| noexcept specification | PASS | |
| **Trivial when all trivially move-constructible/assignable/destructible** | **GAP** | No trivial path |

#### Value assignment `operator=(U&&)`

| Item | Status | Notes |
|------|--------|-------|
| **Default template argument** | **GAP** | Uses `U = T`; standard says `U = remove_cv_t<T>` |
| (11.1) `remove_cvref_t<U>` is not `expected` | PASS | |
| **(11.2) `remove_cvref_t<U>` is not a specialization of unexpected** | **GAP** | Checks `unexpect_t` instead of unexpected specialization |
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
| **Hardened preconditions: `has_value()` is true** | **GAP** | No precondition check |

#### operator*

| Item | Status | Notes |
|------|--------|-------|
| All 4 overloads (const&, &, const&&, &&) | PASS | |
| **Hardened preconditions: `has_value()` is true** | **GAP** | No precondition check |

#### operator bool / has_value()

**PASS**

#### value()

| Item | Status | Notes |
|------|--------|-------|
| Returns val / throws bad_expected_access | PASS | |
| **Mandates (const&, &): `is_copy_constructible_v<E>`** | **GAP** | No static_assert |
| **Mandates (&&, const&&): `is_copy_constructible_v<E>` and `is_constructible_v<E, decltype(std::move(error()))>`** | **GAP** | No static_assert |

#### error()

| Item | Status | Notes |
|------|--------|-------|
| All 4 overloads correct | PASS | |
| **Hardened preconditions: `has_value()` is false** | **GAP** | No precondition check |

#### value_or()

| Item | Status | Notes |
|------|--------|-------|
| Correct behavior | PASS | |
| **Mandates (const&): `is_copy_constructible_v<T>` and `is_convertible_v<U, T>`** | **GAP** | No static_assert |
| **Mandates (&&): `is_move_constructible_v<T>` and `is_convertible_v<U, T>`** | **GAP** | No static_assert |

#### error_or()

| Item | Status | Notes |
|------|--------|-------|
| Correct behavior | PASS | |
| **Mandates (const&): `is_copy_constructible_v<E>` and `is_convertible_v<G, E>`** | **GAP** | No static_assert |
| **Mandates (&&): `is_move_constructible_v<E>` and `is_convertible_v<G, E>`** | **GAP** | No static_assert |

### 4.8 Monadic operations [expected.object.monadic]

#### and_then (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Mandates: U is specialization of expected | PASS | static_assert |
| Mandates: `U::error_type` is E | PASS | static_assert |
| **Constraints (&, const&): `is_constructible_v<E, decltype(error())>`** | **GAP** | No requires clause |
| **Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>`** | **GAP** | No requires clause |

#### or_else (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Mandates: G is specialization of expected | PASS | |
| Mandates: `G::value_type` is T | PASS | |
| **Constraints (&, const&): `is_constructible_v<T, decltype((val))>`** | **GAP** | No requires clause |
| **Constraints (&&, const&&): `is_constructible_v<T, decltype(std::move(val))>`** | **GAP** | No requires clause |

#### transform (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Handles void U case | PASS | |
| Handles non-void U case | PASS | |
| **Constraints (&, const&): `is_constructible_v<E, decltype(error())>`** | **GAP** | No requires clause |
| **Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>`** | **GAP** | No requires clause |
| **Mandates: U is a valid value type for expected** | **GAP** | No static_assert |
| **Mandates (non-void U): declaration `U u(invoke(...))` is well-formed** | **GAP** | No static_assert |

#### transform_error (all 4 overloads)

| Item | Status | Notes |
|------|--------|-------|
| Correct return types | PASS | |
| **Constraints (&, const&): `is_constructible_v<T, decltype((val))>`** | **GAP** | No requires clause |
| **Constraints (&&, const&&): `is_constructible_v<T, decltype(std::move(val))>`** | **GAP** | No requires clause |
| **Mandates: G is valid template argument for unexpected** | **GAP** | No static_assert |
| **Mandates: declaration `G g(invoke(...))` is well-formed** | **GAP** | No static_assert |

### 4.9 Equality operators [expected.object.eq]

#### operator==(expected, expected<T2, E2>)

| Item | Status | Notes |
|------|--------|-------|
| requires `!is_void_v<T2>` | PASS | |
| Correct semantics | PASS | |

#### operator==(expected, T2)

| Item | Status | Notes |
|------|--------|-------|
| **Constraints: T2 is not a specialization of expected** | **GAP** | No constraint |
| **Constraints: `*x == v` is well-formed and convertible to bool** | **GAP** | No constraint (hard error instead of SFINAE) |

#### operator==(expected, unexpected<E2>)

**PASS**

---

## 5. expected<void, E> Specialization [expected.void]

### 5.1 Static assertions

**PASS** — More thorough than primary template; checks E is not reference, void, array,
cv-qualified, or unexpected specialization.

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
| **Trivial copy assignment** | **GAP** | Standard says trivial when E's copy/assign/dtor are all trivial |
| **Trivial move assignment** | **GAP** | Same for move |

### 5.5 Swap [expected.void.swap]

**PASS** — Constraints, noexcept spec, and all cases correct.

### 5.6 Observers [expected.void.obs]

| Item | Status | Notes |
|------|--------|-------|
| `operator bool` / `has_value()` | PASS | |
| `operator*() const noexcept` | PASS | |
| **`operator*()` Hardened preconditions** | **GAP** | No check |
| `value() const&` behavior | PASS | |
| `value() &&` behavior | PASS | |
| **`value() const&` Mandates: `is_copy_constructible_v<E>`** | **GAP** | No static_assert |
| **`value() &&` Mandates: `is_copy_constructible_v<E>` and `is_move_constructible_v<E>`** | **GAP** | No static_assert |
| `error()` all overloads | PASS | |
| **`error()` Hardened preconditions** | **GAP** | No check |
| `error_or()` behavior | PASS | |
| **`error_or()` Mandates** | **GAP** | No static_assert |

### 5.7 Monadic operations [expected.void.monadic]

#### and_then

| Item | Status | Notes |
|------|--------|-------|
| Mandates | PASS | static_asserts present |
| **Constraints (&, const&): `is_constructible_v<E, decltype(error())>`** | **GAP** | No requires |
| **Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>`** | **GAP** | No requires |

#### or_else

| Item | Status | Notes |
|------|--------|-------|
| Mandates: G is specialization of expected | PASS | |
| Mandates: `is_same_v<G::value_type, T>` | MINOR | Checks `is_void_v` instead of `is_same_v<..., T>` — equivalent when T is exactly `void`, differs for `const void` |
| No Constraints in standard | PASS | |

#### transform

| Item | Status | Notes |
|------|--------|-------|
| Handles void U and non-void U | PASS | |
| **Constraints (&, const&): `is_constructible_v<E, decltype(error())>`** | **GAP** | No requires |
| **Constraints (&&, const&&): `is_constructible_v<E, decltype(std::move(error()))>`** | **GAP** | No requires |
| **Mandates: U is a valid value type** | **GAP** | No static_assert |
| **Mandates (non-void U): declaration well-formed** | **GAP** | No static_assert |

#### transform_error

| Item | Status | Notes |
|------|--------|-------|
| Correct return types | PASS | |
| No Constraints in standard | PASS | |
| **Mandates: G is valid template argument for unexpected** | **GAP** | No static_assert |
| **Mandates: declaration `G g(invoke(...))` well-formed** | **GAP** | No static_assert |
| MINOR: Returns `expected<void, G>` not `expected<T, G>` | MINOR | Differs if T is `const void` etc. |

### 5.8 Equality operators [expected.void.eq]

**PASS** — Both `operator==(expected<T2,E2>)` and `operator==(unexpected<E2>)` correct.

---

## Summary of Gaps

### Critical (affects overload resolution / rejects valid programs)

1. **Primary template: converts-from-any-cvref not exempted for T=bool**
   - Converting constructors from `expected<U,G>` reject valid programs when T is `bool`
   - `expected<bool, E>` cannot be properly constructed from other `expected<U, G>` values

2. **Value constructor: missing constraint (23.4)**
   - `remove_cvref_t<U>` must not be a specialization of `unexpected`
   - Without this, `unexpected<G>` values could match the value constructor

3. **Value constructor: missing constraint (23.6)**
   - If T is cv bool, `remove_cvref_t<U>` must not be a specialization of `expected`
   - Without this, nested expected values could match the bool value constructor

4. **Value assignment: wrong default template argument**
   - Uses `U = T` instead of `U = remove_cv_t<T>`

5. **Value assignment: checks unexpect_t instead of unexpected specialization**
   - Constraint (11.2) should reject `unexpected<X>` specializations, not `unexpect_t`

6. **Move assignment: missing nothrow disjunction constraint (6.5)**
   - Standard requires `is_nothrow_move_constructible_v<T> || is_nothrow_move_constructible_v<E>`

7. **Equality operator==(expected, T2): missing constraints**
   - T2 must not be a specialization of expected (SFINAE, not hard error)
   - `*x == v` must be well-formed and convertible to bool

### Structural (affects ABI/performance, not correctness)

8. **Primary template: missing trivial copy constructor**
9. **Primary template: missing trivial move constructor**
10. **Primary template: missing trivial copy assignment**
11. **Primary template: missing trivial move assignment**
12. **Void specialization: missing trivial copy/move assignment**

### Missing Constraints on Monadic Operations (SFINAE impact)

All 16 monadic operation overloads on the primary template and 8 on the void
specialization (and_then, transform) are missing their `requires` clauses. The standard
specifies these as *Constraints*, meaning they should participate in SFINAE. Without
them, calling a monadic operation when the constraint isn't met produces a hard error
instead of graceful overload failure.

13. **and_then**: needs `is_constructible_v<E, decltype(error())>` (or move variant)
14. **or_else**: needs `is_constructible_v<T, decltype((val))>` (or move variant)
15. **transform**: needs `is_constructible_v<E, decltype(error())>` (or move variant)
16. **transform_error**: needs `is_constructible_v<T, decltype((val))>` (or move variant)

### Missing Mandates (diagnostic quality)

17. **value() all overloads**: missing `is_copy_constructible_v<E>` (and move for rvalue overloads)
18. **value_or()**: missing `is_copy/move_constructible_v<T>` and `is_convertible_v<U, T>`
19. **error_or()**: missing `is_copy/move_constructible_v<E>` and `is_convertible_v<G, E>`
20. **transform**: missing "U is a valid value type" and well-formed declaration check
21. **transform_error**: missing "G is valid template argument for unexpected" and well-formed declaration check
22. **T is not a specialization of unexpected** (static_assert on primary template)

### Missing Hardened Preconditions

23. **operator->()**: `has_value()` is true
24. **operator*()**: `has_value()` is true
25. **error()**: `has_value()` is false

(Project memory notes these should be guarded by `BEMAN_EXPECTED_HARDENED`.)

### Minor / Low Priority

26. **unexpected friend swap**: missing `requires is_swappable_v<E>` constraint
27. **void or_else**: checks `is_void_v<G::value_type>` not `is_same_v<G::value_type, T>`
28. **void transform_error**: returns `expected<void, G>` not `expected<T, G>`

### Extensions (not in standard, may want to keep or remove)

- Conditional `noexcept` on unexpected constructors (conforming extension)
- Conditional `noexcept` on expected default constructor (conforming extension)

---

## Recommended Priority for Fixes

**Before reference specialization work (Steps 7-10):**

1. Fix critical constraint gaps (items 1-7) — these affect program correctness
2. Add trivial special members (items 8-12) — affects ABI/codegen quality
3. Add monadic operation constraints (items 13-16) — SFINAE correctness

**Can be done incrementally:**

4. Add Mandates static_asserts (items 17-22) — diagnostic quality
5. Add hardened preconditions (items 23-25) — runtime safety
6. Fix minor deviations (items 26-28) — pedantic conformance
