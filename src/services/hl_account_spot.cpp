//=============================================================================
// hl_account_spot.cpp - Spot clearinghouse state and abstraction mode (I/O)
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, hl_http.h, ws_price_cache.h
//=============================================================================

#include "hl_account_spot.h"
#include "../foundation/hl_globals.h"
#include "../transport/hl_http.h"
#include "../transport/ws_price_cache.h"
#include <cstdio>
#include <cstring>

namespace hl {
namespace account {

// =============================================================================
// SESSION STATE
// =============================================================================

static AbstractionMode s_abstractionMode = AbstractionMode::Unknown;
static bool s_abstractionQueried = false;

// =============================================================================
// ACCOUNT ABSTRACTION MODE
// =============================================================================

void resetAbstractionMode() {
    s_abstractionMode = AbstractionMode::Unknown;
    s_abstractionQueried = false;
}

AbstractionMode queryAbstractionMode() {
    s_abstractionQueried = true;

    if (!g_config.walletAddress[0]) {
        s_abstractionMode = AbstractionMode::Unknown;
        return s_abstractionMode;
    }

    char body[256];
    sprintf_s(body, "{\"type\":\"userAbstraction\",\"user\":\"%s\"}", g_config.walletAddress);

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        g_logger.log(1, "queryAbstractionMode: HTTP request failed, mode Unknown");
        s_abstractionMode = AbstractionMode::Unknown;
        return s_abstractionMode;
    }

    if (g_config.diagLevel >= 1) {
        g_logger.logf(1, "queryAbstractionMode: response=%s", resp.body.c_str());
    }

    s_abstractionMode = parseAbstractionMode(resp.body.c_str());
    if (s_abstractionMode == AbstractionMode::Unknown) {
        // Hyperliquid has added a mode name before. Say so rather than guess —
        // the balance arithmetic reads the response shape, not this value.
        g_logger.logf(1, "queryAbstractionMode: unrecognized mode %s", resp.body.c_str());
    }

    g_logger.logf(1, "Account abstraction mode: %s", abstractionModeName(s_abstractionMode));
    return s_abstractionMode;
}

AbstractionMode ensureAbstractionMode() {
    if (s_abstractionQueried) return s_abstractionMode;
    return queryAbstractionMode();
}

AbstractionMode getAbstractionMode() {
    return s_abstractionMode;
}

// =============================================================================
// SPOT CLEARINGHOUSE STATE
// =============================================================================

double refreshSpotBalance() {
    char body[256];
    sprintf_s(body, "{\"type\":\"spotClearinghouseState\",\"user\":\"%s\"}",
              g_config.walletAddress);

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        g_logger.log(1, "refreshSpotBalance: HTTP request failed or empty");
        return 0.0;
    }

    if (g_config.diagLevel >= 1) {
        char snippet[512];
        strncpy_s(snippet, resp.body.c_str(), sizeof(snippet) - 1);
        snippet[sizeof(snippet) - 1] = '\0';
        g_logger.logf(1, "refreshSpotBalance: HTTP response (%zu bytes): %s",
                      resp.body.length(), snippet);
    }

    SpotState spot;
    if (!parseSpotClearinghouseState(resp.body.c_str(), resp.body.length(), spot)) {
        g_logger.log(1, "refreshSpotBalance: JSON parse error");
        return 0.0;
    }

    if (g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
        cache->setSpotState(spot.usdcTotal, spot.availAfterMaint, spot.unifiedPool);
    }

    // Cross-check the shape against the declared mode. They agreed on every
    // account sampled; a disagreement means Hyperliquid changed something and
    // the operator needs to know, because this figure sizes positions.
    AbstractionMode mode = getAbstractionMode();
    if (mode != AbstractionMode::Unknown && modePoolsSpot(mode) != spot.unifiedPool) {
        g_logger.logf(1, "refreshSpotBalance: WARNING mode %s disagrees with spot "
                         "response shape (unified pool=%s) — using the shape",
                      abstractionModeName(mode), spot.unifiedPool ? "yes" : "no");
    }

    if (g_config.diagLevel >= 1) {
        g_logger.logf(1, "refreshSpotBalance: USDC total=%.6f unifiedPool=%s "
                         "availAfterMaint=%.6f",
                      spot.usdcTotal, spot.unifiedPool ? "yes" : "no",
                      spot.availAfterMaint);
    }

    return spot.usdcTotal;
}

} // namespace account
} // namespace hl
