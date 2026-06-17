# Hyperliquid Plugin for Zorro

[![CI](https://github.com/opmau/hyperliquid-zorro-plugin/actions/workflows/ci.yml/badge.svg)](https://github.com/opmau/hyperliquid-zorro-plugin/actions/workflows/ci.yml)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](LICENSE)
[![Sponsor](https://img.shields.io/badge/Sponsor-%E2%9D%A4-ea4aaa?logo=githubsponsors)](https://github.com/sponsors/opmau)

A broker plugin that connects the [Zorro](https://zorro-project.com/) algorithmic trading platform to the [Hyperliquid](https://hyperliquid.xyz/) decentralized exchange.

> ❤️ **Find this useful?** This plugin is free and open source (AGPL-3.0). If it
> saves you time or makes you money, please consider
> [**sponsoring its development**](https://github.com/sponsors/opmau) — it
> directly funds maintenance, new features, and bug fixes.

<!-- -->

> ⚠️ **Risk warning — read before trading.** This software places real orders on a live exchange with real funds. It is provided "as is", without warranty of any kind (see [LICENSE](LICENSE)). Trading perpetual futures is high-risk and you can lose more than your deposit. **Always test on testnet first**, start with small size, and prefer an [API/agent wallet](https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/nonces-and-api-wallets) over your master account's private key. You are solely responsible for any losses. See [SECURITY.md](SECURITY.md) for how to report vulnerabilities.

## Features

- **Perpetual futures trading** on Hyperliquid L1 (spot and HIP-3 support is planned — see [Status](#status))
- **Real-time market data** via WebSocket (l2Book, allMids)
- **HTTP fallback** when the WebSocket is unhealthy
- **Native EIP-712 order signing** (no external signer process)
- **Position and order management** through Zorro's broker interface

## Architecture

```text
API Layer         hl_broker.cpp, hl_broker_commands.cpp, hl_broker_trade.cpp
                  (Zorro broker interface - thin adapter)

Service Layer     hl_market_service, hl_trading_service, hl_account_service, hl_meta
                  (Business logic, coordinates transport)

Transport Layer   hl_http, ws_connection, ws_manager, ws_price_cache
                  (Network I/O, WebSocket, caching)

Foundation Layer  hl_types, hl_config, hl_globals, hl_utils, hl_crypto, hl_eip712
                  (No dependencies on upper layers)
```

See [docs/dev/ARCHITECTURE.md](docs/dev/ARCHITECTURE.md) for the full design.

## Prerequisites

- **Windows** (Zorro is Windows-only)
- **[Zorro](https://zorro-project.com/)** trading platform (the free version works)
- **[Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/)** with the C++ workload
- **[CMake](https://cmake.org/)** ≥ 3.16 (bundled with the VS Build Tools)
- **[vcpkg](https://vcpkg.io/)** package manager
- **[IXWebSocket](https://github.com/machinezone/IXWebSocket)** with TLS — installed via vcpkg (see below)

## Setup

### 1. Clone

```batch
git clone https://github.com/opmau/hyperliquid-zorro-plugin.git
cd hyperliquid-zorro-plugin
```

### 2. Provide the Zorro SDK headers

The build needs two proprietary Zorro SDK headers — `trading.h` and `variables.h`
— in an `include/` directory at the repo root. They are **not** redistributed
here (they are © oP group and ship with Zorro). Use whichever applies:

**(a) You have Zorro installed locally** — junction to your install:

```batch
mklink /J include "C:\Zorro\include"
```

**(b) No local install** — extract just the two headers from the official Zorro
distribution (this is what CI does, see [.github/workflows/ci.yml](.github/workflows/ci.yml)):

```powershell
mkdir include
curl -L -o zorro.zip https://opserver.de/down/ZorroBeta.zip
tar -xf zorro.zip -C include --strip-components=2 ZorroBeta/include/trading.h ZorroBeta/include/variables.h
```

> The plugin is built against the Zorro `TRADE` ABI; the current stable and beta
> headers share an identical `TRADE` struct layout, so either works at runtime.

### 3. Install dependencies

vcpkg resolves dependencies from [vcpkg.json](vcpkg.json) (manifest mode) during
the CMake configure step, so no manual `vcpkg install` is required. If you prefer
to install manually, match the manifest's TLS features:

```batch
vcpkg install ixwebsocket[ssl,mbedtls]:x86-windows-static
```

### 4. Build

```batch
mkdir build_vcpkg
cd build_vcpkg
cmake .. -G "Visual Studio 17 2022" -A Win32 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x86-windows-static
cmake --build . --config Release
```

**Important:** Zorro is a 32-bit application. Always build with `-A Win32`.

Output: `build_vcpkg/Release/Hyperliquid_Dev.dll`

#### Production build

To build as `Hyperliquid.dll` instead of `Hyperliquid_Dev.dll`, add `-DDEV_BUILD=OFF`:

```batch
cmake .. -DDEV_BUILD=OFF -G "Visual Studio 17 2022" -A Win32 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x86-windows-static
cmake --build . --config Release
```

Output: `build_vcpkg/Release/Hyperliquid.dll`

### 5. Deploy

Copy the DLL to your Zorro `Plugin` directory:

```batch
REM Dev build
copy build_vcpkg\Release\Hyperliquid_Dev.dll "C:\Zorro\Plugin\"

REM Production build
copy build_vcpkg\Release\Hyperliquid.dll "C:\Zorro\Plugin\"
```

> Zorro loads the plugin DLL at startup and holds it in memory. After deploying a
> new DLL, fully restart Zorro for it to take effect.

### 6. Configure in Zorro

1. Select **Hyperliquid** (or **Hyperliquid-DEV** for dev builds) as the broker.
2. Enter your **master account address** in the User field.
3. Enter your **API/agent wallet private key** in the Password field
   (prefer an [agent wallet](https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/nonces-and-api-wallets) over your master key).
4. **Use testnet first** — select a testnet account so trades go to
   `api.hyperliquid-testnet.xyz` before risking real funds.

## Testing

A clean clone builds and tests via CMake (this is what CI runs):

```batch
cmake --build build_vcpkg --config Release --target test_ws_price_cache
build_vcpkg\Release\test_ws_price_cache.exe
```

The `tests/` directory also contains standalone `cl.exe` compile-and-run scripts
for individual modules. Run them from a Developer Command Prompt:

```batch
cd tests
run_unit_tests.bat
```

## Project Structure

```text
src/
  foundation/    Types, config, utilities, crypto primitives
  transport/     HTTP client, WebSocket connection/manager/cache
  services/      Market data, trading, account, metadata services
  api/           Zorro broker plugin interface
  vendor/        Vendored third-party libraries (yyjson, secp256k1) — see each LICENSE
tests/           Unit and compile tests
docs/dev/        Developer documentation
build/           CMake helper scripts
.github/         CI/CD workflows
```

## Documentation

- [docs/dev/ARCHITECTURE.md](docs/dev/ARCHITECTURE.md) — layered design and dependency rules
- [docs/dev/GETTING_STARTED.md](docs/dev/GETTING_STARTED.md) — developer onboarding
- [docs/dev/TESTING.md](docs/dev/TESTING.md) — test strategy and how to run tests
- [docs/dev/ADDING_FEATURES.md](docs/dev/ADDING_FEATURES.md) — extending the plugin
- [docs/dev/GOTCHAS.md](docs/dev/GOTCHAS.md) — platform pitfalls (Lite-C, Zorro, OneDrive)
- [CHANGELOG.md](CHANGELOG.md) — release history
- [CONTRIBUTING.md](CONTRIBUTING.md) — how to contribute
- [SECURITY.md](SECURITY.md) — vulnerability reporting

## Status

Under active development. Currently supports:

- Perpetual futures (spot and HIP-3 are architecturally supported but not yet enabled)
- Market and limit orders
- Position queries and account info
- Real-time price streaming

## License

[GNU Affero General Public License v3.0](LICENSE) (AGPL-3.0).

This is copyleft: you are free to use, study, and modify the plugin, but any
distributed or network-served derivative must also be released under the
AGPL-3.0. This keeps modifications open and prevents closed-source resale. For a
commercial/proprietary license, contact the maintainer.

Vendored third-party components under `src/vendor/` retain their own permissive
licenses (MIT / public domain) — see `src/vendor/*/LICENSE`. The Zorro SDK
headers are © oP group and are not included in this repository.
