---
name: retro
description: Run a post-session retrospective. Use when the user says "retro", "retrospective", or "what should we update?".
argument-hint: ""
user-invocable: true
allowed-tools: Read, Grep, Glob, Bash
model: sonnet
---

# /retro — Post-session retrospective

Review the current session and propose documentation updates.

## Steps

1. Summarize what was accomplished this session

2. Answer these questions:
   - **What went well?** Did approaches work on first try?
   - **What went wrong?** Any failed fixes, unexpected errors, or test failures?
   - **What was surprising?** Any undocumented codebase behavior discovered?
   - **What's left?** Any incomplete work to carry to next session?

3. Check for documentation updates needed:
   - Read current `CLAUDE.md` Domain Knowledge section — any new gotchas to add?
   - Read current `docs/KNOWN_ISSUES.md` — any issues to add or update?
   - Read current `docs/CURRENT_SPRINT.md` — any progress to record?
   - Any build/test commands that changed?
   - Any architecture decisions that should be documented?

4. Report in this format:
   ```
   ## Session Retrospective

   ### Accomplished
   - [what was done]

   ### Issues Encountered
   - [any problems and how they were resolved]

   ### Proposed Documentation Updates
   - [ ] CLAUDE.md: Add gotcha about [X] to Domain Knowledge
   - [ ] KNOWN_ISSUES.md: [Add/Update] issue about [Y]
   - [ ] CURRENT_SPRINT.md: Update progress on [Z]
   - [ ] No updates needed ← (say this if true, don't invent updates)

   ### Carry Forward
   - [anything incomplete for next session]
   ```

## Notes

- Only propose documentation updates that are genuinely needed
- Don't invent problems — if the session was clean, say so
- This skill enforces the Feedback Loop from CLAUDE.md
