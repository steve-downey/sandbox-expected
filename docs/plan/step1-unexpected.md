# Step 1: Implement unexpected<E>

**Branch:** `step1-unexpected`
**Depends on:** None (first step)
**Read first:** `docs/plan/handoff.md` and `docs/plan/index.md`

---

## Goal

Implement the `unexpected<E>` class template per [expected.unexpected] in the
C++ standard. This is the wrapper type that holds error values for storage in
`expected` objects.

## Context for Executing Agent

The file `include/beman/expected/unexpected.hpp` already exists with the full
specification as a comment block (lines 6-49). The namespace `beman::expected`
exists but is empty. Your job is to write the actual class template inside
that namespace.

### Key files

- `include/beman/expected/unexpected.hpp` — implement here
- `tests/beman/expected/unexpected.test.cpp` — expand from breathing test
- `tests/beman/expected/CMakeLists.txt` — may need updates for new test files

### Standard reference

Section [expected.un.general] specifies:

```
namespace std {
  template<class E>
  class unexpected {
  public:
    constexpr unexpected(const unexpected&) = default;
    constexpr unexpected(unexpected&&) = default;
    template<class Err = E>
      constexpr explicit unexpected(Err&&);
    template<class... Args>
      constexpr explicit unexpected(in_place_t, Args&&...);
    template<class U, class... Args>
      constexpr explicit unexpected(in_place_t, initializer_list<U>, Args&&...);

    constexpr unexpected& operator=(const unexpected&) = default;
    constexpr unexpected& operator=(unexpected&&) = default;

    constexpr const E& error() const & noexcept;
    constexpr E& error() & noexcept;
    constexpr const E&& error() const && noexcept;
    constexpr E&& error() && noexcept;

    constexpr void swap(unexpected& other) noexcept(see below);

    template<class E2>
      friend constexpr bool operator==(const unexpected&, const unexpected<E2>&);

    friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)));

  private:
    E unex;  // exposition only
  };

  template<class E> unexpected(E) -> unexpected<E>;
}
```

### Constraints on the converting constructor

Per [expected.un.cons]:
- `template<class Err = E> constexpr explicit unexpected(Err&&)`:
  - Constraint: `!is_same_v<remove_cvref_t<Err>, unexpected>` and
    `!is_same_v<remove_cvref_t<Err>, in_place_t>`
  - Mandates: `is_constructible_v<E, Err>`

### Constraints on E

Per [expected.un.general] para 2:
- `E` shall be a valid value type for `unexpected`
- `E` shall not be a non-object type, an array type, a specialization of
  `unexpected`, or a cv-qualified type

## Deliverables

1. **`include/beman/expected/unexpected.hpp`** — full implementation of:
   - `unexpect_t` tag type and `unexpect` inline constexpr instance
   - `unexpected<E>` class template with all members listed above
   - CTAD deduction guide: `template<class E> unexpected(E) -> unexpected<E>`

2. **`tests/beman/expected/unexpected.test.cpp`** — comprehensive tests:
   - Default-construct from value: `unexpected(42)`, `unexpected(std::string("err"))`
   - In-place construction: `unexpected<std::string>(std::in_place, "hello")`
   - In-place with initializer_list: `unexpected<std::vector<int>>(std::in_place, {1,2,3})`
   - Copy and move construction
   - `error()` observers in all 4 ref-qualified overloads
   - `swap()` member and friend
   - `operator==` with same and different E types
   - CTAD: `unexpected u(42);` deduces `unexpected<int>`
   - Verify constraint: `unexpected` should not be constructible from
     another `unexpected` via the converting constructor (it should use copy/move)

## Procedure

1. Create branch `step1-unexpected` from `main`
2. In `unexpected.hpp`, add includes: `<type_traits>`, `<utility>`, `<initializer_list>`
3. Define `unexpect_t` and `unexpect` in `beman::expected` namespace
4. Define `unexpected<E>` class template with private `E unex_;` member
5. Implement constructors (define out-of-line after class body):
   - Converting constructor with SFINAE constraints
   - In-place constructors
   - Default copy/move = default
6. Implement `error()` in 4 overloads
7. Implement `swap()` member and friend
8. Implement `operator==` as hidden friend
9. Add deduction guide
10. Expand `unexpected.test.cpp` with comprehensive tests
11. Run `make test` and `make lint`
12. Write `docs/plan/handoff-next.md` summarizing what was done

## Verification

```bash
make test
make lint
```

## Handoff to Step 2

Step 1 done, next read `docs/plan/step2-bad-expected-access.md`.
`unexpected<E>` is now available for use in `bad_expected_access` and `expected`.
