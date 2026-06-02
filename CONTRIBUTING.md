# Contributing

Thanks for your interest in improving the Hyperliquid plugin for Zorro!

## Before you start

- This is a trading plugin that handles real funds. Correctness matters more
  than speed — see [docs/dev/GOTCHAS.md](docs/dev/GOTCHAS.md) for the pitfalls
  that have caused real losses.
- By contributing, you agree your contributions are licensed under the project's
  [GNU Affero General Public License v3.0](LICENSE) (AGPL-3.0).
- Security issues: do **not** open a public issue — see [SECURITY.md](SECURITY.md).

## Development setup

Follow the [Setup](README.md#setup) section of the README. In short:

1. Clone the repo.
2. Provide the two Zorro SDK headers in `include/` (junction to a local Zorro
   install, or extract them from the official Zorro zip — see the README).
3. Build with CMake + vcpkg (32-bit / `-A Win32`).

A clean clone builds in CI without any local Zorro install — see
[.github/workflows/ci.yml](.github/workflows/ci.yml).

## Branching model

- `main` — production. Tagged releases are cut from here.
- `develop` — integration branch. **Open pull requests against `develop`.**
- `feature/*` — branch from `develop` for individual changes.

## Building and testing

```batch
REM Build (Dev DLL)
cmake --build build_vcpkg --config Release

REM Run the price-cache unit test (what CI runs)
cmake --build build_vcpkg --config Release --target test_ws_price_cache
build_vcpkg\Release\test_ws_price_cache.exe
```

Additional per-module compile-and-run tests live in `tests/` and can be run from
a Developer Command Prompt (`cd tests && run_unit_tests.bat`). See
[docs/dev/TESTING.md](docs/dev/TESTING.md).

Please make sure the build is green and relevant tests pass before opening a PR.

## Coding conventions

- Respect the layered architecture (Foundation → Transport → Services → API).
  Do not introduce reverse or circular dependencies. See
  [docs/dev/ARCHITECTURE.md](docs/dev/ARCHITECTURE.md).
- Match the style of surrounding code.
- Keep changes focused — one concern per pull request.

## Commit messages

Use [Conventional Commits](https://www.conventionalcommits.org/):

```text
type(scope): short description

feat(services): add TWAP order support
fix(transport): handle WebSocket reconnect race
docs: clarify build prerequisites
ci: pin vcpkg baseline
```

## Pull requests

1. Describe **what** changed and **why**.
2. Note any testing you performed (unit tests, testnet smoke test).
3. Update [CHANGELOG.md](CHANGELOG.md) under `[Unreleased]` if the change is
   user-facing.
4. Ensure CI is green.

## Code of conduct

Participation is governed by the [Code of Conduct](CODE_OF_CONDUCT.md).
