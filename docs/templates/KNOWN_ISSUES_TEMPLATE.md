# Known Issues — [Project Name]

Bugs and technical debt discovered during development.

<!-- ═══════════════════════════════════════════════════════════════════════
     HOW TO USE THIS FILE

     1. When you discover a bug, add it here BEFORE attempting to fix it
     2. During refactoring phases: document here and DO NOT fix (per CLAUDE.md)
     3. During normal development: document here, then fix in a dedicated session
     4. Mark issues as ~~FIXED~~ with strikethrough when resolved
     5. Keep the fix details — they're valuable post-mortems
     ═══════════════════════════════════════════════════════════════════════ -->

---

## Issue Statuses

| Status | Meaning |
|--------|---------|
| **Open** | Confirmed bug, not yet fixed |
| **Investigating** | Root cause being analyzed |
| **FIXED** | Resolved (keep details for reference) |
| **Mitigated** | Workaround in place, not fully resolved |
| **Won't Fix** | Accepted behavior, documented why |

---

<!-- ══════════════════════════════════════════
     ACTIVE ISSUES (unfixed)
     ══════════════════════════════════════════ -->

## 1. [Short descriptive title]

**Status:** Open
**Priority:** [High | Medium | Low]
**Found in:** [file/module where bug lives]

**Symptom:**
[What the user sees or what goes wrong — observable behavior]

**Root Cause:**
[Why it happens. Use "Suspected:" prefix if unconfirmed]

**Evidence:**
[Log lines, test output, or code references that prove the issue]

**Fix Approach:**
[Brief description of how to fix — enough for a future session to act on]

**Target Module:** [Which file(s) should contain the fix]

---

<!-- ══════════════════════════════════════════
     RESOLVED ISSUES (keep for reference)
     ══════════════════════════════════════════ -->

## ~~2. [Example resolved issue]~~ FIXED

**Status:** FIXED — [date or commit ref]

**Root Cause:** [What was wrong]

**Fix Applied:** [What was changed and where]

---

<!-- ══════════════════════════════════════════
     TEMPLATE — Copy this for new issues
     ══════════════════════════════════════════ -->

## Template for New Issues

```markdown
## [N]. [Short Title]

**Status:** Open
**Priority:** [High | Medium | Low]
**Found in:** [file/module]

**Symptom:**
[What the user sees or what happens]

**Root Cause:**
[Why it happens, or "Suspected:" if unconfirmed]

**Evidence:**
[Log output, test failures, code references]

**Fix Approach:**
[Brief description of how to fix]

**Target Module:** [Which file(s) should contain the fix]
```
