---
paths:
  - "src/**/*"
  - "tests/**/*"
---

# File Size Limits

These limits optimize Claude Code's performance (context window, edit accuracy, blast radius).

| Type | Limit | Action if Exceeded |
|------|-------|-------------------|
| Header / interface files | 150 lines | Split into sub-modules |
| Implementation files | 400 lines | Split into sub-modules |
| Total per module | 500 lines | Split or refactor |

If you're about to create a file exceeding these limits, STOP and discuss splitting strategy.

## Splitting Strategies

| Situation | Strategy |
|-----------|----------|
| Multiple unrelated responsibilities | Extract by responsibility |
| One responsibility but lots of code | Extract helpers/utilities |
| Header has too many declarations | Split public vs internal |
| Test file is too long | Split by feature under test |
