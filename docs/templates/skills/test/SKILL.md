---
name: test
description: Run tests. Use when the user says "test", "run tests", or before commits.
argument-hint: "[test-name or module]"
user-invocable: true
allowed-tools: Bash, Read, Grep
model: haiku
---

# /test — Run project tests

Run the test suite and report results.

## Steps

1. If `$ARGUMENTS` is provided, run the specific test:
   ```
   [YOUR SPECIFIC TEST COMMAND] $ARGUMENTS
   ```

2. If no arguments, run the full test suite:
   ```
   [YOUR FULL TEST COMMAND HERE]
   ```

3. Parse the output and report in this format:
   ```
   TESTS: [PASS/FAIL]
   Passed: [count]
   Failed: [count]
   Skipped: [count]

   [If failures:]
   FAILED: [test name] — [error summary]
   FAILED: [test name] — [error summary]
   ```

4. If tests fail, identify which test failed and the likely cause. Do NOT attempt to fix — just report.

## Notes

- Run the FULL suite before commits (no arguments)
- If a specific module was just changed, suggest which specific tests to run based on the test mapping in CLAUDE.md
