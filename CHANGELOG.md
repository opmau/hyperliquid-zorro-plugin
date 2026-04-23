# Changelog

All notable changes to the Hyperliquid Zorro Plugin are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- `build/build_prod_dll.bat` — production build script (CMake + vcpkg, `DEV_BUILD=OFF`)
- `.github/workflows/release.yml` — CI release workflow that builds and attaches `Hyperliquid.dll` on tag push
- `/release` skill — codifies the release workflow with pre-flight checks

### Changed
- PerpDex `clearinghouseState` WS subscriptions are now lazy — the plugin only subscribes to perpDexes the strategy actually uses via `BrokerAsset` (main-dex-only strategies avoid ~54k unused WS messages per session) [OPM-439]

### Fixed
- PerpDex position lookup: extract `dex` directly from WS `data` object and resolve `@index` coin format via index mapping, so `getPosition()` finds perpDex positions reliably [OPM-226]
- `BrokerCommand` trace log and HTTP-fallback `getPrice` notices raised to `diagLevel>=3`/`>=2` respectively (were flooding logs at `diagLevel=1`) [OPM-438]
- `BrokerAsset` now sets `priceSymbol` for `GET_PRICE` contract
- Flat positions are detected before issuing close orders to prevent Error 075 [OPM-227]
- `HL_GET_OPEN_ORDERS` queries WS `PriceCache` directly [OPM-237]

## [2.0.0] — _unreleased_

First production release of the refactored plugin.

### Architecture
- 4-layer modular architecture (Foundation → Transport → Services → API) replacing the monolithic `Hyperliquid_Native.cpp`
- 17 source files organized under `src/foundation/`, `src/transport/`, `src/services/`, `src/api/`
- File size limits enforced (header ≤250 lines, impl ≤600 lines, module ≤800 lines)

### WebSocket
- Migrated from WinHTTP to IXWebSocket backend with native auto-reconnect [OPM-127, OPM-128]
- PerpDex `clearinghouseState` subscriptions [OPM-218, OPM-219]
- L2 book subscriptions with 60-second health-check fallback to HTTP

### Trading
- NFA-compliant order flow (Hedge=0 semantics, no stop-and-reverse)
- Bracket orders via separate stop-loss placement (`SET_ORDERTYPE +8`)
- `BrokerTrade` uses WS `PriceCache` for open orders [OPM-237]

### Testing
- Regression suite (`tests/run_unit_tests.bat`) with 21 test modules covering crypto, HTTP parsing, position tracking, trading logic, and WS cache interactions
- `SmokeTest_Dev.c` / `SmokeTest_Prod.c` integration scripts for testnet and mainnet verification

---

[Unreleased]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.0...HEAD
[2.0.0]: https://github.com/opmau/hyperliquid-zorro-plugin/releases/tag/v2.0.0
