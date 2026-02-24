"""Test 2: Check candle gaps for less liquid assets and validate T field format."""
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

def analyze_gaps(coin, candles, interval_ms=60000):
    """Analyze candle gaps and print summary."""
    if not candles:
        print(f"  {coin}: No candles returned!")
        return

    print(f"  {coin}: {len(candles)} candles returned")

    gaps = []
    zero_trade = 0
    for i, c in enumerate(candles):
        if c['n'] == 0:
            zero_trade += 1
        if i > 0:
            gap = candles[i]['t'] - candles[i-1]['t']
            if gap > interval_ms:
                gap_min = gap / 60000
                gaps.append((i, gap_min, candles[i-1]['t'], candles[i]['t']))

    print(f"  Zero-trade candles (n=0): {zero_trade} / {len(candles)}")
    print(f"  Gaps > expected interval: {len(gaps)}")
    if gaps:
        for idx, gap_min, t1, t2 in gaps[:5]:  # show first 5
            print(f"    Gap at index {idx}: {gap_min:.1f} min ({t1} -> {t2})")
        max_gap = max(g[1] for g in gaps)
        print(f"  Largest gap: {max_gap:.1f} min")

    # Show T field format
    if candles:
        c = candles[0]
        print(f"  Sample candle fields: t={c['t']} T={c['T']} o={c['o']} h={c['h']} l={c['l']} c={c['c']} v={c['v']} n={c['n']}")
        t_diff = c['T'] - c['t']
        print(f"  T - t = {t_diff} ms ({t_diff/60000:.3f} min) -- note T is close time, not next open")

def main():
    now_ms = int(time.time() * 1000)
    start_ms = now_ms - (120 * 60 * 1000)  # 2 hours

    print("=" * 70)
    print("Testing candle gaps for various liquidity levels (2h of 1m candles)")
    print("=" * 70)

    # Test various assets
    coins = ["BTC", "ETH", "DOGE", "SHIB", "PEPE", "MEME"]
    for coin in coins:
        try:
            candles = fetch_candles(coin, "1m", start_ms, now_ms)
            analyze_gaps(coin, candles)
        except Exception as e:
            print(f"  {coin}: Error - {e}")
        print()

    # Test a perpDex asset (HIP-3)
    print("=" * 70)
    print("Testing perpDex (HIP-3) assets")
    print("=" * 70)
    perpdex_coins = ["xyz:XYZ100"]
    for coin in perpdex_coins:
        try:
            candles = fetch_candles(coin, "1m", start_ms, now_ms)
            analyze_gaps(coin, candles)
        except Exception as e:
            print(f"  {coin}: Error - {e}")
        print()

    # Test: What does the response look like when a period has NO trading at all?
    # Use a very old time range that's beyond 5000 candle window
    print("=" * 70)
    print("Testing: Empty response (time range beyond 5000 candle window)")
    print("=" * 70)
    old_start = now_ms - (365 * 24 * 60 * 60 * 1000)  # 1 year ago
    old_end = old_start + (36 * 60 * 1000)
    try:
        candles = fetch_candles("DOGE", "1m", old_start, old_end)
        print(f"  DOGE 1yr ago: {len(candles)} candles (expect 0 if beyond window)")
    except Exception as e:
        print(f"  Error: {e}")

if __name__ == "__main__":
    main()
