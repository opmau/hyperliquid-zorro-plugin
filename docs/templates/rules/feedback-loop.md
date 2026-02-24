# Feedback Loop & Continuous Improvement

## Post-Session Review

At the end of each significant work session, consider:

1. **What went well?** — Did the approach work on first try?
2. **What went wrong?** — Did a fix fail? Did tests catch something?
3. **What was surprising?** — Any undocumented codebase behavior discovered?
4. **Should CLAUDE.md be updated?** — Any new gotchas for Domain Knowledge?

## Living Documentation Rule

These project docs are living documents. Proactively suggest updates when:

- A gotcha was discovered that's not in Domain Knowledge
- A build/test command changed
- An architecture decision was made
- A bug was fixed (update KNOWN_ISSUES.md)
- A rule in CLAUDE.md was wrong or unhelpful — say so

## Retrospective Triggers

After these events, prompt a brief retrospective:

- A bug that tests didn't catch
- A fix that took >2 attempts
- A refactor that touched >5 files
- A production incident
