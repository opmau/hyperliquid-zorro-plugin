# brokerCommand Reference

Every `brokerCommand` mode this plugin implements, as callable from a Zorro
Lite-C strategy. Modes not listed here return `0`.

ALO (post-only), atomic modify, TWAP and bracket orders shipped before v2.0.0
without ever being documented; v2.1.0 brings them into this file and makes ALO
and modify actually reachable from a script.

---

## Behaviour a strategy must know before designing against this plugin

### The plugin ignores `SET_WAIT`

`BrokerBuy2` **never blocks and never polls**. It returns as soon as the
exchange responds — including on a `resting` response, where it returns the
trade ID with `*pFill = 0`. There is no `SET_WAIT` handler anywhere in the
plugin.

The Zorro manual's blocking convention therefore does not apply. A strategy
that places a resting (ALO/GTC) order must poll for the fill itself:

```c
int id = enterLong();                     // returns immediately, fill may be 0
while (elapsed < timeout) {
    var pos = brokerCommand(GET_POSITION, "BTC");   // cheap WS-cache read
    if (pos != 0) break;
    wait(200);
}
```

`GET_POSITION(symbol)` is a cheap cached read and is safe to poll in a loop.
`GET_POSITION(0)` is a destructive full rebuild — never call it mid-flight.

### Order type is sticky only for ALO

Zorro auto-calls `SET_ORDERTYPE` at every order entry and can only derive types
0–3 from `TradeMode`, so before v2.1.0 it overwrote any script-selected ALO
back to `Ioc` immediately before the order was built. `50012("Alo")` now sets a
**sticky** override that the auto-call leaves alone.

Release it explicitly with `50012("Ioc")` or `50012("Gtc")`. It is also cleared
at login and logout.

Market orders (no `OrderLimit`) always go out as `Ioc` regardless, because the
plugin prices them to cross deliberately (ask × 1.05); sending those post-only
would be a guaranteed reject.

---

## Standard Zorro modes

| Mode | Parameter | Returns | Notes |
|------|-----------|---------|-------|
| `GET_COMPLIANCE` (51) | — | `0` | Defers to the `NFA` column in `Accounts.csv` |
| `GET_MAXREQUESTS` (45) | — | `5` | Max concurrent HTTP requests |
| `GET_MAXTICKS` (43) | — | `5000` | Candles per history request |
| `GET_BROKERZONE` (40) | — | `0` | Hyperliquid timestamps are UTC |
| `GET_VOLUME` (61) | — | `1` | Signals tick activity exists |
| `GET_PRICETYPE` (150) | — | `2` | Plugin returns bid/ask, not traded prices |
| `SET_DIAGNOSTICS` (138) | level `int` | `1` | 0=off, 1=errors, 2=info, 3=verbose |
| `SET_AMOUNT` (139) | `double*` | `1` | Contracts per lot |
| `SET_SYMBOL` (132) | symbol `string` | `1` | Sets the asset context for `GET_PRICE`, `GET_POSITION`, `DO_CANCEL(0)` |
| `SET_ORDERTYPE` (157) | `0`/`2`/`4` (+`8`) | echo | `0`→Ioc, `2`→Gtc, `4`→Alo (sticky). `+8` = protective stop. `1`/`3` (AON) unsupported → `0` |
| `SET_HWND` (172) | `HWND` | `1` | |
| `GET_PRICE` (60) | `4`=mid, `5`=ask, `6`=bid | price | Per-asset; requires `SET_SYMBOL`. Hard `0.0` when no data |
| `GET_POSITION` (53) | symbol `string` | net size | Cheap cached read, safe to poll |
| `GET_POSITION` (53) | `0` | `1` | **Destructive** — rebuilds all positions from the broker |
| `GET_TRADES` (71) | `TRADE*` | count | Imports broker positions into Zorro |
| `DO_CANCEL` (301) | trade ID | `1`/`0` | Cancels that order. If a resting **close** order exists for the position, cancels that instead of the filled entry |
| `DO_CANCEL` (301) | `0` | count | Cancels every resting order for the current `SET_SYMBOL`, or account-wide if none is set |

---

## Hyperliquid-specific modes

### Configuration

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50010 | `HL_VALIDATE_PRICES` | — | `1` if BTC/ETH/SOL all have prices |
| 50011 | `HL_ENABLE_WEBSOCKET` | `1`/`0` | `1` on success |
| 50012 | `HL_SET_ORDER_TYPE` | `"Ioc"`/`"Gtc"`/`"Alo"` | `1`, or `0` if invalid |
| 50021 | `HL_SET_ACCOUNT_MODE` | `0`=API wallet, `1`=vault | `1` |

`50012` accepts any casing and stores the canonical form. This matters: the
signing path canonicalizes and the JSON path did not, so a non-canonical string
used to produce a signature mismatch that Hyperliquid reports as the very
misleading *"User or API Wallet does not exist"*.

### Queries

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50020 | `HL_GET_OPEN_ORDERS` | symbol `string`, or `0` for all | Count of resting orders |
| 50023 | `HL_GET_LAST_ORDER_ERROR` | `0`, or `char[256]` buffer | Error class (see below) |
| 50031 | `HL_GET_FUNDING_RATE` | coin `string`, or `0` for current symbol | Hourly funding rate |

`50020` normalizes the symbol the same way `SET_SYMBOL` does (so perpDex names
work) and falls back to an HTTP query when the WebSocket cache is empty.

#### `50023` error classes

| Value | Meaning | Typical strategy response |
|-------|---------|---------------------------|
| `0` | No error recorded | — |
| `1` | Post-only order would have immediately matched | Reprice one tick further back and retry |
| `2` | Insufficient margin | Stop trading this asset; alert |
| `3` | Anything else (tick size, min notional, reduce-only, signing) | Log the text and investigate |

Pass a buffer of **at least 256 bytes** to also retrieve the exchange's
verbatim message. The class is cleared on every successful order placement, so
read it immediately after a `BrokerBuy2` that returned `0`.

```c
char errText[256];
int id = enterLong();
if (!id) {
    int cls = brokerCommand(50023, errText);
    if (cls == 1) { /* would cross — back off one tick */ }
    else if (cls == 2) { /* margin — stop */ }
    else printf("\nOrder rejected: %s", errText);
}
```

### Order management

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50032 | `HL_SCHEDULE_CANCEL` | seconds from now, `0` to clear | `1` armed, `0` not armed — see below |
| 50040 | `HL_PLACE_TWAP` | `TwapRequest*` | TWAP ID |
| 50041 | `HL_CANCEL_TWAP` | TWAP ID | `1` on success |
| 50042 | `HL_MODIFY_ORDER` | `ModifyRequest*` | `1` on success |
| 50043 | `HL_PLACE_BRACKET` | `BracketRequest*` | Entry trade ID |
| 50044 | `HL_MODIFY_BY_TRADEID` | `double[3]` | See below |

**`50040`, `50042` and `50043` are not callable from Lite-C.** Their request
structs contain `std::string` members, so they are reachable only from C++.
`50044` is the Lite-C-callable equivalent of `50042`.

#### `50032` — the dead-man's switch can be refused; check the return

Hyperliquid does not offer `scheduleCancel` to every account: below a lifetime
traded-volume threshold the exchange refuses it, with a message of the form —

> Cannot set scheduled cancel time until enough volume traded.
> Required: $1000000.

On any refusal (or HTTP failure) the command returns `0`. A strategy that arms
the switch without checking the return believes its resting orders are covered
when they are not. Treat `0` as **unprotected** and decide whether to leave
orders resting at all; with `SET_DIAGNOSTICS` (138) at level 1 or above, the
refusal is also logged with the exchange's message.

#### Modify replaces with a maker order, never an IOC

Both modify paths omit the action's `a` (`always_place`) field, because
Hyperliquid rejects any action hashed with `a: false`. `always_place` is
therefore false, and that branch constrains the replacement order: it must be
`Alo`, or a **non-executable** `Gtc` that the exchange then rewrites to `Alo`.

`50044` always sends `Alo`, so it is unaffected. `50042` forwards whatever TIF
the caller put in its `ModifyRequest` — whose struct default is `Gtc` — so an
`Ioc` modify was previously signed, sent, and rejected by the exchange with no
indication of why. It is now refused locally with an explanatory error before
the round trip. A `Gtc` modify is still forwarded: whether it is executable is
a question about the current book, which only the exchange can answer.

#### `50044` — reprice a resting order

```c
ThisTrade = enterLong();  // enterLong()/enterShort() return a TRADE*, not an ID

var params[3];
params[0] = TradeID;      // the ID the broker assigned to this trade
params[1] = newPrice;     // must be > 0
params[2] = 0;            // new size in contracts, or <= 0 to keep current

int r = brokerCommand(50044, params);
```

`params[0]` is `TradeID`, the broker's own identifier for the trade. Assigning
the returned `TRADE*` to `ThisTrade` makes `TradeID` and the other trade
variables readable; passing the pointer itself yields `-1`. Check the pointer
for nonzero first — reading a trade variable through a null `ThisTrade`
crashes.

`TradeID` is readable as soon as the order is acknowledged, with `TradeLots`
still `0` until it fills, so a chase loop can reprice without waiting for a
fill. Inside a `for(open_trades)` loop `TradeID` refers to the trade being
iterated, which is the other way to reach it.

Pass a positive `TradeID`. Zorro sets it to `-1` for a trade identified by a
`TradeUUID` string rather than a number, and `50044` returns `-1` for an
untracked trade — so passing one straight through is indistinguishable from a
failed lookup.

| Return | Meaning |
|--------|---------|
| `1` | Modified. The plugin adopts whatever oid the exchange returned |
| `0` | Exchange rejected the modify, or the HTTP call failed |
| `-1` | Trade ID is not tracked |
| `-2` | Order already filled or cancelled — nothing to move |

Uses Hyperliquid's `batchModify`. Prefer it over cancel-and-replace: one round
trip instead of two, and no window in which the order could fill between the
cancel and the replacement.

It does **not** preserve queue position, and nothing in the exchange docs says
it does. A reprice moves the order to a different price level, where it is a
new arrival like any other. Absent a priority fee, which this plugin never
sends, a requote joins the **back** of its new level — see
`docs/hyperliquid-api/12a-priority-fees.md`. Budget reprices accordingly rather
than assuming they are free in queue terms.

`-2` is the expected outcome when a reprice races a fill. Treat it as "the
order is gone, re-read the position", not as an error.

If the trade has a resting **close** order against it, `50044` reprices that
close order (reduce-only), not the filled entry.

### Export

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50001 | `HL_EXPORT_ASSETS` | path `string` | `1` |
| 50002 | `HL_EXPORT_META` | path `string` | Assets written |
| 50003 | `HL_EXPORT_ACCOUNT` | path `string` | `1` |

| 50004 | `HL_SET_EXPORT_NFA` | `0`…`15` | `1`, or `0` if out of range |

`50003` writes a single-row `Accounts.csv` template from the current connection.

Its `NFA` column is Zorro's account-compliance bitfield — `1` no partial
closing, `2` no hedging, `4` FIFO, `8` no closing of trades, `14`/`15` full NFA
compliance. That is your setting, not the plugin's: you choose it in
`Accounts.csv`, or in the strategy via `set(NFA)` and `Hedge`. Zorro never
passes it to the plugin, so the plugin cannot read it back — it defaults to `0`
("no restrictions"), and `50004` sets what `50003` will write:

```c
brokerCommand(50004, 5);                    // virtual hedging, FIFO compliant
brokerCommand(50003, "Accounts_HL.csv");    // template now carries NFA=5
```

Note that `14`/`15` switch Zorro's close path from `BrokerSell2` to
`BrokerBuy2(StopDist=-1)`. Both paths are implemented.

### Debug

| Code | Name | Parameter | Returns |
|------|------|-----------|---------|
| 50030 | `HL_FORCE_WS_DISCONNECT` | — | `1` — forces a reconnect, for testing |

---

## Working a maker (ALO) order end to end

```c
// Dead-man's switch: cancel all in 60s. 0 = refused (volume-gated, see 50032
// above) — you are NOT protected; quote only what you can babysit.
if (!brokerCommand(50032, 60)) return;

brokerCommand(50012, "Alo");         // sticky — survives Zorro's auto-call

// Derive a price that cannot cross. An ALO order that would match is
// cancelled outright, so this is the single most important step.
brokerCommand(SET_SYMBOL, "BTC");
var bid = brokerCommand(GET_PRICE, 6);
var ask = brokerCommand(GET_PRICE, 5);
if (bid <= 0 || ask <= 0) return;    // hard 0.0 means no data — never guess

OrderLimit = bid;                    // buy: join the bid. Sell: use ask.
ThisTrade = enterLong();

if (!ThisTrade) {                    // never read a trade variable through null
    char errText[256];
    brokerCommand(50023, errText);   // 1 = would have crossed; requote next bar
    return;
}

int id = TradeID;                    // broker's ID for the order — 50044 takes this

// Poll for the fill — the plugin does not block.
while (waited < timeout) {
    if (brokerCommand(GET_POSITION, "BTC") != 0) break;
    wait(200);
    waited += 200;
}

// Still unfilled: reprice rather than cancel/replace. Cap the attempts.
if (reprices < maxReprices) {
    var params[3];
    params[0] = id; params[1] = brokerCommand(GET_PRICE, 6); params[2] = 0;
    if (brokerCommand(50044, params) == -2) { /* it filled — re-read position */ }
    reprices += 1;
}

// Give up: cancel everything resting on this symbol.
brokerCommand(SET_SYMBOL, "BTC");
brokerCommand(DO_CANCEL, 0);

brokerCommand(50012, "Ioc");         // release the sticky override
brokerCommand(50032, 0);             // disarm the dead-man's switch
```

### Choosing the passive price

`GET_PRICE` needs `SET_SYMBOL` first and returns a hard `0.0` when the cache
has nothing — treat that as "do not trade this bar", never as a price.

| Side | Post at             | Crossing price that gets you `BadAloPx` |
|------|---------------------|-----------------------------------------|
| Buy  | bid, `GET_PRICE(6)` | anything at or above the ask            |
| Sell | ask, `GET_PRICE(5)` | anything at or below the bid            |

Joining your own side is safe; reaching across is what triggers the reject.
The plugin rounds side-aware (buys floor, sells ceil), so rounding alone will
never push your quote across the spread — but it cannot rescue a price that was
already crossing when you passed it.

### Budgeting reprices

Every reprice costs one address-based request, and that budget is earned by
**filling**, not by waiting: Hyperliquid allows one request per 1 USDC of
cumulative traded volume since address inception, on top of an initial 10,000.
Their own worked example is that a 100 USDC order needs a 1% fill rate to break
even — so roughly 100 reprices per filled 100 USDC order before you are
spending faster than you earn. Batched actions count as `n` requests here, not
one.

Exhausting it throttles the address to **one request every 10 seconds**, which
is a bad state to reach holding an open position. Cap reprices per entry and
fall back to a taker or to flat, rather than chasing a trend indefinitely. See
`docs/hyperliquid-api/10-rate-limits.md`.

A close placed while ALO is active rests too. `BrokerSell2` reports
`*pFill = 0` until it actually fills, and Zorro's position stays open — which
is correct, because the contracts are still on the exchange.
