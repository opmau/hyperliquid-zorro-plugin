---
paths:
  - "src/**/*"
  - "tests/**/*"
---

# Testing Protocol

## When to Run Tests

Check the test mapping table in CLAUDE.md to determine which tests to run based on what you modified. When in doubt, run the full suite.

## Bug Handling

- During refactoring: DO NOT FIX bugs. Document in KNOWN_ISSUES.md, continue working.
- During normal development: If a bug blocks your task, fix it. If unrelated, document it first.
- If a test fails: Document in KNOWN_ISSUES.md. Do NOT attempt on-the-fly fixes.

## Before Every Commit

1. Run the relevant test suite
2. If tests fail, do not commit
3. Document failures in KNOWN_ISSUES.md if they're pre-existing
