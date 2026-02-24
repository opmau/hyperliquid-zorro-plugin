---
name: check-sizes
description: Check all source files against CLAUDE.md file size limits. Use when the user says "check sizes" or "check file sizes".
argument-hint: "[directory]"
user-invocable: true
allowed-tools: Bash, Read, Glob, Grep
model: haiku
---

# /check-sizes — Audit file sizes against limits

Scan source files and flag any that exceed the file size limits defined in CLAUDE.md.

## Steps

1. Read the file size limits from CLAUDE.md (File Size Limits section)

2. Count lines in all source files:
   - Headers/interfaces: check against header limit
   - Implementation files: check against implementation limit
   - Total per module (header + impl): check against total limit

3. Report in this format:
   ```
   FILE SIZE AUDIT
   ===============

   Within limits:  [count] files
   Over limit:     [count] files

   [If any over limit:]
   OVER LIMIT:
     [file] — [lines] lines ([limit] limit, [amount] over)
       Suggestion: [splitting strategy from CLAUDE.md]

   APPROACHING LIMIT (>80%):
     [file] — [lines] lines ([limit] limit)
   ```

## Arguments

- No arguments: scan all source directories
- `$ARGUMENTS`: scan specific directory

## Notes

- Use the file size calibration table in CLAUDE.md for the correct limits per language
- Suggest specific splitting strategies from the Splitting Strategies table
