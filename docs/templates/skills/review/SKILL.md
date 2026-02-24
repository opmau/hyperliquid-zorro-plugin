---
name: review
description: Review staged changes before commit. Use when the user says "review", "check my changes", or before committing.
argument-hint: ""
user-invocable: true
allowed-tools: Bash, Read, Grep, Glob
model: sonnet
---

# /review — Pre-commit code review

Review all staged and unstaged changes against project rules.

## Steps

1. Get the diff of all changes:
   ```
   git diff
   git diff --cached
   git status
   ```

2. For each changed file, check:
   - **Scope:** Is this file in scope for the current task?
   - **Layer violations:** Do new imports/includes violate the architecture dependency rules in CLAUDE.md?
   - **File size:** Does the file now exceed the limits in CLAUDE.md?
   - **Edge cases:** Are there unhandled edge cases in the new code?
   - **Test coverage:** Is there a test for this change? Should there be?

3. Report using this format:
   ```
   ## Review: [summary of changes]

   **Verdict:** APPROVE / NEEDS_WORK

   ### Files Changed:
   - [file] — [what changed and why]

   ### Checks:
   - [ ] Architecture: [PASS/FAIL — detail if fail]
   - [ ] File sizes: [PASS/FAIL — detail if fail]
   - [ ] Scope: [PASS/FAIL — any out-of-scope changes?]
   - [ ] Edge cases: [list any concerns]
   - [ ] Tests: [existing tests cover this? / new test needed?]

   ### Concerns (if NEEDS_WORK):
   - [specific concern with file:line]
   - [suggested fix]
   ```

## Notes

- Be critical. Flag real issues, not style nitpicks.
- If changes look clean, APPROVE quickly — don't invent problems.
- Reference specific CLAUDE.md rules when flagging violations.
