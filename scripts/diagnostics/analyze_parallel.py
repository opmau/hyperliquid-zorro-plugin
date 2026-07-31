#!/usr/bin/env python3
"""
analyze_parallel.py — cross-reference HL WS cadence CSV with plugin stale-age log.

Inputs:
  --csv           parallel_run.csv from hl_ws_cadence.py
  --log           Zorro plugin log for the strategy under investigation
  --window-start  parallel-window start in seconds since script start
                  (i.e. when Zorro began running, relative to Python script start)
  --window-end    parallel-window end in seconds since script start

Outputs to stdout:
  - Per-coin l2Book gap percentiles in the parallel window (script-side ground truth)
  - Plugin-side cache-age distribution from stale(age=) lines
  - Side-by-side comparison: do plugin cache ages match script-observed gaps?
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path


def percentile(sorted_values, p):
    if not sorted_values:
        return 0
    idx = max(0, min(len(sorted_values) - 1, int(len(sorted_values) * p)))
    return sorted_values[idx]


def fmt_dist(values, label):
    if not values:
        print(f"  {label}: NO SAMPLES")
        return
    s = sorted(values)
    n = len(s)
    print(f"  {label} (n={n}): "
          f"min={s[0]} p50={percentile(s,0.50)} p90={percentile(s,0.90)} "
          f"p95={percentile(s,0.95)} p99={percentile(s,0.99)} max={s[-1]} "
          f"mean={sum(s)/n:.0f}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", type=Path, required=True)
    ap.add_argument("--log", type=Path, required=True)
    ap.add_argument("--window-start", type=int, required=True,
                    help="seconds since python script start when Zorro began")
    ap.add_argument("--window-end", type=int, required=True,
                    help="seconds since python script start when Zorro ended")
    args = ap.parse_args()

    win_start_ms = args.window_start * 1000
    win_end_ms = args.window_end * 1000

    # --- Load CSV, find script start time, filter to parallel window ---
    print(f"Loading CSV: {args.csv}")
    with args.csv.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)

    if not rows:
        print("ERROR: empty CSV")
        return 1

    # recv_ms is monotonic from script start (first row's recv_ms IS roughly t=0)
    first_recv_ms = int(rows[0]["recv_ms"])
    last_recv_ms = int(rows[-1]["recv_ms"])
    print(f"  Total rows: {len(rows)}")
    print(f"  Script run: 0 -> {(last_recv_ms - first_recv_ms)/1000:.0f}s")
    print(f"  Parallel window: {args.window_start}s -> {args.window_end}s")

    # Per-coin l2Book and trades arrival times within parallel window
    l2_arrivals = defaultdict(list)  # coin -> list of recv_ms
    tr_arrivals = defaultdict(list)
    rows_in_window = 0

    for row in rows:
        rel_ms = int(row["recv_ms"]) - first_recv_ms
        if rel_ms < win_start_ms or rel_ms > win_end_ms:
            continue
        rows_in_window += 1
        ch = row["channel"]
        coin = row["coin"]
        if ch == "l2Book":
            l2_arrivals[coin].append(rel_ms)
        elif ch == "trades":
            tr_arrivals[coin].append(rel_ms)

    print(f"  Rows in parallel window: {rows_in_window}")
    print()

    # --- Compute per-coin l2Book gap distribution ---
    print("=" * 90)
    print("SCRIPT-SIDE: per-coin l2Book inter-arrival gaps in parallel window (ms)")
    print("=" * 90)
    coins = sorted(l2_arrivals.keys())
    print(f"  {'coin':<6} {'count':>6} {'min':>6} {'p50':>6} {'p90':>6} "
          f"{'p95':>6} {'p99':>6} {'max':>6} {'mean':>7} {'rate/s':>7}")
    all_gaps = []
    for coin in coins:
        arr = l2_arrivals[coin]
        if len(arr) < 2:
            print(f"  {coin:<6} {len(arr):>6} (insufficient samples)")
            continue
        gaps = [arr[i+1] - arr[i] for i in range(len(arr)-1)]
        all_gaps.extend(gaps)
        s = sorted(gaps)
        n = len(s)
        win_s = (args.window_end - args.window_start)
        rate = (len(arr)) / win_s if win_s > 0 else 0
        print(f"  {coin:<6} {n:>6} {s[0]:>6} "
              f"{percentile(s,0.50):>6} {percentile(s,0.90):>6} "
              f"{percentile(s,0.95):>6} {percentile(s,0.99):>6} "
              f"{s[-1]:>6} {sum(s)/n:>7.0f} {rate:>7.2f}")

    print()
    print("Aggregate across all 10 coins:")
    fmt_dist(all_gaps, "  combined l2Book gaps")

    # --- Connection-wide silence: gap between any two consecutive messages ---
    print()
    print("CONNECTION-WIDE: gap between any-channel messages in parallel window")
    all_arrivals = []
    for row in rows:
        rel_ms = int(row["recv_ms"]) - first_recv_ms
        if win_start_ms <= rel_ms <= win_end_ms:
            all_arrivals.append(rel_ms)
    all_arrivals.sort()
    global_gaps = [all_arrivals[i+1] - all_arrivals[i]
                   for i in range(len(all_arrivals)-1)]
    fmt_dist(global_gaps, "  global gaps")

    # Top 10 longest silences in parallel window
    if global_gaps:
        print()
        print("  Top 10 longest connection-wide silences:")
        for g in sorted(global_gaps, reverse=True)[:10]:
            print(f"    {g}ms")

    # --- Trade vs l2Book correlation ---
    print()
    print("=" * 90)
    print("TRADE-DRIVEN vs BOOK-DRIVEN check (per coin in parallel window)")
    print("=" * 90)
    print(f"  {'coin':<6} {'l2_count':>10} {'tr_count':>10} {'tr/l2':>8}")
    for coin in coins:
        l2_n = len(l2_arrivals.get(coin, []))
        tr_n = len(tr_arrivals.get(coin, []))
        ratio = (tr_n / l2_n) if l2_n > 0 else 0.0
        print(f"  {coin:<6} {l2_n:>10} {tr_n:>10} {ratio:>8.2f}")
    print()
    print("  Interpretation: tr/l2 < 1 means l2Book pushes outpace trade events,")
    print("  i.e. book modifications (cancels/adds) DO trigger pushes without trades.")
    print("  tr/l2 ~ 1+ means each l2Book push is closely associated with a trade.")

    # --- Plugin log: stale(age=) distribution ---
    print()
    print("=" * 90)
    print("PLUGIN-SIDE: stale(age=Xms) distribution from plugin log")
    print("=" * 90)
    log_text = args.log.read_text(encoding="utf-8", errors="replace")
    stale_re = re.compile(r"([A-Z]+) stale \(age=(\d+)ms, max=1500\)")
    plugin_ages_per_coin = defaultdict(list)
    for m in stale_re.finditer(log_text):
        plugin_ages_per_coin[m.group(1)].append(int(m.group(2)))
    print(f"  {'coin':<6} {'count':>6} {'min':>6} {'p50':>6} {'p90':>6} "
          f"{'p95':>6} {'p99':>6} {'max':>6}")
    all_plugin_ages = []
    for coin in sorted(plugin_ages_per_coin.keys()):
        ages = plugin_ages_per_coin[coin]
        all_plugin_ages.extend(ages)
        s = sorted(ages)
        if not s:
            continue
        print(f"  {coin:<6} {len(s):>6} {s[0]:>6} "
              f"{percentile(s,0.50):>6} {percentile(s,0.90):>6} "
              f"{percentile(s,0.95):>6} {percentile(s,0.99):>6} {s[-1]:>6}")
    print()
    fmt_dist(all_plugin_ages, "  combined plugin cache ages")

    # --- Side-by-side ---
    print()
    print("=" * 90)
    print("CROSS-REFERENCE: script-observed l2Book gaps vs plugin cache age")
    print("=" * 90)
    print()
    s_gaps_sorted = sorted(all_gaps)
    p_ages_sorted = sorted(all_plugin_ages)
    print(f"  {'percentile':<10} {'script gap':>12} {'plugin age':>12} {'delta':>10}")
    for p in (0.50, 0.90, 0.95, 0.99, 1.00):
        sg = percentile(s_gaps_sorted, p) if p < 1.0 else (s_gaps_sorted[-1] if s_gaps_sorted else 0)
        pa = percentile(p_ages_sorted, p) if p < 1.0 else (p_ages_sorted[-1] if p_ages_sorted else 0)
        delta = pa - sg
        label = "p50" if p == 0.50 else "p90" if p == 0.90 else "p95" if p == 0.95 else "p99" if p == 0.99 else "max"
        print(f"  {label:<10} {sg:>12} {pa:>12} {delta:>+10}")
    print()
    print("Decision rule:")
    print("  - If plugin cache ages >> script-observed gaps  -> plugin-internal latency")
    print("  - If plugin cache ages ~= script-observed gaps  -> HL/network cadence is the cause")
    print("  - Note: plugin only logs ages above 1500ms threshold (biased upward sample);")
    print("    script gaps are unbiased. Compare TAILS (p95+, max) for fairest read.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
