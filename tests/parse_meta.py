"""Parse and display ALL Hyperliquid asset classes: perps, perpDex, and spot.

Usage:
    python parse_meta.py              # Summary of all asset classes
    python parse_meta.py --full       # Full asset listings
    python parse_meta.py --spot       # Spot-only deep dive
    python parse_meta.py --perpdex    # PerpDex-only deep dive
    python parse_meta.py --compare    # Compare plugin assumptions vs reality
    python parse_meta.py --json       # Dump raw JSON responses
"""
import json
import sys
import urllib.request

API_URL = "https://api.hyperliquid.xyz/info"


def api_post(payload):
    """POST to Hyperliquid info endpoint and return parsed JSON."""
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(API_URL, data=data,
                                headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=15) as resp:
        return json.loads(resp.read())


# =============================================================================
# FETCHERS
# =============================================================================

def fetch_perp_meta():
    """Fetch regular perpetual futures metadata."""
    return api_post({"type": "meta"})


def fetch_spot_meta():
    """Fetch spot token and pair metadata."""
    return api_post({"type": "spotMeta"})


def fetch_spot_meta_and_ctxs():
    """Fetch spot metadata + market context (prices, volume)."""
    return api_post({"type": "spotMetaAndAssetCtxs"})


def fetch_perpdex_list():
    """Fetch list of perpDex (builder-deployed perp) platforms."""
    return api_post({"type": "perpDexs"})


def fetch_perpdex_meta(dex_name):
    """Fetch metadata for a specific perpDex."""
    return api_post({"type": "meta", "dex": dex_name})


def fetch_all_mids(dex=None):
    """Fetch all mid prices. Default includes perps + spot."""
    payload = {"type": "allMids"}
    if dex:
        payload["dex"] = dex
    return api_post(payload)


# =============================================================================
# DISPLAY HELPERS
# =============================================================================

def print_header(title):
    print(f"\n{'=' * 70}")
    print(f"  {title}")
    print(f"{'=' * 70}")


def print_subheader(title):
    print(f"\n--- {title} ---")


# =============================================================================
# ANALYSIS: REGULAR PERPS
# =============================================================================

def analyze_perps(meta, verbose=False):
    print_header("REGULAR PERPS (asset IDs 0..N-1)")
    universe = meta.get("universe", [])
    print(f"Total assets: {len(universe)}")

    # Collect szDecimals distribution
    sz_dist = {}
    for asset in universe:
        sz = asset.get("szDecimals", "?")
        sz_dist[sz] = sz_dist.get(sz, 0) + 1

    print(f"\nszDecimals distribution:")
    for sz in sorted(sz_dist.keys()):
        px = max(0, 6 - sz) if isinstance(sz, int) else "?"
        print(f"  szDecimals={sz}: {sz_dist[sz]} assets  (pxDecimals={px})")

    # Show first/last few and some well-known assets
    print(f"\nSample assets (index -> name):")
    well_known = {"BTC", "ETH", "SOL", "DOGE", "HYPE"}
    for i, asset in enumerate(universe):
        name = asset.get("name", "?")
        sz = asset.get("szDecimals", "?")
        lev = asset.get("maxLeverage", "?")
        px = max(0, 6 - sz) if isinstance(sz, int) else "?"
        if i < 5 or name in well_known or i == len(universe) - 1:
            print(f"  [{i:3d}] {name:<12s}  szDec={sz}  pxDec={px}  maxLev={lev}")

    if verbose:
        print_subheader("Full perp listing")
        for i, asset in enumerate(universe):
            name = asset.get("name", "?")
            sz = asset.get("szDecimals", "?")
            lev = asset.get("maxLeverage", "?")
            px = max(0, 6 - sz) if isinstance(sz, int) else "?"
            delisted = asset.get("isDelisted", False)
            flag = " [DELISTED]" if delisted else ""
            print(f"  [{i:3d}] {name:<12s}  szDec={sz}  pxDec={px}  maxLev={lev}{flag}")

    return universe


# =============================================================================
# ANALYSIS: PERPDEX (BUILDER-DEPLOYED PERPS)
# =============================================================================

def analyze_perpdex(verbose=False):
    print_header("PERPDEX / BUILDER-DEPLOYED PERPS (asset IDs 100000+)")

    dex_list = fetch_perpdex_list()
    print(f"Raw perpDexs response: {len(dex_list)} entries (first is null = default perps)")

    # Parse perpDex names and offsets
    dexes = []
    dex_index = 0
    for i, entry in enumerate(dex_list):
        if entry is None:
            print(f"  [{i}] null (default perps, offset=0)")
            continue
        name = entry.get("name", "?")
        full_name = entry.get("fullName", "")
        offset = 100000 + (dex_index + 1) * 10000
        dexes.append({"name": name, "fullName": full_name, "offset": offset,
                       "arrayIndex": i, "dexIndex": dex_index + 1})
        print(f"  [{i}] {name:<8s} ({full_name})  offset={offset}")
        dex_index += 1

    # Fetch meta for each perpDex
    total_assets = 0
    perpdex_issues = []  # Track assets where pxDecimals=0 is wrong
    for dex in dexes:
        print_subheader(f"PerpDex: {dex['name']} (offset={dex['offset']})")
        try:
            meta = fetch_perpdex_meta(dex["name"])
        except Exception as e:
            print(f"  ERROR fetching meta: {e}")
            continue

        universe = meta.get("universe", [])
        print(f"  Assets: {len(universe)}")
        total_assets += len(universe)

        for j, asset in enumerate(universe):
            name = asset.get("name", "?")
            sz = asset.get("szDecimals", "?")
            lev = asset.get("maxLeverage", "?")
            isolated = asset.get("onlyIsolated", False)
            delisted = asset.get("isDelisted", False)
            asset_id = dex["offset"] + j

            # Calculate what pxDecimals SHOULD be (same formula as regular perps)
            correct_px = max(0, 6 - sz) if isinstance(sz, int) else "?"

            # Flag if plugin's hardcoded pxDecimals=0 would be wrong
            if isinstance(correct_px, int) and correct_px != 0:
                perpdex_issues.append({
                    "dex": dex["name"], "asset": name, "assetId": asset_id,
                    "szDecimals": sz, "correct_pxDecimals": correct_px,
                    "plugin_pxDecimals": 0
                })

            flags = []
            if isolated:
                flags.append("ISO")
            if delisted:
                flags.append("DELISTED")
            flag_str = f"  [{','.join(flags)}]" if flags else ""

            if verbose or j < 3 or j == len(universe) - 1:
                print(f"    [{asset_id}] {name:<16s}  szDec={sz}  pxDec={correct_px}  maxLev={lev}{flag_str}")

        if not verbose and len(universe) > 4:
            print(f"    ... ({len(universe) - 4} more)")

    print(f"\nTotal perpDex assets across all dexes: {total_assets}")

    # Report OPM-111 bug impact
    if perpdex_issues:
        print_subheader(f"BUG OPM-111: {len(perpdex_issues)} assets affected by hardcoded pxDecimals=0")
        for issue in perpdex_issues[:10]:
            print(f"  {issue['dex']}:{issue['asset']}  szDec={issue['szDecimals']}  "
                  f"plugin=0  correct={issue['correct_pxDecimals']}")
        if len(perpdex_issues) > 10:
            print(f"  ... and {len(perpdex_issues) - 10} more")

    return dexes, perpdex_issues


# =============================================================================
# ANALYSIS: SPOT
# =============================================================================

def analyze_spot(verbose=False):
    print_header("SPOT TOKENS & PAIRS (asset IDs 10000+)")

    spot = fetch_spot_meta()
    tokens = spot.get("tokens", [])
    universe = spot.get("universe", [])

    print(f"Tokens:  {len(tokens)}")
    print(f"Pairs:   {len(universe)}")

    # Build token index -> token info map
    token_map = {}
    for tok in tokens:
        token_map[tok.get("index", -1)] = tok

    # Analyze tokens
    print_subheader("Token overview")
    canonical_count = sum(1 for t in tokens if t.get("isCanonical", False))
    print(f"Canonical tokens: {canonical_count} / {len(tokens)}")

    sz_dist = {}
    for tok in tokens:
        sz = tok.get("szDecimals", "?")
        sz_dist[sz] = sz_dist.get(sz, 0) + 1
    print(f"\nToken szDecimals distribution:")
    for sz in sorted(sz_dist.keys()):
        px = max(0, 8 - sz) if isinstance(sz, int) else "?"
        print(f"  szDecimals={sz}: {sz_dist[sz]} tokens  (pxDecimals={px}, MAX_DECIMALS=8)")

    # Well-known tokens
    print_subheader("Well-known tokens")
    well_known = {"USDC", "HYPE", "PURR", "BTC", "ETH", "SOL"}
    for tok in tokens:
        if tok.get("name") in well_known:
            print(f"  {tok['name']:<8s}  index={tok.get('index', '?'):>4}  "
                  f"szDec={tok.get('szDecimals', '?')}  "
                  f"weiDec={tok.get('weiDecimals', '?')}  "
                  f"canonical={tok.get('isCanonical', '?')}")

    if verbose:
        print_subheader("All tokens")
        for tok in tokens[:50]:
            evm = tok.get("evmContract")
            evm_str = f"  evm={evm['address'][:10]}..." if evm else ""
            print(f"  {tok.get('name', '?'):<10s}  idx={tok.get('index', '?'):>4}  "
                  f"szDec={tok.get('szDecimals', '?')}  "
                  f"weiDec={tok.get('weiDecimals', '?')}  "
                  f"canonical={tok.get('isCanonical', '?')}{evm_str}")
        if len(tokens) > 50:
            print(f"  ... ({len(tokens) - 50} more)")

    # Analyze pairs (universe)
    print_subheader("Spot pairs")
    canonical_pairs = sum(1 for p in universe if p.get("isCanonical", False))
    print(f"Canonical pairs: {canonical_pairs} / {len(universe)}")

    # Show canonical pairs
    print(f"\nCanonical pairs:")
    for pair in universe:
        if pair.get("isCanonical", False):
            pair_tokens = pair.get("tokens", [])
            base = token_map.get(pair_tokens[0], {}).get("name", "?") if len(pair_tokens) > 0 else "?"
            quote = token_map.get(pair_tokens[1], {}).get("name", "?") if len(pair_tokens) > 1 else "?"
            spot_index = pair.get("index", "?")
            asset_id = 10000 + spot_index if isinstance(spot_index, int) else "?"
            print(f"  [{asset_id}] {pair.get('name', '?'):<14s}  "
                  f"base={base} (idx {pair_tokens[0]})  quote={quote} (idx {pair_tokens[1]})")

    # Show some well-known non-canonical pairs
    print(f"\nSample non-canonical pairs (use @N naming):")
    shown = 0
    for pair in universe:
        if pair.get("isCanonical", False):
            continue
        pair_tokens = pair.get("tokens", [])
        base = token_map.get(pair_tokens[0], {}).get("name", "?") if len(pair_tokens) > 0 else "?"
        quote = token_map.get(pair_tokens[1], {}).get("name", "?") if len(pair_tokens) > 1 else "?"
        spot_index = pair.get("index", "?")
        asset_id = 10000 + spot_index if isinstance(spot_index, int) else "?"

        # Show well-known or first few
        if base in {"HYPE", "BTC", "ETH", "SOL"} or shown < 5:
            base_tok = token_map.get(pair_tokens[0], {})
            sz = base_tok.get("szDecimals", "?")
            px = max(0, 8 - sz) if isinstance(sz, int) else "?"
            print(f"  [{asset_id}] @{spot_index:<4}  {base}/{quote:<8s}  "
                  f"szDec={sz}  pxDec={px}  "
                  f"(API coin: \"@{spot_index}\")")
            shown += 1
        if shown >= 10:
            break

    if verbose:
        print_subheader("All spot pairs")
        for pair in universe:
            pair_tokens = pair.get("tokens", [])
            base = token_map.get(pair_tokens[0], {}).get("name", "?") if len(pair_tokens) > 0 else "?"
            quote = token_map.get(pair_tokens[1], {}).get("name", "?") if len(pair_tokens) > 1 else "?"
            spot_index = pair.get("index", "?")
            asset_id = 10000 + spot_index if isinstance(spot_index, int) else "?"
            canonical = "C" if pair.get("isCanonical", False) else " "
            base_tok = token_map.get(pair_tokens[0], {})
            sz = base_tok.get("szDecimals", "?")
            print(f"  [{asset_id}] {canonical} @{spot_index:<4}  {base}/{quote:<8s}  szDec={sz}")

    return tokens, universe, token_map


# =============================================================================
# ANALYSIS: ALLMIDS COVERAGE
# =============================================================================

def analyze_allmids():
    print_header("ALLMIDS COVERAGE")

    # Default allMids (perps + spot)
    mids = fetch_all_mids()
    perp_keys = [k for k in mids if "/" not in k and "@" not in k and ":" not in k]
    spot_keys = [k for k in mids if "/" in k or "@" in k]

    print(f"Default allMids: {len(mids)} total")
    print(f"  Perp keys: {len(perp_keys)}  (e.g., {perp_keys[:3]})")
    print(f"  Spot keys: {len(spot_keys)}  (e.g., {spot_keys[:3]})")

    # Show some spot mid prices
    print_subheader("Sample spot mid prices from allMids")
    spot_samples = ["PURR/USDC"]
    for i in range(1, 10):
        spot_samples.append(f"@{i}")
    for key in spot_samples:
        if key in mids:
            print(f"  {key:<14s}  mid={mids[key]}")

    # PerpDex allMids
    dex_list = fetch_perpdex_list()
    for entry in dex_list:
        if entry is None:
            continue
        dex_name = entry.get("name", "?")
        try:
            dex_mids = fetch_all_mids(dex=dex_name)
            print(f"\n  allMids(dex=\"{dex_name}\"): {len(dex_mids)} assets")
            sample = list(dex_mids.items())[:3]
            for k, v in sample:
                print(f"    {k:<16s}  mid={v}")
        except Exception as e:
            print(f"\n  allMids(dex=\"{dex_name}\"): ERROR - {e}")


# =============================================================================
# COMPARISON: PLUGIN ASSUMPTIONS VS REALITY
# =============================================================================

def compare_plugin_vs_reality():
    print_header("PLUGIN ASSUMPTIONS vs REALITY")

    meta = fetch_perp_meta()
    spot = fetch_spot_meta()

    perp_universe = meta.get("universe", [])
    spot_tokens = spot.get("tokens", [])
    spot_universe = spot.get("universe", [])

    # 1. PIPCost assumption
    print_subheader("PIPCost formula")
    print("Plugin assumes: PIPCost = 10^(-6) = 0.000001 for ALL assets (constant)")
    print("  Perps:  MAX_DECIMALS=6  -> PIPCost = 10^(-6)  = 0.000001  CORRECT")
    print("  Spot:   MAX_DECIMALS=8  -> PIPCost = 10^(-8)  = 0.00000001  DIFFERENT!")
    print("  Impact: Spot PIPCost is 100x smaller than perps")

    # 2. pxDecimals formula
    print_subheader("pxDecimals formula")
    print("Plugin (perps):    pxDecimals = 6 - szDecimals")
    print("Plugin (perpDex):  pxDecimals = 0  [HARDCODED - BUG OPM-111]")
    print("Correct (perpDex): pxDecimals = 6 - szDecimals  (same as perps)")
    print("Correct (spot):    pxDecimals = 8 - szDecimals")

    # 3. Asset naming
    print_subheader("Asset naming in API calls")
    print("Perps:     coin name directly     (\"BTC\", \"ETH\")")
    print("PerpDex:   dex:coin format         (\"xyz:TSLA\")")
    print("Spot:      canonical or @index     (\"PURR/USDC\" or \"@107\")")
    print("")
    print("Plugin Zorro names (proposed):")
    print("  Perps:   BTC, ETH, SOL")
    print("  PerpDex: xyz:TSLA, flx:GOLD")
    print("  Spot:    HYPE-SPOT, BTC-SPOT, PURR-SPOT  (needs convention)")

    # 4. Position model
    print_subheader("Position data model")
    print("Perps/PerpDex: positions[] with size, entryPx, leverage, liquidationPx, marginUsed")
    print("Spot:          balances[] with coin, token, hold, total, entryNtl")
    print("  -> Spot has NO margin, leverage, or liquidation concept")

    # 5. Order differences
    print_subheader("Order placement differences")
    print("Same format for all three, but:")
    print("  Asset ID:     perp=index, spot=10000+index, perpDex=100000+offset+index")
    print("  reduceOnly:   meaningful for perps, meaningless for spot")
    print("  grouping:     perps can use positionTpsl, spot must use 'na'")
    print("  Price round:  perps 5 sigfig within 6 dec, spot 5 sigfig within 8 dec")

    # 6. Show actual szDecimals for well-known spot tokens
    token_map = {}
    for tok in spot_tokens:
        token_map[tok.get("name", "")] = tok

    print_subheader("Spot token szDecimals for well-known assets")
    for name in ["USDC", "HYPE", "PURR", "BTC", "ETH", "SOL"]:
        tok = token_map.get(name)
        if tok:
            sz = tok.get("szDecimals", "?")
            px = max(0, 8 - sz) if isinstance(sz, int) else "?"
            print(f"  {name:<8s}  szDec={sz}  pxDec={px}  weiDec={tok.get('weiDecimals', '?')}")
        else:
            # Try UBTC, UETH etc (some are remapped)
            for alt in [f"U{name}", f"W{name}"]:
                tok = token_map.get(alt)
                if tok:
                    sz = tok.get("szDecimals", "?")
                    px = max(0, 8 - sz) if isinstance(sz, int) else "?"
                    print(f"  {name:<8s}  (as {alt})  szDec={sz}  pxDec={px}  "
                          f"weiDec={tok.get('weiDecimals', '?')}")
                    break
            else:
                print(f"  {name:<8s}  NOT FOUND in spot tokens")


# =============================================================================
# JSON DUMP
# =============================================================================

def dump_json():
    print_header("RAW JSON RESPONSES")

    print_subheader("meta (first 3 universe entries)")
    meta = fetch_perp_meta()
    sample = {"universe": meta.get("universe", [])[:3]}
    print(json.dumps(sample, indent=2))

    print_subheader("spotMeta (first 3 tokens + first 3 universe)")
    spot = fetch_spot_meta()
    sample = {
        "tokens": spot.get("tokens", [])[:3],
        "universe": spot.get("universe", [])[:3],
    }
    print(json.dumps(sample, indent=2))

    print_subheader("spotMetaAndAssetCtxs (first 3 contexts)")
    spot_ctxs = fetch_spot_meta_and_ctxs()
    if isinstance(spot_ctxs, list) and len(spot_ctxs) >= 2:
        sample = spot_ctxs[1][:3]
        print(json.dumps(sample, indent=2))

    print_subheader("perpDexs")
    dex_list = fetch_perpdex_list()
    print(json.dumps(dex_list, indent=2))

    # First perpDex meta (first 3 entries)
    for entry in dex_list:
        if entry is None:
            continue
        dex_name = entry.get("name", "?")
        print_subheader(f"meta(dex=\"{dex_name}\") — first 3 entries")
        dex_meta = fetch_perpdex_meta(dex_name)
        sample = {"universe": dex_meta.get("universe", [])[:3]}
        print(json.dumps(sample, indent=2))
        break  # Only show first perpDex


# =============================================================================
# MAIN
# =============================================================================

def main():
    args = set(sys.argv[1:])

    if "--json" in args:
        dump_json()
        return

    if "--compare" in args:
        compare_plugin_vs_reality()
        return

    # Default: summary of all
    print("Fetching Hyperliquid asset metadata...")
    print(f"Endpoint: {API_URL}")

    meta = fetch_perp_meta()
    analyze_perps(meta, verbose="--full" in args)

    if "--spot" in args or not ("--perpdex" in args):
        analyze_spot(verbose="--full" in args or "--spot" in args)

    if "--perpdex" in args or not ("--spot" in args):
        analyze_perpdex(verbose="--full" in args or "--perpdex" in args)

    if "--full" in args:
        analyze_allmids()

    # Always show summary
    print_header("ASSET CLASS SUMMARY")
    perp_count = len(meta.get("universe", []))
    spot = fetch_spot_meta()
    spot_pairs = len(spot.get("universe", []))
    spot_tokens = len(spot.get("tokens", []))

    dex_list = fetch_perpdex_list()
    dex_count = sum(1 for d in dex_list if d is not None)

    print(f"  Regular perps:  {perp_count} assets   (IDs 0..{perp_count - 1})")
    print(f"  Spot tokens:    {spot_tokens} tokens, {spot_pairs} pairs  (IDs 10000+)")
    print(f"  PerpDex:        {dex_count} platforms (IDs 100000+)")
    print(f"\n  Formula reference:")
    print(f"    Perps:    pxDecimals = 6 - szDecimals  |  PIPCost = 10^(-6)")
    print(f"    Spot:     pxDecimals = 8 - szDecimals  |  PIPCost = 10^(-8)")
    print(f"    PerpDex:  pxDecimals = 6 - szDecimals  |  PIPCost = 10^(-6)")


if __name__ == "__main__":
    main()
