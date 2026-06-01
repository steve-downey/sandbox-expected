# Step 2: Implement bad_expected_access

**Branch:** `step2-bad-expected-access`
**Depends on:** None (independent of Step 1)
**Read first:** `docs/plan/handoff.md` and `docs/plan/index.md`

---

## Goal

Implement `bad_expected_access<void>` (the base class) and
`bad_expected_access<E>` (the derived template) per [expected.bad] and
[expected.bad.void].

## Context for Executing Agent

The file `include/beman/expected/bad_expected_access.hpp` exists with the
specification as comments. These exception types are thrown by `expected::value()`
when accessed without a value.

### Key files

- `include/beman/expected/bad_expected_access.hpp` — implement here
- `tests/beman/expected/bad_expected_access.test.cpp` — expand tests
- `tests/beman/expected/CMakeLists.txt` — already includes this test

### Standard reference

**bad_expected_access<void>** [expected.bad.void]:
```cpp
template<>
class bad_expected_access<void> : public exception {
protected:
    constexpr bad_expected_access() noexcept;
    constexpr bad_expected_access(const bad_expected_access&) noexcept;
    constexpr bad_expected_access(bad_expected_access&&) noexcept;
    constexpr bad_expected_access& operator=(const bad_expected_access&) noexcept;
    constexpr bad_expected_access& operator=(bad_expected_access&&) noexcept;
    constexpr ~bad_expected_access();
public:
    constexpr const char* what() const noexcept override;
};
```

**bad_expected_access<E>** [expected.bad]:
```cpp
template<class E>
class bad_expected_access : public bad_expected_access<void> {
public:
    constexpr explicit bad_expected_access(E);
    constexpr const char* what() const noexcept override;
    constexpr E& error() & noexcept;
    constexpr const E& error() const & noexcept;
    constexpr E&& error() && noexcept;
    constexpr const E&& error() const && noexcept;
private:
    E unex;  // exposition only
};
```

### Notes

- `bad_expected_access<void>` inherits from `std::exception`
- The `what()` message should return `"bad expected access"` (matches
  libstdc++ and libc++ convention)
- Protected constructors on the void specialization — it's only a base class
- The derived template stores the unexpected value and provides `error()` access

## Deliverables

1. **`include/beman/expected/bad_expected_access.hpp`** — full implementation:
   - Forward declaration of `bad_expected_access` template
   - `bad_expected_access<void>` explicit specialization (base class)
   - `bad_expected_access<E>` primary template (derived)

2. **`tests/beman/expected/bad_expected_access.test.cpp`** — tests:
   - Construct `bad_expected_access<int>(42)` and verify `error() == 42`
   - Verify `what()` returns a non-null string
   - Verify it inherits from `std::exception`
   - Verify `error()` in all 4 ref-qualified overloads
   - Verify `bad_expected_access<std::string>` works with move semantics

## Procedure

1. Create branch `step2-bad-expected-access` from `main`
2. In `bad_expected_access.hpp`, add includes: `<exception>`, `<utility>`
3. Add forward declaration: `template<class E> class bad_expected_access;`
4. Implement `bad_expected_access<void>` specialization:
   - Protected constructors/assignment = default
   - Public `what()` returning `"bad expected access"`
5. Implement `bad_expected_access<E>` primary template:
   - Constructor taking `E` by value, storing via move
   - `what()` override (same string)
   - `error()` in 4 overloads
6. Expand tests
7. Run `make test` and `make lint`
8. Write `docs/plan/handoff-next.md`

## Verification

```bash
make test
make lint
```

## Handoff to Step 3

Step 2 done, next read `docs/plan/step3-expected-primary.md`.
`bad_expected_access<E>` is now available for `expected::value()` to throw.
