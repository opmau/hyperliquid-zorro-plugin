# Scope of Change Guardrails

## Single-Responsibility Changes

- Each commit addresses ONE concern (one bug, one feature, one refactor)
- If you discover a second problem, DOCUMENT it in KNOWN_ISSUES.md, don't fix it now
- Do NOT "improve" code you're reading for context — only modify files you were asked to modify

## Pre-Change Checklist

Before modifying any file:

1. Is this file in scope for the current task?
2. Is this the right architectural layer for this change?
3. Will this break callers? If changing a signature, who calls it?
4. Is there a test for this? Run it after. No test? Consider adding one.

## Change Size Limits

| Change Type | Max Files | If Exceeded |
|-------------|-----------|-------------|
| Bug fix | 1-3 files | Ask if scope is correct |
| Feature addition | 3-5 files | Present plan before coding |
| Refactor | 5-10 files | Must have user-approved plan |
| Architecture change | Any | ALWAYS present plan first |
