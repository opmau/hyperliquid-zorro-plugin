# Adding Features

How to extend the plugin without breaking the architecture.

---

## Adding a New BrokerCommand

`BrokerCommand` is the extension point for non-standard operations. Custom command codes 50000+ are plugin-defined.

### Steps

1. **Define the constant** in `src/api/hl_broker_internal.h`:
   ```cpp
   #define HL_GET_FUNDING_RATE  50031
   ```

2. **Add the case** in `src/api/hl_broker_commands.cpp`:
   ```cpp
   case HL_GET_FUNDING_RATE: {
       const char* coin = (const char*)parameter;
       if (!coin || !*coin) return 0;

       double rate = hl::market::getFundingRate(coin);  // Service call
       return rate;
   }
   ```

3. **Implement the service function** (if new logic is needed):
   - Add declaration to the appropriate service header (`hl_market_service.h`)
   - Implement in the corresponding `.cpp` file
   - Follow the layer rules: service calls transport, never API

4. **Call from Lite-C strategy**:
   ```c
   // In a Zorro Lite-C script
   #define HL_GET_FUNDING_RATE  50031
   var rate = brokerCommand(HL_GET_FUNDING_RATE, "BTC");
   ```

5. **Add a test** in `tests/unit/` (see [TESTING.md](TESTING.md))

6. **Document the command** in [ARCHITECTURE.md](ARCHITECTURE.md) BrokerCommand reference table

### Parameter Conventions

`BrokerCommand(int mode, intptr_t parameter)` returns `double`.

| Parameter type | How to pass | How to read |
|---------------|------------|-------------|
| Integer | `brokerCommand(CODE, 42)` | `int val = (int)parameter;` |
| String | `brokerCommand(CODE, "BTC")` | `const char* s = (const char*)parameter;` |
| Double pointer | `brokerCommand(CODE, &myVar)` | `double u = *(double*)parameter;` |
| No param | `brokerCommand(CODE, 0)` | Check `parameter == 0` |

Return value is always `double`. Return 0 for failure, non-zero for success (unless the command naturally returns a numeric value like a price or count).

### Existing Custom Commands

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50001 | `HL_EXPORT_ASSETS` | file path string | 1 on success; writes CSV for BTC/ETH/SOL |
| 50002 | `HL_EXPORT_META` | file path string | asset count; writes CSV for all assets |
| 50003 | `HL_EXPORT_ACCOUNT` | file path string | 1 on success; writes Accounts.csv row |
| 50010 | `HL_VALIDATE_PRICES` | 0 | 1 if BTC+ETH+SOL prices OK |
| 50011 | `HL_ENABLE_WEBSOCKET` | 0/1 | 1 on success |
| 50012 | `HL_SET_ORDER_TYPE` | "Ioc"/"Gtc"/"Alo" | 1 on success |
| 50020 | `HL_GET_OPEN_ORDERS` | symbol or 0 | count |
| 50021 | `HL_SET_ACCOUNT_MODE` | 0=API/1=vault | 1 on success |
| 50030 | `HL_FORCE_WS_DISCONNECT` | 0 | 1 on success (debug only) |
| 50031 | `HL_GET_FUNDING_RATE` | coin string or 0 | hourly funding rate (double) |

---

## Adding a New API Endpoint

### Which layer?

| Scenario | Layer |
|----------|-------|
| Raw HTTP request/response, no business logic | Transport (`hl_http.cpp`) |
| Business logic on top of HTTP | Services (`hl_*_service.cpp`) |
| Called directly by Zorro | API layer via existing service |

### Steps

1. **Add the HTTP call** in the appropriate service. Use `hl::http::infoPost()` for read queries, `hl::http::exchangePost()` for signed actions:

   ```cpp
   // In hl_market_service.cpp
   double hl::market::getFundingRate(const char* coin) {
       char json[256];
       sprintf_s(json, "{\"type\":\"fundingHistory\",\"coin\":\"%s\",\"startTime\":0}", coin);

       auto response = hl::http::infoPost(json);
       if (response.failed()) return 0;

       // Parse response...
       return rate;
   }
   ```

2. **Parse the response.** The transport layer provides two parsing approaches:

   - **yyjson** (preferred for new code): Use `json_helpers.h` wrappers. Available in transport and service layers.
   - **Naive string search** (legacy pattern): `strstr()` + manual extraction. Fragile but used in older code.

3. **Add to the service header** so other layers can call it.

4. **Expose via BrokerCommand** (if the strategy needs it) or use from existing broker functions.

### PerpDex Awareness

If the endpoint needs to support perpDex assets, use `hl::http::infoPostPerpDex()` which injects the perpDex venue into the request URL:

```cpp
auto response = hl::http::infoPostPerpDex(json, perpDex);
// Uses /info?perpDex=xyz if perpDex is non-empty
```

---

## Adding a New Asset Type

The plugin currently supports three asset types:

| Type | Zorro symbol | API format | Index range |
|------|-------------|------------|-------------|
| Standard perps | `BTC-USDC` | `BTC` | 0-999 |
| PerpDex perps | `GOLD-USDC_xyz` | `xyz:GOLD` | 110000+ |
| Spot | `BTC/USDC` or `@107` | `BTC/USDC` or `@107` | 10000+ |

### Symbol Parsing

All symbol parsing flows through `parsePerpDex()` in `hl_broker.cpp`:

```
"BTC-USDC"        → perpDex="", coin="BTC"
"GOLD-USDC_xyz"   → perpDex="xyz", coin="GOLD"
"BTC/USDC"        → perpDex="", coin="BTC/USDC"  (spot: kept as-is)
"@107"             → perpDex="", coin="@107"       (spot: kept as-is)
```

Then `buildCoinForApi(perpDex, coin)` produces the API-ready name:
- `""` + `"BTC"` = `"BTC"`
- `"xyz"` + `"GOLD"` = `"xyz:GOLD"`

### Metadata Loading

Asset metadata is fetched at login via `hl::meta::fetchMeta()` (perps), `hl::meta::fetchAllPerpDexMeta()` (perpDex), and `hl::meta::fetchSpotMeta()` (spot). Each populates `g_assets` with `AssetInfo` entries containing `szDecimals`, `pxDecimals`, `minSize`, `maxLeverage`, etc.

To add a new asset class:
1. Add a fetch function in `hl_meta.cpp` or a new `hl_meta_*.cpp` file
2. Call it from `market::refreshMeta()`
3. Ensure `parsePerpDex()` can route the symbol correctly
4. Add test coverage for the index scheme

---

## Modifying Order Signing

The order signing pipeline is the most sensitive part of the plugin. Changes here can cause **all orders to be rejected** by the exchange.

### Pipeline

```
OrderRequest → OrderAction → MsgPack → keccak256 → EIP-712 → secp256k1 sign
```

Each stage has specific requirements:

| Stage | File | Critical constraint |
|-------|------|-------------------|
| OrderAction | `hl_eip712.h` | Field names must exactly match Hyperliquid's type hashes |
| MsgPack | `hl_msgpack.cpp` | Key ordering must match Python SDK exactly |
| keccak256 | `hl_crypto.cpp` | Standard Ethereum keccak256 (not SHA3-256) |
| EIP-712 | `hl_eip712.cpp` | source = "a" (mainnet) / "b" (testnet) |
| secp256k1 | `hl_crypto.cpp` | Recovery flag (v = 27/28) |

### Before Changing Anything

1. Run the EIP-712 regression test: `compile_eip712_source_test.bat`
2. Verify against the [Hyperliquid Python SDK](https://github.com/hyperliquid-dex/hyperliquid-python-sdk) -- it's the reference implementation
3. Test on testnet first -- signing errors silently reject orders

### Common Pitfalls

- **MsgPack key order matters.** Hyperliquid hashes the msgpack bytes. Different key ordering = different hash = rejected signature.
- **The `source` field** is not the network name. It's a single character: `"a"` or `"b"`. This was a real bug (OPM-22).
- **Nonce must be monotonically increasing.** The plugin uses a timestamp-based nonce with atomic increment.
- **Trigger orders** use a different msgpack encoding (`packTriggerOrderAction`) with additional fields (`tpsl`, `triggerPx`, `isMarket`).

---

## File Size Limits

When adding code, keep files within these limits:

| Type | Limit |
|------|-------|
| Header files | 250 lines |
| Implementation files | 600 lines |
| Total per module (h + cpp) | 800 lines |
| Hard ceiling | 1000 lines |

If a file would exceed 600 lines, split it. Examples from the codebase:
- `hl_trading_service.cpp` (orders) + `hl_trading_cancel.cpp` (cancellations)
- `hl_meta.cpp` (perps) + `hl_meta_spot.cpp` (spot)
- `hl_broker.cpp` (lifecycle) + `hl_broker_market.cpp` (data) + `hl_broker_trade.cpp` (orders) + `hl_broker_commands.cpp` (commands)

---

## Dependency Checklist

Before adding an `#include`:

1. What layer is the including file in?
2. What layer is the included file in?
3. Is the dependency direction correct? (Foundation <- Transport <- Services <- API)
4. Am I introducing a circular dependency?

**Forbidden patterns:**
- Transport file including a service header
- Foundation file including a transport header
- Any file including `hl_broker_internal.h` (API-only header)
