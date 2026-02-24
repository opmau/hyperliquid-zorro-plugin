#!/bin/bash
# ============================================================================
# session-check.sh — Stop hook
# ============================================================================
# When Claude finishes a significant response, reminds it about the
# feedback loop. This is a lightweight nudge, not a full retrospective.
#
# Only triggers every N responses to avoid being noisy.
# ============================================================================

# Track response count in a temp file
COUNTER_FILE="/tmp/claude-session-response-count"
if [ -f "$COUNTER_FILE" ]; then
  COUNT=$(cat "$COUNTER_FILE")
else
  COUNT=0
fi

COUNT=$((COUNT + 1))
echo "$COUNT" > "$COUNTER_FILE"

# Only trigger every 10th response (adjust as needed)
if [ $((COUNT % 10)) -eq 0 ]; then
  cat << 'EOF'
{
  "hookSpecificOutput": {
    "hookEventName": "Stop",
    "additionalContext": "SESSION CHECK: You've had 10+ interactions. Consider: (1) Any gotchas discovered that should go in CLAUDE.md? (2) Any bugs found that should go in KNOWN_ISSUES.md? (3) Is context getting full — should we /clear?"
  }
}
EOF
fi

exit 0
