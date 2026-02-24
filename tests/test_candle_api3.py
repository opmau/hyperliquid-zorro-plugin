"""Test 3: Deep analysis of candle gaps - specifically MEME and DOGE edge cases."""
import json
import urllib.request
import time
from datetime import datetime, timezone

def fetch_candles(coin, interval, start_ms, end_ms):
    url = "https://api.hyperliquid.xyz/info"
    payload = json.dumps({
        "type": "candleSnapshot",
        "req": {
            "coin": coin,
            "interval": interval,
            "startTime": start_ms,
            "endTime": end_ms
        }
    }).encode("utf-8")

    req = urllib.request.Request(url, data=payload,
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=10) as resp:
        return json.loads(resp.read())

def ms_to_utc(ms):
    return datetime.fromtimestamp(ms / 1000, tz=timezone.utc).strftime("%H:%M:%S")

def main():
    now_ms = int(time.time() * 1000)

    # Test MEME more carefully - it had missing candles
    print("=" * 70)
    print("DEEP ANALYSIS: MEME 1m candles (2h window)")
    print("=" * 70)
    start_ms = now_ms - (120 * 60 * 1000)
    candles = fetch_candles("MEME", "1m", start_ms, now_ms)

    expected_count = 120  # 120 minutes
    print(f"Start: {ms_to_utc(start_ms)} UTC")
    print(f"End:   {ms_to_utc(now_ms)} UTC")
    print(f"Expected ~{expected_count} candles, got {len(candles)}")
    print(f"Missing: {expected_count - len(candles)} candles")

    if candles:
        # Check every consecutive pair for gaps
        print("\nDetailed gap analysis:")
        total_gaps = 0
        for i in range(1, len(candles)):
            gap_ms = candles[i]['t'] - candles[i-1]['t']
            gap_min = gap_ms / 60000
            if gap_min > 1.1:  # Allow small tolerance
                total_gaps += 1
                missing_count = int(gap_min) - 1
                print(f"  GAP at {ms_to_utc(candles[i-1]['t'])} -> {ms_to_utc(candles[i]['t'])}: "
                      f"{gap_min:.1f} min ({missing_count} candles missing)")
                print(f"    Before gap: n={candles[i-1]['n']} v={candles[i-1]['v']}")
                print(f"    After gap:  n={candles[i]['n']} v={candles[i]['v']}")

        if total_gaps == 0:
            print("  No gaps detected (all candles consecutive)")

        # Count zero-trade candles
        zero_n = [i for i, c in enumerate(candles) if c['n'] == 0]
        print(f"\nZero-trade candles (n=0): {len(zero_n)} out of {len(candles)}")
        if zero_n:
            print("  Indices:", zero_n[:20])  # Show first 20
            # Show a zero-trade candle
            zc = candles[zero_n[0]]
            print(f"  Sample zero-trade candle: o={zc['o']} h={zc['h']} l={zc['l']} c={zc['c']} v={zc['v']}")
            print(f"  (Note: OHLC are all equal = previous close carried forward)")

    # Test DOGE with longer window to find natural gaps
    print("\n" + "=" * 70)
    print("DEEP ANALYSIS: DOGE 1m candles (12h window)")
    print("=" * 70)
    start_ms = now_ms - (12 * 60 * 60 * 1000)
    candles = fetch_candles("DOGE", "1m", start_ms, now_ms)
    expected_count = 720
    print(f"Expected ~{expected_count} candles, got {len(candles)}")
    print(f"Missing: {expected_count - len(candles)} candles")

    if candles:
        gaps = []
        for i in range(1, len(candles)):
            gap_ms = candles[i]['t'] - candles[i-1]['t']
            if gap_ms > 60001:
                gap_min = gap_ms / 60000
                gaps.append((i, gap_min, candles[i-1]['t'], candles[i]['t']))

        print(f"Gaps found: {len(gaps)}")
        for idx, gm, t1, t2 in gaps[:10]:
            print(f"  {ms_to_utc(t1)} -> {ms_to_utc(t2)}: {gm:.1f} min gap")

        zero_n = sum(1 for c in candles if c['n'] == 0)
        print(f"Zero-trade candles: {zero_n} out of {len(candles)}")

    # CRITICAL TEST: What happens with endTime in the past?
    # Simulate what BrokerHistory2 does
    print("\n" + "=" * 70)
    print("BROKER SIMULATION: BrokerHistory2 36-min request for DOGE")
    print("=" * 70)
    # Simulate nTicks=36, tickMinutes=1
    nTicks = 36
    tickMinutes = 1
    end_ms = now_ms
    interval_ms = tickMinutes * 60 * 1000
    start_ms_calc = end_ms - (nTicks * interval_ms)
    print(f"startMs = endMs - (nTicks * intervalMs) = {end_ms} - ({nTicks} * {interval_ms}) = {start_ms_calc}")
    print(f"Time range: {ms_to_utc(start_ms_calc)} to {ms_to_utc(end_ms)} UTC")

    candles = fetch_candles("DOGE", "1m", start_ms_calc, end_ms)
    print(f"Candles returned: {len(candles)}")

    if candles:
        actual_start = candles[0]['t']
        actual_end = candles[-1]['t']
        print(f"Actual range: {ms_to_utc(actual_start)} to {ms_to_utc(actual_end)} UTC")
        print(f"Time covered: {(actual_end - actual_start) / 60000:.1f} minutes")

        gaps = []
        for i in range(1, len(candles)):
            gap_ms = candles[i]['t'] - candles[i-1]['t']
            if gap_ms > 60001:
                gaps.append((candles[i-1]['t'], candles[i]['t'], gap_ms / 60000))
        if gaps:
            print(f"GAPS FOUND: {len(gaps)}")
            for t1, t2, gm in gaps:
                print(f"  {ms_to_utc(t1)} -> {ms_to_utc(t2)}: {gm:.1f} min")
        else:
            print("No gaps - all candles are consecutive")

    # Test: Does HL include the endTime candle?
    print("\n" + "=" * 70)
    print("BOUNDARY: endTime alignment test")
    print("=" * 70)
    # Set endTime to exact candle boundary
    aligned_end = (now_ms // 60000) * 60000  # Round down to minute
    aligned_start = aligned_end - (10 * 60000)  # 10 minutes before
    candles_aligned = fetch_candles("DOGE", "1m", aligned_start, aligned_end)
    print(f"startTime={aligned_start} ({ms_to_utc(aligned_start)})")
    print(f"endTime  ={aligned_end} ({ms_to_utc(aligned_end)})")
    print(f"Candles returned: {len(candles_aligned)}")
    if candles_aligned:
        print(f"First t={candles_aligned[0]['t']} ({ms_to_utc(candles_aligned[0]['t'])})")
        print(f"Last  t={candles_aligned[-1]['t']} ({ms_to_utc(candles_aligned[-1]['t'])})")
        first_matches = candles_aligned[0]['t'] == aligned_start
        print(f"First candle starts at startTime: {first_matches}")
        # Does last candle match endTime?
        last_matches = candles_aligned[-1]['t'] == aligned_end
        print(f"Last candle starts at endTime: {last_matches}")
        if not last_matches:
            last_candle_end = candles_aligned[-1]['T']
            print(f"Last candle T={last_candle_end} ({ms_to_utc(last_candle_end)})")
            print(f"Note: endTime includes candles whose open time <= endTime")

if __name__ == "__main__":
    main()
