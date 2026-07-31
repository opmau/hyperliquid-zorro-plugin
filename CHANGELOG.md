# Changelog

All notable changes to the Hyperliquid Zorro Plugin are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> `OPM-NNN` references throughout this file are the maintainer's internal issue
> tracker IDs. They are kept for historical traceability of the rationale behind
> each change; they do not resolve to a public URL.

## [Unreleased]

### Fixed

- **Account balance on unified and portfolio-margin accounts.** `BrokerAccount`
  reported `clearinghouseState.marginSummary.accountValue`, which covers perps
  only — and only on the main dex. On an account using Hyperliquid's unified or
  portfolio-margin abstraction, a single spot USDC pool collateralizes
  everything, so that figure is not the account's equity. The plugin now reports
  the spot USDC `total`, which Hyperliquid marks to market continuously and
  which already includes unrealized PnL from every perp dex.

  **This raises the balance Zorro sees, so a strategy sizing from `Balance` or
  `Equity` will trade larger.** Check your position sizing before deploying.
  Two live examples: an account with 10 open positions went from 15,372 to
  24,114 (+57%), and an account funded entirely on the spot side went from `0`
  to its true 806.55.

  A balance of `0` also used to trigger the zero-balance guard, which halts the
  strategy with a message blaming the wallet address — so a correctly configured
  account holding only spot collateral could not trade at all.

  Accounts on the `disabled` and `default` abstraction modes are unaffected in
  substance: spot and perps remain separate pools and are still summed.

  Staked HYPE is deliberately **not** counted. Hyperliquid's own `portfolio`
  endpoint includes it, but it is delegated to a validator and cannot back a
  perp position; counting it would over-report tradeable equity.

- **`default` account abstraction mode was not recognized.** Only `disabled`,
  `unifiedAccount` and `portfolioMargin` were matched, so `default` fell through
  to "unknown" and was then treated as unified. The mode is also now queried on
  every login path — a login that fell back to HTTP used to skip the query
  entirely. The balance calculation no longer depends on the mode string at all:
  it keys off whether the spot response carries
  `tokenToAvailableAfterMaintenance`, which is present exactly when spot
  collateralizes perps.

- **JSON integers were parsed as `0.0`.** The shared `getDouble`/`valToDouble`
  helpers used yyjson's `get_real`, which returns `0.0` for a whole number such
  as `{"token":150}`. Mostly masked because Hyperliquid encodes its numbers as
  strings, but it made a portfolio-margin account read another token's
  collateral figure as its own.

### Added

- `Balance::freeCollateral` — equity available after maintenance margin. On
  unified accounts this is Hyperliquid's own `tokenToAvailableAfterMaintenance`
  figure rather than a derived one. Note that the raw `withdrawable` field reads
  `0.0` on unified accounts regardless of how much equity is free; it is no
  longer a usable proxy for available funds.

## [2.1.0] — 2026-07-28

Adds working post-only (ALO) maker execution, reports exchange order rejects to
strategies, and makes resting orders safe to use for closing and repricing.

**Upgrading:** this release changes fill reporting and cancellation semantics.
Read [Changed](#changed) before deploying to a live strategy.

### Added

- **`50044 HL_MODIFY_BY_TRADEID`** — reprice or resize a resting order from
  Lite-C:

  ```c
  var params[3];
  params[0] = tradeID; params[1] = newPrice; params[2] = 0;  // size <= 0 keeps current
  int r = brokerCommand(50044, params);
  ```

  Uses Hyperliquid's `batchModify`, which preserves queue priority, so a reprice
  costs one round trip instead of the two a cancel-and-replace needs, and leaves
  no window for the order to fill in between. Returns `1` on success, `0` if the
  exchange rejected the modify, `-1` for an unknown trade ID, and `-2` if the
  order had already filled or been cancelled — so a reprice loop can tell a lost
  race from a genuine failure. The pre-existing `50042` takes a struct
  containing `std::string` members and remains callable only from C++.

- **`50023 HL_GET_LAST_ORDER_ERROR`** — the class of the most recent order
  reject: `0` none, `1` post-only would have matched, `2` insufficient margin,
  `3` other. Pass a `char` buffer of at least 256 bytes to also receive the
  exchange's message. This lets a strategy distinguish "reprice one tick back"
  from "stop trading this asset"; previously both surfaced only as
  `BrokerBuy2 == 0`.

- **`DO_CANCEL(0)`** — cancels all resting orders for the current `SET_SYMBOL`,
  or account-wide when no symbol is selected. Orders are read from the exchange
  rather than from local state, so orders left behind by a previous session are
  cancellable after a restart.

### Fixed

- **The post-only order type was reset before every order.** Zorro calls
  `SET_ORDERTYPE` automatically at each order entry and derives only types 0–3
  from `TradeMode`, which overwrote an order type selected with
  `brokerCommand(50012, "Alo")`. `50012("Alo")` now sets a sticky override that
  the automatic call does not change; `50012("Ioc")`, `50012("Gtc")`, login and
  logout release it.

- **Market orders inherited a configured `"Alo"` time-in-force.** Market orders
  are priced to cross the spread, so sending them post-only guaranteed a reject.
  The order request now determines the time-in-force instead of the global
  setting.

- **`50012` accepted mixed-case values without canonicalizing them.** A value
  such as `"ioc"` was signed as `"Ioc"` but serialized as `"ioc"`, producing a
  signature mismatch that Hyperliquid reports as the unrelated-sounding
  "User or API Wallet does not exist".

- **Exchange order rejects were not reported.** Hyperliquid returns per-order
  errors in `statuses[i].error` with a top-level `status` of `"ok"`, which the
  response parser did not read, so every reject appeared as a generic failure.
  Rejects are now parsed, logged as `Order REJECTED: <reason>`, and readable
  via `50023`.

- **`BrokerSell2` reported an unfilled resting close as fully closed.** A
  resting close order returns no fill, but the previous code reported the full
  requested amount and updated the position cache and close ledger to match, so
  Zorro's records diverged from the exchange while the contracts were still
  open. It now reports only the amount actually filled, and updates no cached
  state until a fill is confirmed.

- **A resting close order could not be cancelled from a script.**
  `BrokerSell2` placed the close order under an internal ID it did not return,
  so `DO_CANCEL` targeted the already-filled entry order.

- **Prices were rounded symmetrically.** A passive buy limit could be rounded up
  through the spread, and a sell limit down through it, causing post-only
  rejects that appeared random. Buy limits now round down and sell limits round
  up.

- **Hyperliquid's integer-price rule was not implemented.** Prices were snapped
  to a 5-significant-figure grid, so above 100000 only every tenth integer was
  expressible even though Hyperliquid accepts any integer price. Integer prices
  now pass through unchanged, making $1-level quotes on high-priced assets
  usable.

- **A partially filled order that was then cancelled reported no fill,** so
  Zorro discarded contracts that existed on the exchange. Cancelled orders now
  report any partial fill; only a cancel with no fill is reported as a
  non-existent order.

- **`50020` did not normalize symbols and had no HTTP fallback.** PerpDex
  symbols never matched, and a resting order reported a count of zero whenever
  the WebSocket cache was empty.

- **Cancelling an order the plugin was not tracking submitted a cancel for
  order ID 0** and reported success. Such requests are now rejected.

- **`BrokerLogin` left two internal order-type settings inconsistent** until the
  first `SET_ORDERTYPE`.

### Changed

These affect strategy behaviour. Review before upgrading a live system.

- **`BrokerSell2` now reports a fill of 0 for a resting close order,** so the
  trade stays open in Zorro until the close actually fills. This is the intended
  correction, and it is what makes post-only closes safe — but a strategy that
  treated a `BrokerSell2` return as confirmation that a position was closed must
  now poll `GET_POSITION` instead.
- **`DO_CANCEL(tradeId)` cancels a resting close order for that trade, if one
  exists,** rather than the filled entry order.
- **The default order type at login is now `Ioc` instead of `Gtc`,** matching
  what Zorro sets automatically at the first order.
- **The plugin does not implement `SET_WAIT`.** `BrokerBuy2` returns as soon as
  the exchange responds, reporting a fill of 0 for an order that rests. The
  Zorro manual's blocking convention does not apply; strategies must poll
  `GET_POSITION` for fills. This was already the behaviour and is now documented.

### Documentation

- Added [docs/BROKERCOMMAND_REFERENCE.md](docs/BROKERCOMMAND_REFERENCE.md),
  covering every `brokerCommand` mode the plugin implements and the behaviour
  contracts a strategy must design against, including a worked example of
  running a maker order end to end. This also documents the ALO, atomic modify,
  TWAP and bracket support that was available but undocumented before 2.0.0.

### Internal

- Regression tests covering each change in this release (50 cases), plus a
  guard against a price rounding to zero on assets that permit no decimal
  places.
- `cancelAllOrders` is bounded per call and reports explicitly when orders
  remain, rather than issuing an unbounded burst of requests.
- Several modules split to stay within the project's file-size limits; no
  behaviour change.

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
  share accounting so each tradeID reports only its own portion. Observed in live
  trading on a daily rebalance, where two assets each drifted by exactly one prior
  order's size.

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
