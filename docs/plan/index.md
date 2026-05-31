# Plan: Implement expected<T&, E> and expected<T, E&> Reference Specializations

## Proposal

Implement `std::expected` over references for both `T` and `E` parameters,
applying the same assign-through and rebind semantics adopted for C++26
`std::optional<T&>` (P2988). This requires first completing a working primary
template `expected<T, E>` from the existing skeleton.

## Reference Materials

- **beman/optional** — reference implementation of `optional<T&>` with rebind
  semantics (`~/src/steve-downey/optional/main`)
- **C++26 standard wording** — `[expected.expected]`, `[expected.void]`,
  `[expected.unexpected]`, `[expected.bad]`
- **P2988** — `std::optional<T&>` adopted design (rebind on assignment)

## Phase Overview

| Step | Branch | Deliverable | Depends on |
|------|--------|-------------|-----------|
| 1 | `step1-unexpected` | `unexpected<E>` class template | — |
| 2 | `step2-bad-expected-access` | `bad_expected_access<E>` + void specialization | — |
| 3 | `step3-expected-primary` | `expected<T, E>` primary template (value types only) | Steps 1-2 |
| 4 | `step4-expected-void` | `expected<void, E>` partial specialization | Steps 1-2 |
| 5 | `step5-expected-primary-monadic` | Monadic operations for `expected<T, E>` | Step 3 |
| 6 | `step6-expected-void-monadic` | Monadic operations for `expected<void, E>` | Step 4 |
| 7 | `step7-expected-ref-t` | `expected<T&, E>` reference specialization | Steps 3, 5 |
| 8 | `step8-expected-ref-e` | `expected<T, E&>` error-reference specialization | Steps 3, 5 |
| 9 | `step9-expected-ref-both` | `expected<T&, E&>` both-reference specialization | Steps 7, 8 |
| 10 | `step10-expected-void-ref-e` | `expected<void, E&>` void+error-ref specialization | Steps 4, 6 |

Steps 1-2 are independent foundations (can be parallel).
Steps 3-4 depend on 1-2 and are independent of each other.
Steps 5-6 add monadic ops to their respective primary/void templates.
Steps 7-10 are the reference specializations (the novel work in this proposal).

## Standing Conventions

- Include guards: `#ifndef`/`#define`/`#endif` (never `#pragma once`)
- Format: `BEMAN_EXPECTED_<PATH>_HPP` (uppercase, path separators to `_`)
- Includes: angle brackets, full paths from include root
- SPDX license: `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`
- Functions defined out-of-line in headers (body after class)
- `constexpr` everything possible
- Test framework: GoogleTest
- Test files include the header under test twice (idempotence check)
- Each step runs `make test` and `make lint` before completion
- Namespace: `beman::expected`

## Step Details

- [Step 1: unexpected<E>](step1-unexpected.md)
- [Step 2: bad_expected_access](step2-bad-expected-access.md)
- [Step 3: expected<T,E> primary template](step3-expected-primary.md)
- [Step 4: expected<void,E> specialization](step4-expected-void.md)
- [Step 5: Monadic ops for expected<T,E>](step5-expected-primary-monadic.md)
- [Step 6: Monadic ops for expected<void,E>](step6-expected-void-monadic.md)
- [Step 7: expected<T&,E> reference specialization](step7-expected-ref-t.md)
- [Step 8: expected<T,E&> error-reference specialization](step8-expected-ref-e.md)
- [Step 9: expected<T&,E&> both-reference specialization](step9-expected-ref-both.md)
- [Step 10: expected<void,E&> void+error-ref specialization](step10-expected-void-ref-e.md)

## Checklist

- [ ] Step 1: `unexpected<E>` — constructors, error() observers, swap, equality, deduction guide
- [ ] Step 2: `bad_expected_access<E>` and `bad_expected_access<void>` base
- [ ] Step 3: `expected<T, E>` primary — constructors, destructor, assignment, emplace, swap, observers, value_or, error_or, equality
- [ ] Step 4: `expected<void, E>` — constructors, destructor, assignment, emplace, swap, observers, error_or, equality
- [ ] Step 5: `expected<T, E>` monadic — and_then, or_else, transform, transform_error (4 ref-qualified overloads each)
- [ ] Step 6: `expected<void, E>` monadic — and_then, or_else, transform, transform_error
- [ ] Step 7: `expected<T&, E>` — pointer storage, rebind assignment, observers returning T&, value_or, monadic ops, dangling prevention
- [ ] Step 8: `expected<T, E&>` — union+pointer storage, error as E&, rebind error assignment, observers, monadic ops
- [ ] Step 9: `expected<T&, E&>` — both pointer storage, rebind both, observers, monadic ops
- [ ] Step 10: `expected<void, E&>` — no value storage, error pointer, rebind error, observers, monadic ops
