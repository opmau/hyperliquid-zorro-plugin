#!/bin/bash
# ============================================================================
# inject-critical-rules.sh — PreCompact hook
# ============================================================================
# Before context compaction, outputs the most critical rules so they
# survive in the compressed context. Without this, Claude can "forget"
# rules after long sessions.
#
# CUSTOMIZE: Add your most important rules below.
# ============================================================================

cat << 'EOF'
{
  "hookSpecificOutput": {
    "hookEventName": "PreCompact",
    "additionalContext": "CRITICAL RULES (survive compaction):\n1. Evidence-based claims: FIND code, QUOTE file:line, VERIFY before claiming\n2. Max 2 fix attempts: STOP after 2nd failure, SUMMARIZE, ASK for guidance\n3. Anti-sycophancy: Push back when you disagree, don't agree just to be agreeable\n4. Scope: Only modify files in scope for the current task\n5. File size limits: Check limits after edits, propose splitting if exceeded\n6. Document bugs: Add to KNOWN_ISSUES.md, don't fix during other work\n7. Test before commit: Run relevant tests before any commit"
  }
}
EOF

exit 0
