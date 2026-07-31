#!/usr/bin/env python3
"""Guard against publishing private material in this public repository.

This repository is public and is used by traders running real money. Everything
git tracks -- source, comments, tests, docs, CI config, commit messages -- is
read by strangers. This script fails the build when content that belongs to the
maintainer's private environment is about to be committed.

Two classes of check:

1. Built-in patterns that are unsafe for any public repo: absolute developer
   home paths, private-key material, and assignments to secret-shaped
   environment variables.

2. An optional site-specific deny list, one regular expression per line, read
   from a file that is itself untracked (see PRIVATE_TERMS_FILE below). Private
   strategy, account, and log names belong there -- never in this file, which is
   public and would otherwise disclose the very names it is meant to protect.

Exit codes:
    0  nothing found
    1  at least one finding (commit should be blocked)
    2  usage or environment error

Usage:
    python scripts/check_public_safety.py --staged   # what is about to commit
    python scripts/check_public_safety.py --all      # every tracked file
    python scripts/check_public_safety.py FILE...    # specific paths
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

# Deny list of site-specific terms. Untracked on purpose -- see module docstring.
PRIVATE_TERMS_FILE = ".private-terms"

# Paths that are third-party or generated, and are not ours to police.
EXCLUDED_PREFIXES = (
    "src/vendor/",
    "include/",
    "docs/zorro_docs/",
)

# A line carrying this marker is exempt. Use it to keep well-known public test
# vectors (for example the Anvil account-0 key) in the tree, clearly labelled.
ALLOW_MARKER = "public-test-vector"

# A file may waive one rule for all of its lines by declaring, near the top:
#     check-public-safety: allow private-key (published SDK signing vectors)
# The trailing reason is required so every waiver is self-documenting, and the
# rule name is explicit so a waiver cannot silently disable the other checks.
FILE_WAIVER = re.compile(
    r"check-public-safety:\s*allow\s+([a-z][a-z-]*)\s*\(([^)]+)\)"
)

BUILTIN_RULES: list[tuple[str, str, str]] = [
    (
        "developer-home-path",
        r"[A-Za-z]:[\\/]+Users[\\/]+[A-Za-z0-9._-]+",
        "Absolute path into a developer home directory. Use a relative path or "
        "an environment variable such as %USERPROFILE%.",
    ),
    (
        "developer-home-path",
        r"/[a-z]/Users/[A-Za-z0-9._-]+",
        "Absolute MSYS path into a developer home directory. Use a relative "
        "path or an environment variable.",
    ),
    (
        "private-key",
        r"0x[0-9a-fA-F]{64}\b",
        "32-byte value in private-key form. Never commit key material; if this "
        f"is a published test vector, label the line '{ALLOW_MARKER}'.",
    ),
    (
        "secret-assignment",
        r"(?i)\b(private_key|secret_key|api_key|api_token|access_token|password)"
        r"\s*[=:]\s*[\"']?[A-Za-z0-9_\-./+]{12,}",
        "Assignment of a credential-shaped value.",
    ),
]


def repo_root() -> Path:
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            capture_output=True, text=True, check=True,
        )
    except (OSError, subprocess.CalledProcessError):
        sys.stderr.write("error: not inside a git repository\n")
        raise SystemExit(2)
    return Path(out.stdout.strip())


def load_private_terms(root: Path) -> list[tuple[str, str, str]]:
    """Read the untracked deny list, if present."""
    path = root / PRIVATE_TERMS_FILE
    if not path.is_file():
        return []
    rules = []
    for lineno, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        term = raw.strip()
        if not term or term.startswith("#"):
            continue
        try:
            re.compile(term)
        except re.error as exc:
            sys.stderr.write(f"warning: {PRIVATE_TERMS_FILE}:{lineno}: {exc}\n")
            continue
        rules.append((
            "private-term",
            term,
            "Matches a term from the private deny list. Describe the observable "
            "behaviour instead, with no reference to the private source.",
        ))
    return rules


def is_excluded(path: str) -> bool:
    norm = path.replace("\\", "/")
    return norm.startswith(EXCLUDED_PREFIXES) or norm == f"scripts/{Path(__file__).name}"


def git_lines(args: list[str]) -> list[str]:
    out = subprocess.run(["git", *args], capture_output=True, text=True, check=True)
    return [ln for ln in out.stdout.splitlines() if ln]


def staged_content(path: str) -> str | None:
    """Content of a path as it currently sits in the index."""
    out = subprocess.run(["git", "show", f":{path}"], capture_output=True, text=True)
    if out.returncode != 0:
        return None
    return out.stdout


def scan(text: str, path: str, rules) -> list[tuple[str, int, str, str, str]]:
    findings = []
    lines = text.splitlines()
    waived = {m.group(1) for m in (FILE_WAIVER.search(ln) for ln in lines) if m}
    # The deny list is the whole point of this check; it can never be waived.
    waived.discard("private-term")
    for lineno, line in enumerate(lines, 1):
        if ALLOW_MARKER in line:
            continue
        for name, pattern, advice in rules:
            if name in waived:
                continue
            match = re.search(pattern, line)
            if match:
                found = match.group(0)
                if name in ("private-key", "secret-assignment"):
                    found = found[:10] + "...[redacted]"
                findings.append((path, lineno, name, found, advice))
                break
    return findings


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Fail if private material is about to be published."
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--staged", action="store_true",
                       help="scan files staged for commit (default)")
    group.add_argument("--all", action="store_true",
                       help="scan every tracked file")
    parser.add_argument("paths", nargs="*", help="specific paths to scan")
    args = parser.parse_args()

    root = repo_root()
    rules = BUILTIN_RULES + load_private_terms(root)

    if args.paths:
        targets = [(p, None) for p in args.paths]
    elif args.all:
        targets = [(p, None) for p in git_lines(["ls-files"])]
    else:
        targets = [(p, "staged") for p in
                   git_lines(["diff", "--cached", "--name-only", "--diff-filter=ACM"])]

    findings = []
    for path, source in targets:
        if is_excluded(path):
            continue
        if source == "staged":
            text = staged_content(path)
        else:
            file_path = root / path
            if not file_path.is_file():
                continue
            try:
                text = file_path.read_text(encoding="utf-8")
            except (UnicodeDecodeError, OSError):
                continue  # binary or unreadable
        if text is None:
            continue
        findings.extend(scan(text, path, rules))

    if not findings:
        return 0

    sys.stderr.write("\nPrivate material must not be published:\n\n")
    for path, lineno, name, found, advice in findings:
        sys.stderr.write(f"  {path}:{lineno}\n")
        sys.stderr.write(f"    [{name}] {found}\n")
        sys.stderr.write(f"    {advice}\n\n")
    sys.stderr.write(f"{len(findings)} finding(s). Resolve them, or label a "
                     f"published test vector with '{ALLOW_MARKER}'.\n")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
