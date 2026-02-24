# Hyperliquid Plugin Refactor — V2 Checklist

## How to Use This With Claude Code

Claude Code reads `CLAUDE.md` at session start — you don't need to tell it. But for the first task, prompt it to also read the plan:

```
Read docs/REFACTOR_PLAN.md. We're starting Phase 1: Foundation.
First task: Extract hl_types.h. Follow the extraction sequence in the plan.
```

For subsequent tasks after a `/clear`:

```
Read docs/REFACTOR_PLAN.md. We're on Phase [N]: [Name].
Current task: [Step description]. Follow the extraction sequence in the plan.
```

**Key workflow per step:**
1. Start session with the prompt above
2. Let Claude Code do the extraction
3. Verify it compiles: `cmake --build . --config Release`
4. Commit with a clear message
5. `/clear` before next step

**For complex steps (WebSocket split, Services), use a sub-branch:**
```
git checkout -b refactor/ws-split   # before step 8
git checkout -b refactor/services   # before step 9
```
Then merge back to `refactor/modularize` when the group is done.

---

## Pre-Flight

- [ ] **0a.** Ensure current code compiles and passes smoke test on `main`
- [ ] **0b.** Capture baseline snapshots (meta response, l2book response, account response)
- [ ] **0c.** Create branch and safety tag:
  ```bash
  git checkout main && git pull
  git checkout -b refactor/modularize
  git tag pre-refactor-backup
  ```
- [ ] **0d.** Create directory structure:
  ```bash
  mkdir -p src/{foundation,transport,services,api}
  mkdir -p tests
  ```
- [ ] **0e.** Copy legacy files to `legacy/` for reference (don't delete originals yet)

---

## Phase 1: Foundation (Day 1)

### Step 1 — Extract `hl_types.h`
- [ ] Source: `Hyperliquid_Native.cpp` lines ~50-150 (struct definitions)
- [ ] Create `src/foundation/hl_types.h` — pure data structs, no logic, no dependencies
- [ ] Contents: `OrderState`, `PriceData`, `AssetInfo`, `Position`, `AccountState`, enums
- [ ] Verify: compiles standalone
- [ ] Commit: `refactor: extract hl_types.h`
- [ ] `/clear`

### Step 2 — Extract `hl_config.h`
- [ ] Source: scattered `#defines` and constants throughout
- [ ] Create `src/foundation/hl_config.h` — consolidate endpoints, timeouts, limits
- [ ] Contents: `MAINNET_REST`, `TESTNET_REST`, `WS` URLs, `HTTP_TIMEOUT_MS`, `MAX_ASSETS`, etc.
- [ ] Verify: compiles standalone
- [ ] Commit: `refactor: extract hl_config.h`
- [ ] `/clear`

### Step 3 — Extract `hl_globals`
- [ ] Source: `GLOBAL G` struct and all `g_*` pointers/maps
- [ ] Create `src/foundation/hl_globals.h` and `hl_globals.cpp`
- [ ] Contents: `RuntimeConfig`, `AssetRegistry`, `Logger`, extern declarations
- [ ] Split the God Object: `G` struct → focused state containers
- [ ] Verify: compiles and links
- [ ] Commit: `refactor: extract hl_globals`
- [ ] `/clear`

### Step 4 — Extract `hl_utils`
- [ ] Source: helper functions (`normalize_coin`, JSON parsing, time, rounding)
- [ ] Create `src/foundation/hl_utils.h` and `hl_utils.cpp` — pure functions, no state
- [ ] Contents: `normalizeAssetName()`, `jsonGetString()`, `jsonGetDouble()`, `nowMillis()`, `roundToDecimals()`, `formatSize()`
- [ ] Verify: compiles, unit tests pass (`tests/test_utils.cpp`)
- [ ] Commit: `refactor: extract hl_utils`
- [ ] `/clear`

### Phase 1 Checkpoint
- [ ] Full build succeeds: `cmake --build . --config Release`
- [ ] Run in Zorro: `BrokerOpen` returns correct version
- [ ] Tag: `git tag refactor-phase-1-complete`

---

## Phase 2: Crypto (Day 1-2)

### Step 5 — Extract `hl_crypto`
- [ ] Source: EIP-712 signing, secp256k1 usage, address derivation
- [ ] Create `src/foundation/hl_crypto.h` and `hl_crypto.cpp`
- [ ] Contents: `init()`, `cleanup()`, `deriveAddressFromPrivateKey()`, `buildOrderAction()`, `signTypedData()`, `hexEncode()`, `keccak256()`
- [ ] Dependencies: existing `crypto/` folder (eip712, keccak256, bt_secp256k1) — leave as-is
- [ ] Verify: sign test message, verify known signature vector (`tests/test_crypto.cpp`)
- [ ] Commit: `refactor: extract hl_crypto`
- [ ] `/clear`

### Phase 2 Checkpoint
- [ ] Build succeeds
- [ ] Zorro: `BrokerLogin` succeeds (address derivation works)
- [ ] Tag: `git tag refactor-phase-2-complete`

---

## Phase 3: HTTP (Day 2)

### Step 6 — Extract `hl_http`
- [ ] Source: `send_http()`, `build_url()`, `info_post()`, `exchange_post()`
- [ ] Create `src/transport/hl_http.h` and `hl_http.cpp` — stateless, thread-safe
- [ ] Contents: `Response` struct, `post()`, `get()`, `infoPost()`, `exchangePost()`
- [ ] Verify: can fetch meta endpoint
- [ ] Commit: `refactor: extract hl_http`
- [ ] `/clear`

### Phase 3 Checkpoint
- [ ] Build succeeds
- [ ] Zorro: `BrokerAsset` returns bid/ask (HTTP price fetch works)
- [ ] Tag: `git tag refactor-phase-3-complete`

---

## Phase 4: WebSocket Split (Day 2-3)

> **Use a sub-branch:** `git checkout -b refactor/ws-split`

### Step 7a — Extract `ws_types.h`
- [ ] Source: `ws_manager.h` lines ~1-100
- [ ] Create `src/transport/ws_types.h` — enums, callback typedefs, subscription state
- [ ] Contents: `ConnectionState`, `SubscriptionState`, callback types
- [ ] Verify: compiles standalone
- [ ] Commit: `refactor: extract ws_types.h`
- [ ] `/clear`

### Step 7b — Extract `ws_price_cache`
- [ ] Source: price caching logic from `ws_manager.h`
- [ ] Create `src/transport/ws_price_cache.h` and `ws_price_cache.cpp` — completely standalone
- [ ] Contents: `PriceCache` class with thread-safe `setBidAsk()`, `get()`, `isFresh()`, `clear()`
- [ ] Verify: unit tests pass (`tests/test_price_cache.cpp`)
- [ ] Commit: `refactor: extract ws_price_cache`
- [ ] `/clear`

### Step 7c — Extract `ws_connection`
- [ ] Source: low-level WinHTTP WebSocket handling from `ws_manager.h`
- [ ] Create `src/transport/ws_connection.h` and `ws_connection.cpp`
- [ ] Contents: `Connection` class — `connect()`, `disconnect()`, `send()`, `poll()`, state machine
- [ ] Verify: compiles and links
- [ ] Commit: `refactor: extract ws_connection`
- [ ] `/clear`

### Step 7d — Extract `ws_manager`
- [ ] Source: orchestration logic from `ws_manager.h`
- [ ] Create `src/transport/ws_manager.h` and `ws_manager.cpp`
- [ ] Contents: `WebSocketManager` class — `start()`, `stop()`, subscription management, message routing, threads
- [ ] Dependencies: `ws_connection`, `ws_price_cache`, `ws_types`
- [ ] Verify: compiles and links
- [ ] Commit: `refactor: split ws_manager.h into 4 focused modules`
- [ ] `/clear`

### Merge sub-branch
- [ ] `git checkout refactor/modularize && git merge refactor/ws-split`
- [ ] `git branch -d refactor/ws-split`

### Phase 4 Checkpoint
- [ ] Build succeeds
- [ ] Zorro: WebSocket connects, prices stream (price changes over 3-5 seconds)
- [ ] Tag: `git tag refactor-phase-4-complete`

---

## Phase 5: Services (Day 3-4)

> **Use a sub-branch:** `git checkout -b refactor/services`

### Step 8a — Create `hl_market_service`
- [ ] Create `src/services/hl_market_service.h` and `hl_market_service.cpp`
- [ ] Contents: `MarketService` class — `getPrice()` (WS first, HTTP fallback), `getCandles()`, `refreshMeta()`, `getAsset()`
- [ ] Wraps: `hl_http` + `ws_price_cache` + meta caching logic
- [ ] Commit: `refactor: create hl_market_service`
- [ ] `/clear`

### Step 8b — Create `hl_trading_service`
- [ ] Create `src/services/hl_trading_service.h` and `hl_trading_service.cpp`
- [ ] Contents: `TradingService` class — `placeOrder()`, `cancelOrder()`, `getOrderStatus()`, order tracking
- [ ] Wraps: `hl_crypto` (signing) + `hl_http` (exchange_post) + order state management
- [ ] Commit: `refactor: create hl_trading_service`
- [ ] `/clear`

### Step 8c — Create `hl_account_service`
- [ ] Create `src/services/hl_account_service.h` and `hl_account_service.cpp`
- [ ] Contents: `AccountService` class — `getAccountState()`, `getEquity()`, `getPosition()`, `getAllPositions()`
- [ ] Wraps: `hl_http` + WS account data
- [ ] Commit: `refactor: create hl_account_service`
- [ ] `/clear`

### Merge sub-branch
- [ ] `git checkout refactor/modularize && git merge refactor/services`
- [ ] `git branch -d refactor/services`

### Phase 5 Checkpoint
- [ ] Build succeeds
- [ ] Zorro: can see positions/orders, equity > 0
- [ ] Tag: `git tag refactor-phase-5-complete`

---

## Phase 6: Broker API Adapter (Day 4)

### Step 9 — Create `hl_broker.cpp`
- [ ] Source: all `Broker*` functions from `Hyperliquid_Native.cpp`
- [ ] Create `src/api/hl_broker.cpp` — thin adapter, calls services only
- [ ] This is the ONLY file that knows about Zorro's API
- [ ] Contents: `DllMain`, `BrokerOpen`, `BrokerLogin`, `BrokerAsset`, `BrokerAccount`, `BrokerBuy2`, `BrokerSell2`, `BrokerTrade`, `BrokerHistory2`, `BrokerCommand`
- [ ] Target: ~600 lines (down from ~2800 in the monolith)
- [ ] Verify: full Zorro integration test
- [ ] Commit: `refactor: create hl_broker.cpp adapter`
- [ ] `/clear`

### Phase 6 Checkpoint
- [ ] Build succeeds
- [ ] Zorro: full login → asset → price → account flow works
- [ ] Tag: `git tag refactor-phase-6-complete`

---

## Phase 7: Cleanup (Day 5)

### Step 10 — Remove Legacy Files
- [ ] Move `Hyperliquid_Native.cpp` to /legacy folder
- [ ] Move old `ws_manager.h` (replaced by 4 transport files) to legacy folder
- [ ] Verify build still works without /legacy files
- [ ] Commit: `refactor: remove legacy files`

### Step 11 — Final Polish
- [ ] Update `CMakeLists.txt` (remove any legacy references)
- [ ] Update `build.bat` if needed
- [ ] Clean up duplicate includes, dead code
- [ ] Add `#pragma once` to all headers
- [ ] Verify all file headers match documentation template
- [ ] Commit: `refactor: cleanup and polish`

### Full Regression Test
- [ ] Run `ZorroCheckpoint.c` — all phases pass
- [ ] Run `ZorroFullTest.c` — all 10 tests pass
- [ ] Unit tests all pass: `build/Release/tests.exe`
- [ ] Verify on testnet: login, fetch prices, check account, place+cancel test order

### Merge to Main
- [ ] `git checkout main`
- [ ] `git merge refactor/modularize`
- [ ] `git tag refactor-complete`
- [ ] `git push origin main --tags`

---

## Quick Reference

```
LAYER DEPENDENCIES (allowed direction: →)
  Foundation → Transport → Services → API

FILE SIZE LIMITS
  Headers: ≤150 lines
  Implementations: ≤400 lines
  Total per module: ≤500 lines

COMMIT MESSAGE FORMAT
  refactor: extract <module>
  refactor: split ws_manager into components
  refactor: create <service>
  fix: <module> - <description>
  test: add <module> unit tests

CLAUDE CODE WORKFLOW
  1. Prompt with plan reference + current task
  2. Let it extract
  3. Verify compilation
  4. Commit
  5. /clear
  6. Next task
```

---

## Rollback

**Abandon single step:** `git checkout -- <files>`

**Abandon a phase:** `git reset --hard refactor-phase-N-complete`

**Abandon everything:** `git checkout main && git branch -D refactor/modularize`

**Emergency hotfix during refactor:**
```bash
git stash
git checkout main
git checkout -b hotfix/critical-bug
# ... fix ...
git checkout main && git merge hotfix/critical-bug
git checkout refactor/modularize
git stash pop
git merge main
```
