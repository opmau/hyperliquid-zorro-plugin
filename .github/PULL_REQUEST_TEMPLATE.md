<!--
Thanks for contributing! Open PRs against the `develop` branch.
Do not include secrets (private keys, account addresses) in code, tests, or logs.
-->

## What & why

Describe what this PR changes and the motivation.

## Related issues

Closes #

## Testing

- [ ] Builds locally (`cmake --build build_vcpkg --config Release`)
- [ ] Unit tests pass (`test_ws_price_cache` and/or `tests/run_unit_tests.bat`)
- [ ] Tested against testnet (if it touches trading/order logic)

Describe what you tested and the result.

## Checklist

- [ ] Targets the `develop` branch
- [ ] Respects the layered architecture (Foundation → Transport → Services → API)
- [ ] Updated `CHANGELOG.md` under `[Unreleased]` (if user-facing)
- [ ] No secrets, credentials, or personal paths committed
- [ ] CI is green
