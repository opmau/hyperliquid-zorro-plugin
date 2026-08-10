//=============================================================================
// hl_account_spot.h - Spot clearinghouse state and abstraction mode (I/O)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_account_collateral.h, hl_globals.h, hl_http.h, ws_price_cache.h
// THREAD SAFETY: All public functions are thread-safe
//
// The I/O shell around hl_account_collateral.h: fetches the payloads, caches
// the session mode, publishes to the price cache. All arithmetic and parsing
// lives in the pure header so the unit tests can link it directly. [OPM-824]
//
// Split out of hl_account_service.cpp because the perps clearinghouse state and
// the spot clearinghouse state answer different questions — and on a unified
// account it is the SPOT side that holds the tradeable collateral.
//=============================================================================

#pragma once

#include "hl_account_collateral.h"

namespace hl {
namespace account {

// =============================================================================
// ACCOUNT ABSTRACTION MODE
// =============================================================================

/// Query the abstraction mode over HTTP. Caches the result for the session.
AbstractionMode queryAbstractionMode();

/// Query the mode at most once per session, then return the cached value.
///
/// queryAbstractionMode() used to be reachable from exactly one branch of
/// BrokerAccount, so a login that fell back to HTTP never learned the mode at
/// all. Call this instead — repeat calls cost nothing. [OPM-824]
AbstractionMode ensureAbstractionMode();

/// Cached mode from the last query (Unknown if never queried).
AbstractionMode getAbstractionMode();

/// Forget the cached mode so the next session re-queries it. Call at logout.
void resetAbstractionMode();

// =============================================================================
// SPOT CLEARINGHOUSE STATE
// =============================================================================

/// Fetch spotClearinghouseState over HTTP and publish it to the price cache.
///
/// Records three things beyond the USDC balance itself: whether spot
/// collateralizes perps, Hyperliquid's own free-collateral figure, and that a
/// response was parsed at all — so getBalance() can tell "0 USDC" apart from
/// "never fetched".
///
/// @return Spot USDC `total` (0 if the query fails or there is no USDC entry)
double refreshSpotBalance();

/// Re-fetch the spot state when the cached snapshot is older than the TTL.
///
/// Spot state has no WS channel, and the stale-WS fallback in BrokerAccount
/// does not run while account events keep the WS fresh — nothing else bounds
/// this snapshot's age. A failed fetch does not count as a refresh; the next
/// call retries. [OPM-878]
///
/// @return true if a fetch ran (caller should re-read the balance)
bool ensureSpotFresh();

} // namespace account
} // namespace hl
