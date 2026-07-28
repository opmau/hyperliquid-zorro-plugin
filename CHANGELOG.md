# Changelog

All notable changes to the Hyperliquid Zorro Plugin are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> `OPM-NNN` references throughout this file are the maintainer's internal issue
> tracker IDs. They are kept for historical traceability of the rationale behind
> each change; they do not resolve to a public URL.

## [Unreleased]

## [2.1.0] — 2026-07-28

Makes ALO (post-only / add-liquidity-only) execution actually reachable from a
Zorro strategy. **No ALO order had ever reached the exchange from this stack**:
all 148 live orders in the reference strategy's log went out `"tif":"Ioc"`
despite the script calling `brokerCommand(50012, "Alo")` since February. This
release fixes that and the surrounding defects that made resting orders unsafe
to use for closes, repricing, and error handling.

Also documents ALO, modify, TWAP and bracket support, which shipped
undocumented before 2.0.0.

### Fixed

- **`brokerCommand(50012, "Alo")` was silently reverted before every order**
  ([OPM-791]). Zorro auto-calls `SET_ORDERTYPE` at each order entry and can
  only derive types 0–3 from `TradeMode`, so the handler overwrote the
  script's ALO choice with `"Ioc"` microseconds before the order was built —
  and `case 4` ("Alo") was unreachable in live trading. `50012("Alo")` now
  sets a **sticky** override that the auto-call does not downgrade; it is
  released by `50012("Ioc")`/`50012("Gtc")`, at login and at logout.
  `brokerCommand(157, 4)` is treated as the same explicit intent.
- **Market orders inherited the global `"Alo"` TIF** ([OPM-794]). The order
  builder's TIF switch fell through to the global order-type string, so a
  market order — which the plugin deliberately downgrades to IOC at a
  *crossing* price (ask × 1.05) — still went out `tif:"Alo"`: a guaranteed
  post-only reject, surfaced only as `BrokerBuy2 → 0`. The request enum is now
  the sole source of truth for the wire TIF.
- **`50012` stored the caller's casing verbatim** ([OPM-794]). Only the
  msgpack signing path canonicalized, so `50012("ioc")` signed `"Ioc"` but
  serialized `"ioc"` — a hash mismatch Hyperliquid reports as
  *"User or API Wallet does not exist"* (the OPM-677 failure class). TIF
  strings are now canonicalized on ingestion and in `setOrderType()`.
- **Exchange order rejects were invisible to scripts** ([OPM-795]).
  Hyperliquid returns per-order errors in `statuses[i].error` under a
  **top-level `status:"ok"`**, which neither the error branch nor the status
  parser read; every reject collapsed into "No order ID in exchange response".
  The reject text is now parsed, logged as `Order REJECTED: <text>`, stored on
  the order state, and exposed via the new **`50023`** command so a strategy
  can tell "post-only would cross → reprice one tick back" from
  "margin/signing failure → stop trading this asset".
- **`BrokerSell2` reported an unfilled resting close as fully closed**
  ([OPM-792]). A resting (ALO/GTC) close returns `filledSize = 0`, and the old
  code then reported `abs(amount)` — a full close — through five channels,
  mutating the position cache and the Zorro-close ledger along the way. Zorro
  wrote the position off its books while the contracts were still on the
  exchange. It now reports only what actually filled, and moves no cache or
  ledger state until a fill is confirmed.
- **A resting close order was uncancellable from script** ([OPM-792]).
  `BrokerSell2` placed the close through a fresh internal trade ID it never
  returned, so `DO_CANCEL` targeted the already-filled *entry* oid. The close
  order is now linked to the position; `DO_CANCEL(tradeId)` cancels the
  working close order while one exists, and the link clears once it fills.
- **Prices were rounded symmetrically, across the spread** ([OPM-796]).
  A passive buy limit at the bid could round *up* through the spread (and a
  sell limit *down*), producing post-only rejects that looked random.
  Rounding is now side-aware: buys floor, sells ceil.
- **Hyperliquid's integer-price exemption was not implemented** ([OPM-796]).
  Prices were snapped to a 5-significant-figure grid, so above $100k every
  $1 level on BTC collapsed onto a $10 grid and `123456` became `123460` —
  even though HL accepts every integer price regardless of significant
  figures. $1-level maker quotes on BTC are now expressible.
- **A partially-filled-then-cancelled order lost its partial** ([OPM-798]).
  `BrokerTrade` checked "cancelled" before "anything still open" and returned
  `NAY-1`, telling Zorro the order never existed — so it discarded contracts
  the exchange still held. The HTTP reconciliation path also zeroed
  `filledSize` on cancel. Both now preserve the partial; only a cancel with
  nothing filled is `NAY-1`.
- **Login left two order-type sources of truth disagreeing** ([OPM-798]).
  `BrokerLogin` set `g_config.orderType = "Gtc"` without calling
  `trading::setOrderType()`, whose static stayed `"Ioc"`. Both are now set to
  `"Ioc"` — the value Zorro's auto `SET_ORDERTYPE(0)` picks anyway.
- **`50020` (open-order count) matched symbols unnormalized and had no HTTP
  fallback** ([OPM-798]). PerpDex symbols never matched the cache key, and a
  genuinely resting order reported 0 whenever the WebSocket cache was empty.
  Symbols are now normalized the same way `SET_SYMBOL` does, with an
  authoritative HTTP fallback.
- **Cancels built from synthetic trade IDs submitted a cancel for oid 0**
  ([OPM-797]). `PENDING_…`, `RESUMED_…`, `IMPORTED_…` and `DRY_RUN` all
  become 0 through `_atoi64`, and the resulting well-formed cancel reported
  success. These are now rejected with a clear log line.

### Added

- **`50044 HL_MODIFY_BY_TRADEID`** — scalar C-ABI reprice reachable from
  Lite-C ([OPM-793]). `batchModify` has been fully implemented and
  queue-priority preserving since OPM-80, but its only entry point (`50042`)
  takes a `ModifyRequest` containing `std::string` members, which Lite-C
  cannot construct — so no script had ever called it. The new command takes
  `double[3] = {tradeId, newPrice, newSize}` (size ≤ 0 keeps the current
  size) and resolves coin/side/oid from the tracked order. Returns `1` on
  success, `0` on rejection, `-1` for an unknown trade and `-2` when the order
  already filled or was cancelled, so a reprice loop can tell a lost race from
  a real failure. Replaces cancel → settle-poll → re-place: one round trip
  instead of two, with no fill-race window.
- **`50023 HL_GET_LAST_ORDER_ERROR`** — returns the class of the most recent
  order reject (`0` none, `1` post-only-would-match, `2` margin, `3` other).
  Pass a `char` buffer of at least 256 bytes as the parameter to also receive
  the exchange's verbatim text.
- **`DO_CANCEL(0)`** now cancels every resting order for the current
  `SET_SYMBOL`, or account-wide when no symbol is selected ([OPM-797]).
  Previously an explicit "not implemented". Orders are sourced from the
  exchange rather than the local trade map, so an order left behind by a
  crashed session is reachable after a Zorro restart — that case previously
  required manual intervention in the Hyperliquid web UI.
- The modify path now adopts the post-modify oid returned by the exchange, so
  a reprice loop cannot leave the trade map pointing at a retired oid.

### Documentation

- Recorded that the plugin **ignores `SET_WAIT`**: `BrokerBuy2` never blocks or
  polls and returns immediately on a "resting" response with fill = 0. The
  Zorro manual's blocking convention does not apply — strategies must poll
  `GET_POSITION` for fills ([OPM-798]).
- Documented ALO, modify, TWAP and bracket support, which shipped before
  2.0.0 without appearing in this file.

### Tests / Infrastructure

- New `test_alo_enablement` module: 49 regression tests covering all eight
  changes, run as step 22 of `run_unit_tests.bat`. The rounding, TIF
  canonicalization, error-classification and response-parsing tests exercise
  the real implementations rather than simulations.
- Split four files that had grown past the repo's size limits, with no
  behaviour change: `hl_broker_trade.cpp` → `+ hl_broker_trade_query.cpp`
  (execution vs. status query), `hl_broker_commands.cpp` →
  `+ hl_broker_commands_hl.cpp` (standard Zorro modes vs. the 500xx range),
  and extracted `hl_trading_response.*` (shared exchange-response parsing and
  reject tracking) and `hl_trading_openorders.*` (exchange-sourced resting
  order queries).

## [2.0.7] — 2026-06-17

Fixes a position-tracking defect that corrupted Zorro's trade ledger and
froze the daily rebalance. Validated by ~1 week of live trading before release.

### Fixed

- **BrokerTrade reported the wrong open size after a partial close or a
  second same-side extend** ([OPM-733]). Zorro's automatic BrokerTrade
  fill-poll overwrites its trade ledger with the plugin's return value; the
  plugin returned a stale/inflated size via two paths, so Zorro's books
  diverged from Hyperliquid and the strategy's reconcile guard halted every
  rebalance until a manual `.trd` resync:
  - Mapped trades now record Zorro-driven closes in a new `closedSize` field
    and BrokerTrade reports `filledSize - closedSize` (net open), instead of
    re-reporting the entry order's gross fill (e.g. 61796 after a reduce to
    58508).
  - The OPM-680 pre-extend share snapshot now runs only on the sole→multi
    tracker transition, via a shared `hasOtherSameSideTracker()` predicate
    that also backs BrokerTrade's reporting — so a second consecutive extend
    no longer double-counts a sibling's fill into an imported trade's share
    (81910 → 87741).
  - A sub-lot epsilon on the net-open size prevents a full close that leaves
    a tiny float residual from reporting a phantom 1-lot trade.

### Tests / Infrastructure

- Regression tests replaying both OPM-733 incidents with the exact live-log
  values, plus sub-lot residual cases.
- Fixed a linker failure that had silently disabled the `ws_parsers` unit
  test module since OPM-550 ([OPM-734]).
- Fixed `run_unit_tests.bat` aborting silently after 3 modules (vcvars
  environment overflow) and qualified inner test-exe launches so the suite
  runs on machines that exclude the CWD from the executable search path
  ([OPM-735]).

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

[Unreleased]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.1.0...HEAD
[2.1.0]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.7...v2.1.0
[2.0.7]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.6...v2.0.7
[2.0.6]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.5...v2.0.6
[2.0.5]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.4...v2.0.5
[2.0.4]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.3...v2.0.4
[2.0.3]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.2...v2.0.3
[2.0.2]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.1...v2.0.2
[2.0.1]: https://github.com/opmau/hyperliquid-zorro-plugin/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/opmau/hyperliquid-zorro-plugin/releases/tag/v2.0.0
