#!/bin/bash
# ============================================================================
# check-file-size.sh — PostToolUse hook for Edit|Write
# ============================================================================
# Checks if the edited file now exceeds the size limits from CLAUDE.md.
# Returns additional context (warning) but does NOT block (exit 0).
#
# CUSTOMIZE: Adjust HEADER_LIMIT, IMPL_LIMIT for your language.
#            See CLAUDE.md File Size Limits calibration table.
# ============================================================================

# Read hook input from stdin
INPUT=$(cat)

# Extract the file path from the tool input
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // empty')

if [ -z "$FILE_PATH" ] || [ ! -f "$FILE_PATH" ]; then
  exit 0
fi

# --- CUSTOMIZE THESE FOR YOUR PROJECT ---
HEADER_LIMIT=150
IMPL_LIMIT=400
TOTAL_LIMIT=500
# ----------------------------------------

LINE_COUNT=$(wc -l < "$FILE_PATH")
FILENAME=$(basename "$FILE_PATH")

# Determine limit based on file type
LIMIT=$IMPL_LIMIT
TYPE="implementation"
case "$FILENAME" in
  *.h|*.hpp|*.d.ts|*.types.ts)
    LIMIT=$HEADER_LIMIT
    TYPE="header/interface"
    ;;
esac

if [ "$LINE_COUNT" -gt "$LIMIT" ]; then
  OVER=$((LINE_COUNT - LIMIT))
  # Output warning as additional context (Claude will see this)
  cat << EOF
{
  "hookSpecificOutput": {
    "hookEventName": "PostToolUse",
    "additionalContext": "WARNING: $FILENAME is now $LINE_COUNT lines ($OVER over the $LIMIT-line $TYPE limit). Consider splitting per CLAUDE.md Splitting Strategies."
  }
}
EOF
fi

exit 0
