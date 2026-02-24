"""Test Hyperliquid candleSnapshot API behavior for OPM-104 investigation."""
import json
import urllib.request
import time

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

def main():
    now_ms = int(time.time() * 1000)

    # Test 1: DOGE 1m candles, last 36 minutes
    print("=" * 70)
    print("TEST 1: DOGE 1m candles, last 36 minutes")
    print("=" * 70)
    start_ms = now_ms - (36 * 60 * 1000)
    candles = fetch_candles("DOGE", "1m", start_ms, now_ms)
    print(f"Requested: 36 minutes of 1m candles")
    print(f"Expected:  ~36 candles")
    print(f"Received:  {len(candles)} candles")

    if candles:
        print(f"\nFirst candle: t={candles[0]['t']} T={candles[0]['T']} n={candles[0]['n']} v={candles[0]['v']}")
        print(f"Last  candle: t={candles[-1]['t']} T={candles[-1]['T']} n={candles[-1]['n']} v={candles[-1]['v']}")

        # Check for gaps
        max_gap_ms = 0
        gap_count = 0
        for i in range(1, len(candles)):
            gap = candles[i]['t'] - candles[i-1]['t']
            expected_gap = 60000  # 1 minute in ms
            if gap > expected_gap:
                gap_minutes = gap / 60000
                gap_count += 1
                if gap > max_gap_ms:
                    max_gap_ms = gap
                if gap_minutes > 2:
                    print(f"  GAP at index {i}: {gap_minutes:.1f} min (t={candles[i-1]['t']} -> {candles[i]['t']})")

        print(f"\nGaps > 1min: {gap_count}")
        print(f"Largest gap: {max_gap_ms / 60000:.1f} minutes")

        # Check n=0 (zero-trade) candles
        zero_trade = [c for c in candles if c['n'] == 0]
        print(f"Candles with n=0 (no trades): {len(zero_trade)}")

    # Test 2: A less liquid asset (try a smaller perp)
    print("\n" + "=" * 70)
    print("TEST 2: BTC 1m candles, last 36 minutes (high liquidity baseline)")
    print("=" * 70)
    candles2 = fetch_candles("BTC", "1m", start_ms, now_ms)
    print(f"Requested: 36 minutes of 1m candles")
    print(f"Expected:  ~36 candles")
    print(f"Received:  {len(candles2)} candles")

    if candles2:
        max_gap2 = 0
        for i in range(1, len(candles2)):
            gap = candles2[i]['t'] - candles2[i-1]['t']
            if gap > max_gap2:
                max_gap2 = gap
        print(f"Largest gap: {max_gap2 / 60000:.1f} minutes")

    # Test 3: Boundary test - startTime inclusive?
    print("\n" + "=" * 70)
    print("TEST 3: Exact boundary test - are startTime/endTime inclusive?")
    print("=" * 70)
    if candles and len(candles) >= 2:
        # Use the exact timestamp of the 2nd candle as startTime
        exact_start = candles[1]['t']
        exact_end = candles[-1]['T']
        candles3 = fetch_candles("DOGE", "1m", exact_start, exact_end)
        print(f"Used startTime={exact_start} (2nd candle open time)")
        print(f"Used endTime={exact_end} (last candle close time)")
        print(f"Original had {len(candles)} candles from index 1 to end = {len(candles)-1}")
        print(f"Boundary query returned: {len(candles3)} candles")
        if candles3:
            print(f"First returned t={candles3[0]['t']} (matches startTime: {candles3[0]['t'] == exact_start})")

    # Test 4: 5000 candle limit test
    print("\n" + "=" * 70)
    print("TEST 4: Request >5000 1m candles (testing 5000 cap)")
    print("=" * 70)
    big_start = now_ms - (6000 * 60 * 1000)  # 6000 minutes ago
    candles4 = fetch_candles("DOGE", "1m", big_start, now_ms)
    print(f"Requested: ~6000 minutes of 1m candles")
    print(f"Received:  {len(candles4)} candles")
    print(f"Capped at 5000: {len(candles4) <= 5100}")  # Allow small tolerance
    if candles4:
        actual_range_min = (candles4[-1]['t'] - candles4[0]['t']) / 60000
        print(f"Actual time range covered: {actual_range_min:.0f} minutes")
        print(f"First candle t={candles4[0]['t']}")
        print(f"Last  candle t={candles4[-1]['t']}")

if __name__ == "__main__":
    main()
