#!/bin/bash
# ============================================================================
# check-scope.sh — PreToolUse hook for Edit|Write
# ============================================================================
# Warns when Claude edits files outside expected directories.
# Does NOT block — provides context so Claude can self-correct.
#
# CUSTOMIZE: Adjust ALLOWED_DIRS for your project structure.
# ============================================================================

INPUT=$(cat)
FILE_PATH=$(echo "$INPUT" | jq -r '.tool_input.file_path // empty')

if [ -z "$FILE_PATH" ]; then
  exit 0
fi

# --- CUSTOMIZE THESE FOR YOUR PROJECT ---
# Directories where edits are expected
ALLOWED_DIRS="src/ tests/ docs/"
# Files that are always OK to edit
ALLOWED_FILES="CLAUDE.md docs/KNOWN_ISSUES.md docs/CURRENT_SPRINT.md"
# ----------------------------------------

# Check if file is in an allowed directory
IN_SCOPE=false
for DIR in $ALLOWED_DIRS; do
  if echo "$FILE_PATH" | grep -q "$DIR"; then
    IN_SCOPE=true
    break
  fi
done

# Check if file is in the always-allowed list
for ALLOWED in $ALLOWED_FILES; do
  if echo "$FILE_PATH" | grep -q "$ALLOWED"; then
    IN_SCOPE=true
    break
  fi
done

if [ "$IN_SCOPE" = false ]; then
  cat << EOF
{
  "hookSpecificOutput": {
    "hookEventName": "PreToolUse",
    "additionalContext": "SCOPE WARNING: You're editing '$FILE_PATH' which is outside the expected directories ($ALLOWED_DIRS). Verify this file is in scope for the current task."
  }
}
EOF
fi

# Always allow — this is advisory, not blocking
exit 0
