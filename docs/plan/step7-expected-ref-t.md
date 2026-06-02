# Step 7: Implement expected<T&, E> Reference Specialization

**Branch:** `step7-expected-ref-t`
**Depends on:** Steps 3, 5 (primary template with monadic ops)
**Read first:** `docs/plan/handoff-next.md` and `docs/plan/index.md`

---

## Goal

Implement `expected<T&, E>` — a specialization of `expected` where the value
type is a reference. This uses rebind semantics (not assign-through) per the
design adopted for `std::optional<T&>` in C++26 (P2988).

This is the core novel work of the proposal.

## Context for Executing Agent

### Design principles (from optional<T&>)

The reference implementation for `optional<T&>` at
`~/src/steve-downey/optional/main/include/beman/optional/optional.hpp`
(lines 1515-2119) provides the pattern to follow. Key design choices:

1. **Storage**: `T* value_ptr_ = nullptr` for the value reference, plus
   `union { E unex_; }` and `bool has_val_` for the error state
2. **Rebind semantics**: Assignment always rebinds the reference (changes what
   the reference points to), never assigns through the reference
3. **Shallow const**: `const expected<T&, E>` still allows mutation of T
   through `operator*()` — the const applies to the expected, not the referent
4. **Dangling prevention**: Constructors that would bind temporaries are
   `= delete` using `reference_constructs_from_temporary_v`
5. **No in-place construction for value**: `in_place_t` constructors for the
   value are not useful (just take a reference directly), but `unexpect_t`
   constructors for the error still exist

### Storage model

```cpp
template <class T, class E>
class expected<T&, E> {
    // ...
private:
    union {
        T* val_ptr_;    // active when has_val_ == true, points to referred object
        E unex_;        // active when has_val_ == false
    };
    bool has_val_;
};
```

Wait — this won't work cleanly because we need the pointer to be trivially
constructible but the union with E may not be. Better approach:

```cpp
template <class T, class E>
class expected<T&, E> {
private:
    T* val_ptr_ = nullptr;  // always present, null when in error state
    union {
        char dummy_;
        E unex_;
    };
    bool has_val_ = true;
};
```

Or simplest: since we have `has_val_` to discriminate, use the same union
pattern but with a pointer instead of a value:

```cpp
private:
    bool has_val_;
    union {
        T* val_;
        E unex_;
    };
```

This mirrors the primary template structure. When `has_val_` is true, `val_`
is a pointer to the referred object. When false, `unex_` is the error.

### Reference binding helper

Following optional<T&>, use a helper to bind references safely:

```cpp
template <class U>
constexpr void bind_ref(U&& u) {
    T& r(std::forward<U>(u));
    val_ = std::addressof(r);
}
```

### Constructors

**Value constructors** — take a `U` that can bind to `T&`:
```cpp
template<class U = T>
    requires (std::is_constructible_v<T&, U> &&
              !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
              !std::is_same_v<std::remove_cvref_t<U>, expected> &&
              !is_unexpected_v<std::remove_cvref_t<U>> &&
              !reference_constructs_from_temporary_v<T&, U>)
constexpr explicit(!std::is_convertible_v<U, T&>) expected(U&& u);
```

**Deleted constructor** — prevent binding temporaries:
```cpp
template<class U>
    requires (reference_constructs_from_temporary_v<T&, U>)
constexpr expected(U&&) = delete;
```

**Error constructors** — same as primary template:
```cpp
template<class G>
    constexpr explicit(!std::is_convertible_v<const G&, E>) expected(const unexpected<G>&);
template<class G>
    constexpr explicit(!std::is_convertible_v<G, E>) expected(unexpected<G>&&);
```

**unexpect_t constructors** — same as primary (constructs error in-place):
```cpp
template<class... Args>
    constexpr explicit expected(unexpect_t, Args&&...);
```

**No default constructor** — `expected<T&, E>` cannot be default-constructed
(there is no null reference).

### Assignment (rebind semantics)

Assignment to an expected holding a reference rebinds the reference:

```cpp
// From a value — rebind reference
template<class U = T>
constexpr expected& operator=(U&& u) {
    if (has_val_) {
        // Rebind: point to the new object
        bind_ref(std::forward<U>(u));
    } else {
        // Transition from error to value: destroy error, bind ref
        std::destroy_at(&unex_);
        bind_ref(std::forward<U>(u));
        has_val_ = true;
    }
    return *this;
}
```

### Observers

- `operator*()` returns `T&` (not `T*`), dereferencing the pointer
- `operator->()` returns `T*` (the stored pointer)
- `value()` returns `T&`, throws if no value
- `error()` — same as primary template
- `value_or(U&&)` returns `T` (by value, because the alternative needs a
  common type) — or could return `T&` if U converts to T&. Follow the
  standard wording carefully.

### Monadic operations

Same structure as primary but `value()` returns `T&`:
- `and_then(F)`: `invoke(f, **this)` — passes T& to F
- `transform(F)`: wraps `invoke(f, **this)` in expected<U, E>
- `or_else(F)`, `transform_error(F)`: same as primary

### reference_constructs_from_temporary_v

If `__reference_constructs_from_temporary` compiler built-in is not available
(it's a GCC 13+ / Clang 16+ extension), provide a conservative approximation.
See `optional.hpp` lines 1480-1511 for the portable fallback implementation
using `is_convertible_v` checks.

## Deliverables

1. **`include/beman/expected/expected.hpp`** — add `expected<T&, E>` partial
   specialization after the void specialization

2. **New test file: `tests/beman/expected/expected_ref.test.cpp`** — tests:
   - Construction from lvalue reference
   - Construction from `unexpected`
   - **Rebind semantics**: assign new reference, verify old referent unchanged
   - Copy/move construction (copies the pointer)
   - Converting construction from `expected<U&, G>`
   - Base-derived conversions: `expected<Base&, E>` from `expected<Derived&, E>`
   - `operator*()` returns `T&`, mutation through reference works
   - `operator->()` returns pointer
   - `value()` returns reference, throws on error
   - `error()` works correctly
   - `swap()` between two expected<T&, E>
   - Equality operators
   - Monadic: `and_then`, `transform`, `or_else`, `transform_error`
   - **Dangling prevention**: construction from temporary should not compile
     (negative test or static_assert)
   - **Shallow const**: `const expected<int&, E>` allows mutation through `*e`
   - **No default constructor**: `expected<int&, E> e;` should not compile

3. **Update `tests/beman/expected/CMakeLists.txt`**

## Procedure

1. Create branch from `expected-over-references`
2. Add `reference_constructs_from_temporary_v` concept (portable fallback) in
   a `detail` namespace within `beman::expected`
3. Add `expected<T&, E>` partial specialization with `requires std::is_lvalue_reference_v<...>`
   or directly as `template<class T, class E> class expected<T&, E>`
4. Implement storage (pointer + error union + bool)
5. Implement constructors with dangling prevention
6. Implement rebind assignment
7. Implement observers with reference semantics
8. Implement monadic operations
9. Implement swap and equality
10. Write comprehensive tests
11. Run `make test` and `make lint`
12. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 8

Step 7 done, next read `docs/plan/step8-expected-ref-e.md`.
`expected<T&, E>` is complete. Step 8 does the mirror — `expected<T, E&>`
where the error type is a reference.
