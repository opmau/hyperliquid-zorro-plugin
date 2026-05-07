# Diagnostics Scripts

Out-of-band measurement tools for diagnosing plugin behavior against ground-truth
exchange data. Not part of the build, not part of tests — operator/developer aids
that are kept under version control because they are reusable.

## hl_ws_cadence.py

Subscribes directly to Hyperliquid's public WebSocket (`l2Book` + `trades`) for
a configurable set of coins, on a single connection, and records every incoming
message with a high-resolution local arrival timestamp.

Used to ground-truth plugin claims about WS push cadence — e.g. when the plugin's
`getPrice` cache age routinely exceeds 1500 ms, this script tells us
whether the gap is on HL's side / network side, or inside the plugin's WS path
(IXWebSocket queue, parser, connection thread).

### Setup (one-time)

```powershell
pip install websockets
```

No Hyperliquid auth needed — `l2Book` and `trades` are public channels.

### Run

```powershell
# 30-min run, default 10 mainnet coins (matches YOLO_HL_Native asset set)
python scripts/diagnostics/hl_ws_cadence.py

# Shorter run for a quick check
python scripts/diagnostics/hl_ws_cadence.py --duration 600

# Custom coin set
python scripts/diagnostics/hl_ws_cadence.py --coins BTC ETH SOL

# Testnet (cadence may differ — use mainnet to match real sessions)
python scripts/diagnostics/hl_ws_cadence.py --testnet
```

Press Ctrl-C any time to stop early — the summary prints on shutdown.

### Output

1. CSV file `hl_ws_cadence_<epoch>.csv` with one row per WS message:
   `recv_ms, channel, coin, server_time_ms, raw_size`
2. End-of-run summary printed to stdout:
   - Per-coin l2Book gap distribution (count, median, p90, p95, p99, max, rate/sec)
   - Per-coin trades event count (for trade-driven vs book-change-driven trigger check)
   - Connection-wide silence percentiles + top 10 longest silences

### Interpretation guide

- **per-coin l2Book p95 >> 1500 ms** → HL/network cadence is the root cause; plugin tuning required
- **global p95 << per-coin p95** → HL is event-driven per coin (independent quiet windows); a "shared connection stall" is NOT happening
- **global p95 also >> 1500 ms** → genuine shared-connection / network-level stalls
- **trades count near zero during l2Book gaps** → push is trade-driven (trades cause book changes that trigger pushes)
- **trades arriving without l2Book pushes** → push is **not** purely trade-driven; book modifications (cancels/adds) also trigger

### Recommended methodology

For WS-cadence root-cause confirmation:

1. **Baseline run alone** (~30 min, mainnet, during the same time of day as the
   real YOLO session). Establishes the ground truth for what HL delivers from
   this network/machine.
2. **Parallel run with the plugin** (~30 min, both running simultaneously).
   Compare the script's CSV per-coin gaps against the plugin's
   `Log/YOLO_HL_Native.log` cache-age values from the same wall-clock window.
3. **Decision tree:**
   - If both see 2-6 s per-coin gaps → confirmed HL/network behavior; the
     plugin is reporting reality. Fix at the call site (raise threshold or
     skip `getPrice` for the IMPORTED P&L path).
   - If script sees sub-second gaps but plugin sees 2-6 s → plugin-internal
     issue (queue draining, parser, connection thread). Different fix path.

### Caveats

- `time.perf_counter()`-based ms timestamps are local-monotonic, not wall-clock.
  They are accurate for inter-arrival measurement but not for cross-machine sync.
- The script holds its own WS connection — running it in parallel with the
  plugin means HL sees two simultaneous connections from the same IP. They
  shouldn't interfere, but worth noting if you want a clean A/B.
- Results vary by time of day — run during your usual session window for
  representative numbers.
