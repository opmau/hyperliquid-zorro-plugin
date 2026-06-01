# Changelog

All notable changes to the Hyperliquid Zorro Plugin are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [2.0.6] — 2026-06-01

Infrastructure-only release — no functional plugin changes since v2.0.5. Makes
the repository self-contained so it builds from a clean clone and enables full
CI/CD (the release DLL is now built and published automatically on tag).

### Build

- **Vendored crypto sources** into `src/vendor/secp256k1/` ([OPM-681]): the build
  depended on `Source/HyperliquidPlugin/crypto/keccak256.c`, but `Source/` is
  gitignored (it is a local Zorro install junction), so a clean clone could not
  build. Moved `keccak256.c/.h` and the single-header `bt_secp256k1.h` under
  `src/vendor/secp256k1/` (matching the existing `src/vendor/yyjson/` convention)
  with a LICENSE noting the MIT (bitcoin-core libsecp256k1) and public-domain
  (keccak) origins. Repointed CMake and `hl_crypto.cpp` at the new path.

### CI

- **Continuous integration** (`ci.yml`): builds the Dev DLL and runs unit tests
  on every push and pull request to `develop`/`main`. Proves the repo stays
  self-contained.
- **Zorro SDK headers fetched at build time**: `include/trading.h` and
  `include/variables.h` are proprietary ((c) oP group) and are not committed.
  Both CI and release workflows download the official Zorro beta distribution,
  cache it, and extract only those two headers before configure — no secret and
  no one-time setup required. Verified the `TRADE` struct ABI is byte-identical
  to the installed Zorro 3.016 headers, so CI-built DLLs are runtime-compatible.
- Earlier release-workflow fixes now combine to make tagged releases build and
  publish automatically: vcpkg pinned to a full commit SHA, and
  `VCPKG_BINARY_SOURCES=clear` to avoid the GHA binary-cache requirement.

## [2.0.5] — 2026-05-29

### Fixed

- **Reconcile double-count blocked further trading after EXTEND orders** ([OPM-680]):
  `BrokerTrade`'s `IMPORTED_` branch returned the broker's live aggregate position
  size, which double-counted when a same-side `BrokerBuy2` created a new tradeID
  alongside an existing `IMPORTED_` position. Zorro then saw e.g. `-0.30075` BTC while
  Hyperliquid held `-0.21060`, the reconcile delta exceeded the strategy's $500 HALT
  threshold, and every subsequent daily rebalance was skipped. Fixed with per-tradeID
  share accounting so each tradeID reports only its own portion. Observed on the live
  YOLO_HL_Native rebalance (2026-05-24): BTC and XRP each drifted by exactly one
  prior order's size.

### Added

- **WS send-failure diagnostics** ([OPM-681]): instruments the `connection_.send()`
  failure paths to capture IXWebSocket `readyState` and seconds-since-Open when a send
  fails. Discriminates the hypothesised causes of silent l2Book/subscription failures
  at startup (pre-Open race vs. mid-flight disconnect vs. compression error) so the
  root cause of WS reconnect storms and partial `clearinghouseState` snapshots can be
  identified from production logs.

### CI

- **vcpkg GHA binary caching disabled** ([commit 4178d84]): `VCPKG_BINARY_SOURCES=clear`
  so CMake configure no longer aborts requiring `ACTIONS_RUNTIME_TOKEN`/`ACTIONS_CACHE_URL`.
  Combined with the v2.0.4 vcpkg-SHA pin, this should let the release workflow build and
  publish automatically. (Committed after the v2.0.4 tag, so v2.0.4's CI run still failed.)

## [2.0.4] — 2026-05-21

### Added

- **Compile-time build stamp logged at `BrokerLogin`**: the version line now includes
  `__DATE__ __TIME__`, e.g. `Hyperliquid 2.0.0-Modular (build May 21 2026 22:45:41)`.
  The hardcoded `PLUGIN_VERSION` string can't reveal which DLL build is actually loaded;
  the build stamp changes on every recompile, so a stale or wrong DLL is obvious at
  runtime. Added after a stale DLL on a OneDrive-synced VPS silently ran pre-fix code.

### Fixed

- **CI release workflow failed at vcpkg setup** (`ci`): `lukka/run-vcpkg@v11` requires a
  full 40-char commit SHA for `vcpkgGitCommitId`, but the workflow passed the release tag
  `2024.11.16`. This failed every tagged release (v2.0.1–v2.0.3) before the build started,
  forcing manual publishing. Pinned to the tag's commit SHA `b2cb0da`.

## [2.0.3] — 2026-05-20

### Fixed

- **Order rejected with "User or API Wallet does not exist" on assets with trailing-zero sizes** ([OPM-677]):
  `formatSize()` formatted sizes with fixed-precision but did not strip trailing zeros
  (e.g. `0.09730`, `13.930`). Hyperliquid's server-side signature verification hashes
  the action from its canonical msgpack form, which strips trailing zeros (matching the
  Python SDK's `float_to_wire` / `Decimal.normalize()`). The hash mismatch caused ECDSA
  recovery to yield a phantom signer address — different per retry — so HL rejected the
  order. `formatPrice` already stripped trailing zeros; this aligns `formatSize` to the
  same behaviour. Observed on BTC (2026-05-19) and BNB (2026-05-20) in live rebalances.

- **`queryOrderByCloid` returned empty `oid`, blocking PENDING-order reconciliation** ([OPM-679]):
  The HL `orderStatus` response nests order fields one level deeper than the status field:
  `root.order = {"order":{...,"oid":...},"status":"open","statusTimestamp":...}`. The
  code read `oid` from `root.order` (where status lives) instead of `root.order.order`
  (where `oid` lives). The empty `oid` gated the entire PENDING-order resolution path in
  `BrokerTrade` (`if (qr.oid[0])`), leaving PENDING orders permanently stuck. The unit
  test fixtures encoded the same wrong (flattened) structure as the bug, so the test
  passed while production silently failed. Fixtures corrected to the real nested shape.

## [2.0.2] — 2026-05-13

### Fixed

- **GET_POSITION doubles positions on main-dex assets with perpDex namesakes** ([OPM-600]):
  The OPM-226 fallback in `GET_POSITION` rewrote bare coin lookups to perpDex-prefixed
  keys (e.g. `cash:BTC`, `hyna:SOL`) whenever a matching perpDex asset existed in the
  registry, even when the actual position was on main-dex. Hyperliquid's `cash`/`hyna`
  universes share coin names with main-dex assets, so the rewrite caused cache misses.
  Strategies treated existing positions as flat and doubled up on entry. Fix: prefer
  main-dex (non-perpDex) assets in the fallback; only apply the perpDex rewrite when no
  main-dex asset with that coin exists.

- **Removed stale OPM-219 fallbacks from `getPosition`** ([OPM-555]):
  Defensive prefix-stripping fallbacks introduced during OPM-219 had outlived their
  usefulness. Cache keys are now normalized at write time; an exact-key lookup is the
  only correct behavior. The old fallbacks silently returned wrong-dex positions when an
  asset existed on one dex but not the other.

### Added

- **`hl_protocol.h`** ([OPM-446]): Centralized Hyperliquid wire-format string constants
  for WS channel names, subscription types, HTTP `/info` request types, and POST action
  types. Migrates `ws_manager.cpp` dispatch to use the new constants.

### Changed

- Sanitized repository for public release: removed internal doc templates ([OPM-556]).
- Added upstream API doc links to protocol-touching headers ([OPM-453]).

## [2.0.1] — 2026-05-05

### Fixed

- **WS thread stalls causing price-cache staleness**: `BrokerMessage` (a synchronous `SendMessage` to the Zorro GUI thread) was called directly from the IXWebSocket internal thread. When the GUI was busy, the WS thread blocked for hundreds of milliseconds, starving message dispatch and causing l2Book updates to appear stale by up to 2.5 minutes. Fixed by introducing an async bounded log queue: WS threads enqueue messages non-blocking; the main thread drains the queue on each `BrokerTime`/`BrokerCommand` call.
  - Added `Logger` class (`hl_globals.h/cpp`) with producer/consumer split: `log()`/`logf()`/`enqueue()` for WS threads, `drain()` for main thread only.
  - Added bounded `Connection::poll()` dispatch (max 256 messages or 50 ms wall-clock) to prevent a burst flood from starving IXWebSocket's own receive loop.
  - Split WS log callback (`enqueueLogAsync`) from main-thread log callback (`sinkToBrokerMessage`) to prevent accidental cross-thread `BrokerMessage` calls.
  - Result: `longestTickGap` reduced from 11,297 ms → 141 ms at `SET_DIAGNOSTICS=3`; l2Book `avgMs` reduced from 16.23 ms → 0.02 ms; HTTP fallback storms eliminated.
  - All Phase 1/1.5/1.6 diagnostic instrumentation (per-iteration `HL550_PHASE` timing, per-channel handler timing in `handleMessage`, `setBidAsk` lock-wait tracking) is **runtime-gated** on `g_config.diagLevel >= 3`. Production runs (diagLevel=1) pay zero overhead; operators flip to `SET_DIAGNOSTICS=3` to re-engage full telemetry on demand.

### Changed

- Demoted `refreshPerpDexPositions: no active perpDex names` log line from level 1 → level 3. It was emitted on every `BrokerAccount` tick, flooding the new async log queue and risking real-message drops.

## [2.0.0] — 2026-04-25

First production release of the refactored plugin.

### Architecture

- 4-layer modular architecture (Foundation → Transport → Services → API) replacing the monolithic `Hyperliquid_Native.cpp`
- 17 source files organized under `src/foundation/`, `src/transport/`, `src/services/`, `src/api/`
- File size limits enforced (header ≤250 lines, impl ≤600 lines, module ≤800 lines)

### Added

- `build/build_prod_dll.bat` — production build script (CMake + vcpkg, `DEV_BUILD=OFF`)
- `.github/workflows/release.yml` — CI release workflow that builds and attaches `Hyperliquid.dll` on tag push

### WebSocket

- Migrated from WinHTTP to IXWebSocket backend with native auto-reconnect
- PerpDex `clearinghouseState` subscriptions, lazy-activated per strategy
- L2 book subscriptions with 60-second health-check fallback to HTTP

### Trading

- NFA-compliant order flow (Hedge=0 semantics, no stop-and-reverse)
- Bracket orders via separate stop-loss placement (`SET_ORDERTYPE +8`)
- `BrokerTrade` uses WS `PriceCache` for open orders

### Fixed

- PerpDex position lookup: extract `dex` directly from WS `data` object and resolve `@index` coin format via index mapping
- `BrokerCommand` trace log and HTTP-fallback `getPrice` notices raised to `diagLevel>=3`/`>=2` (were flooding logs at `diagLevel=1`)
- `BrokerAsset` now sets `priceSymbol` for `GET_PRICE` contract
- Flat positions are detected before issuing close orders to prevent Error 075
- `HL_GET_OPEN_ORDERS` queries WS `PriceCache` directly

### Testing

- Regression suite (`tests/run_unit_tests.bat`) with 21 test modules covering crypto, HTTP parsing, position tracking, trading logic, and WS cache interactions
- `SmokeTest_Dev.c` / `SmokeTest_Prod.c` integration scripts for testnet and mainnet verification

---

[Unreleased]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.0...HEAD
[2.0.0]: https://github.com/opmau/hyperliquid-zorro-plugin/releases/tag/v2.0.0
