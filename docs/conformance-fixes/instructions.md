# Instructions for Fix Steps

These instructions are given to a clean Sonnet instance at the start of
each fix step. The instance has no prior context.

---

## Prompt

Read these files in order:

1. `docs/conformance-fixes/index.md` — plan overview and checklist
2. `docs/conformance-fixes/fix<N>-<name>.md` — the current step's spec
3. `docs/conformance-audit.md` — the gap analysis (for context)
4. `docs/plan/handoff-next.md` — current repository state

Then:

1. Create a worktree and branch named per the step spec (e.g. `fix1-constraints`).
2. Implement the changes described in the step file.
3. Write tests as specified; register them in `tests/beman/expected/CMakeLists.txt`.
4. Run `make TOOLCHAIN=gcc-16 test` — all tests must pass.
5. Run `make lint` — all linters must pass (ignore pre-existing beman-tidy crash).
6. Merge (--no-ff) the step branch into `expected-over-references`.
7. Update `docs/conformance-fixes/index.md` checklist to mark the step done.
8. Write a new `docs/plan/handoff-next.md` describing the post-fix state and
   what a fresh instance needs to know for the next step.

Do not merge to `main`. The feature branch `expected-over-references` accumulates
all work and will be merged to `main` when all steps complete.
