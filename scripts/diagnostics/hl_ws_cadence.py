#!/usr/bin/env python3
"""
hl_ws_cadence.py — measure Hyperliquid WebSocket l2Book and trades cadence.

Subscribes to l2Book + trades for a configurable set of mainnet perp coins
on a single WS connection, records every incoming message with a high-resolution
local arrival timestamp, writes a CSV, and on shutdown prints a per-coin
gap summary plus the longest connection-wide silence.

Goal: ground-truth what HL actually delivers, independent of the Zorro plugin.
If this script sees the same 2-6s per-coin gaps that the plugin's cache age
shows, the cause is upstream of our plugin (HL push behavior or network).
If gaps here are sub-second but the plugin sees seconds, the cause is
plugin-side (queueing, parser, connection thread).

Usage:
    python hl_ws_cadence.py                       # default 10 coins, 30 min
    python hl_ws_cadence.py --duration 600        # 10 min
    python hl_ws_cadence.py --coins BTC ETH SOL   # custom set
    python hl_ws_cadence.py --testnet             # testnet endpoint
    python hl_ws_cadence.py --csv out.csv         # custom CSV path

Requires: websockets (pip install websockets)
"""

from __future__ import annotations

import argparse
import asyncio
import csv
import json
import signal
import sys
import time
from collections import defaultdict
from pathlib import Path

try:
    import websockets
except ImportError:
    sys.stderr.write(
        "ERROR: 'websockets' package not installed.\n"
        "Run: pip install websockets\n"
    )
    sys.exit(2)


DEFAULT_COINS = ["BTC", "ETH", "SOL", "AVAX", "LINK", "BNB", "ADA", "DOGE", "TRX", "XRP"]
MAINNET_URL = "wss://api.hyperliquid.xyz/ws"
TESTNET_URL = "wss://api.hyperliquid-testnet.xyz/ws"

# Application-level ping interval (matches plugin's 30s).
APP_PING_INTERVAL_S = 30.0

# Stop running after this many seconds even if Ctrl-C wasn't pressed.
DEFAULT_DURATION_S = 1800  # 30 min


def now_ms() -> int:
    """Monotonic millisecond timestamp. perf_counter avoids wall-clock drift."""
    return int(time.perf_counter() * 1000)


async def subscriber(
    url: str,
    coins: list[str],
    csv_path: Path,
    duration_s: int,
    stop_event: asyncio.Event,
) -> dict:
    """Connect, subscribe to l2Book + trades for each coin, record every message.

    Returns a stats dict for end-of-run summary.
    """
    # Per-coin per-channel last-arrival timestamps; populated as messages flow.
    last_arrival: dict[tuple[str, str], int] = {}
    # Per-coin per-channel gap lists (ms).
    gaps: dict[tuple[str, str], list[int]] = defaultdict(list)
    # Connection-wide last arrival (any channel).
    last_any_arrival: list[int] = [0]
    global_gaps: list[int] = []
    msg_count = 0

    started_ms = now_ms()
    csv_file = csv_path.open("w", newline="", encoding="utf-8")
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(["recv_ms", "channel", "coin", "server_time_ms", "raw_size"])

    print(f"[connect] {url}")
    async with websockets.connect(url, ping_interval=20, ping_timeout=10) as ws:
        # Subscribe to l2Book and trades for every coin.
        for coin in coins:
            for sub_type in ("l2Book", "trades"):
                msg = json.dumps(
                    {"method": "subscribe", "subscription": {"type": sub_type, "coin": coin}}
                )
                await ws.send(msg)
        print(f"[subscribe] {len(coins)} coins x 2 channels = {len(coins) * 2} subs")

        last_app_ping = time.monotonic()

        while not stop_event.is_set():
            elapsed_s = (now_ms() - started_ms) / 1000.0
            if elapsed_s >= duration_s:
                print(f"[stop] reached duration={duration_s}s")
                break

            # Application-level ping at 30s intervals (matches plugin behavior).
            if time.monotonic() - last_app_ping >= APP_PING_INTERVAL_S:
                try:
                    await ws.send(json.dumps({"method": "ping"}))
                    last_app_ping = time.monotonic()
                except Exception as e:
                    print(f"[warn] app ping send failed: {e}")

            # Wait up to 1 second for a message; loop checks stop_event regularly.
            try:
                raw = await asyncio.wait_for(ws.recv(), timeout=1.0)
            except asyncio.TimeoutError:
                continue
            except websockets.exceptions.ConnectionClosed as e:
                print(f"[disconnect] {e}")
                break

            recv_ms = now_ms()
            raw_size = len(raw) if isinstance(raw, (str, bytes)) else 0

            # Record connection-wide gap (any channel).
            if last_any_arrival[0] != 0:
                global_gaps.append(recv_ms - last_any_arrival[0])
            last_any_arrival[0] = recv_ms

            # Parse just enough to route by channel + coin.
            try:
                msg = json.loads(raw)
            except json.JSONDecodeError:
                continue

            channel = msg.get("channel", "")
            data = msg.get("data", {})

            if channel == "subscriptionResponse":
                continue
            if channel == "pong":
                continue

            # Extract coin + server timestamp depending on channel.
            coin = ""
            server_time_ms = 0
            if channel == "l2Book":
                # data: {"coin": "BTC", "time": 1777...ms, "levels": [...]}
                coin = data.get("coin", "")
                server_time_ms = int(data.get("time", 0))
            elif channel == "trades":
                # data: list of trades, each {"coin": "BTC", "time": ..., ...}
                # Take the first trade's coin/time as representative.
                if isinstance(data, list) and data:
                    coin = data[0].get("coin", "")
                    server_time_ms = int(data[0].get("time", 0))
            else:
                # Other channels — record but don't track per-coin gaps.
                continue

            if not coin:
                continue

            key = (channel, coin)
            if key in last_arrival:
                gaps[key].append(recv_ms - last_arrival[key])
            last_arrival[key] = recv_ms

            csv_writer.writerow([recv_ms, channel, coin, server_time_ms, raw_size])
            msg_count += 1

            # Periodic progress update (every ~10s of wall clock).
            if msg_count % 1000 == 0:
                csv_file.flush()
                print(
                    f"[progress] elapsed={elapsed_s:6.1f}s msgs={msg_count} "
                    f"max_global_gap={max(global_gaps) if global_gaps else 0}ms"
                )

    csv_file.close()
    return {
        "started_ms": started_ms,
        "ended_ms": now_ms(),
        "msg_count": msg_count,
        "gaps": gaps,
        "global_gaps": global_gaps,
        "last_arrival": last_arrival,
    }


def percentile(sorted_values: list[int], p: float) -> int:
    if not sorted_values:
        return 0
    idx = max(0, min(len(sorted_values) - 1, int(len(sorted_values) * p)))
    return sorted_values[idx]


def print_summary(coins: list[str], stats: dict) -> None:
    duration_s = (stats["ended_ms"] - stats["started_ms"]) / 1000.0
    print()
    print("=" * 88)
    print(f"  HL WS cadence summary — duration {duration_s:.1f}s, total msgs {stats['msg_count']}")
    print("=" * 88)

    print()
    print("Per-coin l2Book gaps (ms):")
    print(f"  {'coin':<6} {'count':>6} {'median':>8} {'p90':>8} {'p95':>8} {'p99':>8} {'max':>8} {'rate/s':>8}")
    for coin in coins:
        key = ("l2Book", coin)
        sorted_gaps = sorted(stats["gaps"].get(key, []))
        n = len(sorted_gaps)
        rate = (n + 1) / duration_s if duration_s > 0 else 0
        if not sorted_gaps:
            print(f"  {coin:<6} {0:>6} {'-':>8} {'-':>8} {'-':>8} {'-':>8} {'-':>8} {0.0:>8.2f}")
            continue
        print(
            f"  {coin:<6} {n:>6} "
            f"{percentile(sorted_gaps, 0.50):>8} "
            f"{percentile(sorted_gaps, 0.90):>8} "
            f"{percentile(sorted_gaps, 0.95):>8} "
            f"{percentile(sorted_gaps, 0.99):>8} "
            f"{sorted_gaps[-1]:>8} "
            f"{rate:>8.2f}"
        )

    print()
    print("Per-coin trades event count (correlation check):")
    print(f"  {'coin':<6} {'l2_count':>10} {'tr_count':>10} {'tr/l2':>8}")
    for coin in coins:
        l2_n = len(stats["gaps"].get(("l2Book", coin), []))
        tr_n = len(stats["gaps"].get(("trades", coin), []))
        ratio = (tr_n / l2_n) if l2_n > 0 else 0.0
        print(f"  {coin:<6} {l2_n:>10} {tr_n:>10} {ratio:>8.2f}")

    print()
    print("Connection-wide silence (any-channel):")
    sorted_global = sorted(stats["global_gaps"])
    if sorted_global:
        print(f"  total inter-arrival samples: {len(sorted_global)}")
        print(f"  median: {percentile(sorted_global, 0.50)}ms")
        print(f"  p95:    {percentile(sorted_global, 0.95)}ms")
        print(f"  p99:    {percentile(sorted_global, 0.99)}ms")
        print(f"  max:    {sorted_global[-1]}ms")
        print()
        print("  Top 10 longest silences (ms):")
        for g in sorted(stats["global_gaps"], reverse=True)[:10]:
            print(f"    {g}")
    else:
        print("  no samples")

    print()
    print("Interpretation guide:")
    print("  - If per-coin l2Book p95 >> 1500ms  -> HL cadence is the cause")
    print("  - If global p95 << per-coin p95     -> HL is event-driven per coin")
    print("  - If global p95 also >> 1500ms      -> shared connection/network stalls")
    print("  - If trades count ~0 during l2Book gaps -> trade-driven push")
    print()


def main() -> int:
    parser = argparse.ArgumentParser(description="HL WS cadence measurement")
    parser.add_argument("--coins", nargs="+", default=DEFAULT_COINS,
                        help=f"coins to subscribe (default: {DEFAULT_COINS})")
    parser.add_argument("--duration", type=int, default=DEFAULT_DURATION_S,
                        help=f"max run seconds (default: {DEFAULT_DURATION_S})")
    parser.add_argument("--testnet", action="store_true",
                        help="use testnet endpoint instead of mainnet")
    parser.add_argument("--csv", type=Path, default=None,
                        help="CSV output path (default: hl_ws_cadence_<epoch>.csv)")
    args = parser.parse_args()

    url = TESTNET_URL if args.testnet else MAINNET_URL
    csv_path = args.csv or Path(f"hl_ws_cadence_{int(time.time())}.csv")

    print(f"[config] coins={args.coins} duration={args.duration}s csv={csv_path}")

    stop_event = asyncio.Event()

    def _on_signal(*_args):
        print("\n[signal] received, stopping...")
        stop_event.set()

    # Windows signal handling: SIGINT works via Ctrl-C in console.
    try:
        signal.signal(signal.SIGINT, _on_signal)
    except (ValueError, OSError):
        pass

    try:
        stats = asyncio.run(
            subscriber(url, args.coins, csv_path, args.duration, stop_event)
        )
    except KeyboardInterrupt:
        print("\n[interrupt]")
        return 130

    print_summary(args.coins, stats)
    print(f"[done] CSV written to {csv_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
