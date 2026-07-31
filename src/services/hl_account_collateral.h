//=============================================================================
// hl_account_collateral.h - Pure collateral maths and payload parsing
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: json_helpers.h (yyjson) only
// THREAD SAFETY: Stateless pure functions, safe to call from any thread
//
// Header-only and free of globals, HTTP and Zorro on purpose: this is the
// arithmetic that decides the equity Zorro sizes positions from, so the unit
// tests link THIS code rather than a copy of it. hl_account_spot.cpp is the
// I/O shell that fetches the payloads and feeds them in here. [OPM-824]
//=============================================================================

#pragma once

#include "../transport/json_helpers.h"
#include <cstring>

namespace hl {
namespace account {

// =============================================================================
// ACCOUNT ABSTRACTION MODE [OPM-200, OPM-824]
// =============================================================================

/// Hyperliquid account abstraction mode, from the `userAbstraction` endpoint.
/// Determines how spot, perps and perpDex capital is pooled.
///
///   Standard ("disabled")       - separate pools; spot USDC is NOT collateral
///   Default  ("default")        - separate pools (Hyperliquid's default)
///   Unified  ("unifiedAccount") - ONE pool, held on the SPOT side
///   PortfolioMargin             - one pool, multi-collateral
enum class AbstractionMode { Unknown, Standard, Default, Unified, PortfolioMargin };

/// Map a raw `userAbstraction` response body to a mode.
///
/// The endpoint returns a bare JSON string. No two mode names share a
/// substring, so a plain search is unambiguous. [OPM-824] "default" was missing
/// from this mapping and fell through to Unknown on three live mainnet accounts.
inline AbstractionMode parseAbstractionMode(const char* body) {
    if (!body || !*body) return AbstractionMode::Unknown;
    if (strstr(body, "disabled"))        return AbstractionMode::Standard;
    if (strstr(body, "portfolioMargin")) return AbstractionMode::PortfolioMargin;
    if (strstr(body, "unifiedAccount"))  return AbstractionMode::Unified;
    if (strstr(body, "default"))         return AbstractionMode::Default;
    return AbstractionMode::Unknown;
}

/// Whether a mode pools spot USDC as perp collateral.
inline bool modePoolsSpot(AbstractionMode mode) {
    return mode == AbstractionMode::Unified || mode == AbstractionMode::PortfolioMargin;
}

/// Human-readable name, for logs.
inline const char* abstractionModeName(AbstractionMode mode) {
    switch (mode) {
        case AbstractionMode::Standard:        return "Standard(disabled)";
        case AbstractionMode::Default:         return "Default";
        case AbstractionMode::Unified:         return "Unified";
        case AbstractionMode::PortfolioMargin: return "PortfolioMargin";
        default:                               return "Unknown";
    }
}

// =============================================================================
// SPOT CLEARINGHOUSE STATE
// =============================================================================

/// Parsed spotClearinghouseState response.
struct SpotState {
    double usdcTotal = 0.0;       // balances[coin=="USDC"].total
    double availAfterMaint = 0.0; // tokenToAvailableAfterMaintenance for USDC
    bool unifiedPool = false;     // tokenToAvailableAfterMaintenance present
    bool valid = false;           // a response was parsed at all
};

/// Parse a spotClearinghouseState body.
///
/// Shape (verified live 2026-07-31, mainnet and testnet):
///   {"balances":[{"coin":"USDC","token":0,"hold":"0.0","total":"1234.567890",
///                 "entryNtl":"0.0"}, ...],
///    "tokenToAvailableAfterMaintenance":[[0,"1234.567890"]]}
///
/// `tokenToAvailableAfterMaintenance` is present only when spot collateralizes
/// perps. It was present on every unifiedAccount and portfolioMargin account
/// sampled and absent on every "default" and "disabled" one, which makes it a
/// more reliable signal than the mode string — the plugin cannot be caught out
/// by a mode name Hyperliquid has not shipped yet.
///
/// @return true if the body parsed as JSON (out.valid mirrors this)
inline bool parseSpotClearinghouseState(const char* body, size_t len, SpotState& out) {
    out = SpotState();
    if (!body || len == 0) return false;

    yyjson_doc* doc = yyjson_read(body, len, 0);
    if (!doc) return false;

    yyjson_val* root = yyjson_doc_get_root(doc);

    // USDC is token 0 on both networks, but read the index rather than assume it.
    long long usdcToken = 0;
    yyjson_val* balances = json::getArray(root, "balances");
    if (balances) {
        size_t idx, max;
        yyjson_val* item;
        yyjson_arr_foreach(balances, idx, max, item) {
            char coin[32] = {0};
            if (json::getString(item, "coin", coin, sizeof(coin)) &&
                _stricmp(coin, "USDC") == 0) {
                out.usdcTotal = json::getDouble(item, "total");
                usdcToken = json::getInt64(item, "token");
                break;
            }
        }
    }

    // Entries are [tokenIndex, "amountAvailableAfterMaintenanceMargin"].
    yyjson_val* avail = json::getArray(root, "tokenToAvailableAfterMaintenance");
    if (avail) {
        out.unifiedPool = true;
        size_t idx, max;
        yyjson_val* pair;
        yyjson_arr_foreach(avail, idx, max, pair) {
            if (!yyjson_is_arr(pair) || yyjson_arr_size(pair) < 2) continue;
            // Match on the token INDEX. A portfolio-margin account lists one
            // entry per collateral token, and USDC is not necessarily first.
            if (json::valToInt64(yyjson_arr_get(pair, 0)) != usdcToken) continue;
            out.availAfterMaint = json::valToDouble(yyjson_arr_get(pair, 1));
            break;
        }
    }

    yyjson_doc_free(doc);
    out.valid = true;
    return true;
}

// =============================================================================
// COLLATERAL MODEL
// =============================================================================

/// Tradeable equity and free collateral for one account.
struct CollateralView {
    double equity = 0.0;          // What Zorro receives as *pBalance
    double freeCollateral = 0.0;  // Equity available after maintenance margin
    bool unified = false;         // Spot USDC is the collateral pool
};

/// Derive tradeable equity from the perps and spot clearinghouse states.
///
/// Hyperliquid exposes two collateral models and the correct equity differs
/// completely between them:
///
///   Unified / portfolio margin — ONE pool, held on the spot side. Spot USDC
///     `total` is marked to market continuously and ALREADY contains every
///     perp's unrealized PnL, across every perp dex. It is the equity, on its
///     own. Adding the perps figure double-counts PnL (a $1 PnL move would
///     shift Balance by $2); using the perps figure alone under-reports — by
///     100% on a HIP-3 account, whose entire position set lives off dex 0.
///
///   Separate pools ("disabled" / "default") — spot USDC does not collateralize
///     perps, but is still capital the strategy can deploy, so the two add.
///
/// With no spot snapshot, reports the perps figure: that under-reports a
/// unified account, but reporting 0 would trip BrokerAccount's zero-balance
/// guard and halt the strategy outright.
///
/// @param perpsAccountValue clearinghouseState marginSummary.accountValue
/// @param totalMarginUsed   clearinghouseState marginSummary.totalMarginUsed
/// @param spot              parseSpotClearinghouseState output
inline CollateralView computeCollateral(double perpsAccountValue,
                                        double totalMarginUsed,
                                        const SpotState& spot) {
    CollateralView view;

    if (!spot.valid) {
        view.equity = perpsAccountValue;
        view.unified = false;
        view.freeCollateral = perpsAccountValue - totalMarginUsed;
        return view;
    }

    view.unified = spot.unifiedPool;
    if (spot.unifiedPool) {
        view.equity = spot.usdcTotal;
        view.freeCollateral = spot.availAfterMaint;
    } else {
        view.equity = perpsAccountValue + spot.usdcTotal;
        view.freeCollateral = view.equity - totalMarginUsed;
    }
    return view;
}

} // namespace account
} // namespace hl
