//=============================================================================
// hl_account_service.cpp - Account state service implementation
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, ws_price_cache.h, hl_http.h
//=============================================================================

#include "hl_account_service.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../transport/hl_http.h"
#include "../transport/ws_price_cache.h"
#include "../transport/ws_manager.h"
#include "../transport/ws_parsers.h"
#include "../transport/json_helpers.h"
#include <cstdio>
#include <cstring>
#include <set>

namespace hl {
namespace account {

// =============================================================================
// SESSION STATE
// =============================================================================

static AbstractionMode s_abstractionMode = AbstractionMode::Unknown;

// =============================================================================
// LOGGING HELPER
// =============================================================================

static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

// =============================================================================
// BALANCE IMPLEMENTATION
// =============================================================================

bool Balance::isStale(uint32_t maxAgeMs) const {
    if (timestamp == 0) return true;
    DWORD now = GetTickCount();
    DWORD age = (now >= timestamp) ? (now - timestamp) : 0;
    return age > maxAgeMs;
}

Balance getBalance(uint32_t maxAgeMs) {
    Balance result;

    // Try WebSocket cache first
    if (g_config.enableWebSocket && g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
        ws::AccountData accData = cache->getAccountData();
        DWORD age = cache->getAccountDataAge();

        // age < maxAgeMs means WS delivered data recently (even if balance is 0)
        // age == MAXDWORD means WS never delivered clearinghouseState
        if (age < maxAgeMs) {
            result.dataReceived = true;
            // [OPM-200] Only add spot USDC for standard accounts.
            // Unified/PortfolioMargin: crossMarginSummary.accountValue already includes spot.
            // Unknown (query failure): assume unified (the default) to avoid inflation.
            if (s_abstractionMode == AbstractionMode::Standard) {
                result.accountValue = accData.accountValue + accData.spotUSDC;
            } else {
                result.accountValue = accData.accountValue;
            }
            result.withdrawable = accData.withdrawable;
            result.marginUsed = accData.totalMarginUsed;
            result.totalNotional = accData.totalNtlPos;
            result.timestamp = GetTickCount();

            if (g_config.diagLevel >= 2) {
                char msg[256];
                sprintf_s(msg, "WS: perps=%.2f spot=%.2f total=%.2f margin=%.2f (age=%dms)",
                         accData.accountValue, accData.spotUSDC,
                         result.accountValue, result.marginUsed, age);
                logMsg(2, "getBalance", msg);
            }
            return result;
        }

        // Log stale/missing data
        if (g_config.diagLevel >= 2) {
            char msg[128];
            if (age != MAXDWORD) {
                sprintf_s(msg, "WS stale (age=%dms, max=%d)", age, maxAgeMs);
            } else {
                sprintf_s(msg, "WS no data");
            }
            logMsg(2, "getBalance", msg);
        }
    }

    // No fresh WS data - return empty (caller should retry or use HTTP)
    return result;
}

bool hasRealtimeBalance(uint32_t maxAgeMs) {
    if (!g_config.enableWebSocket || !g_priceCache) return false;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    ws::AccountData accData = cache->getAccountData();
    DWORD age = cache->getAccountDataAge();

    return (accData.accountValue > 0 && age < maxAgeMs);
}

bool refreshBalance() {
    // Fetch via HTTP
    char body[256];
    sprintf_s(body, "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}", g_config.walletAddress);

    if (g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "HTTP clearinghouseState for address: %s", g_config.walletAddress);
        logMsg(1, "refreshBalance", msg);
    }

    http::Response resp = http::infoPost(body, false);
    if (!resp.success()) {
        logMsg(1, "refreshBalance", "HTTP request failed");
        return false;
    }

    if (resp.body.empty()) {
        logMsg(1, "refreshBalance", "HTTP response body empty");
        return false;
    }

    // Parse response using same parser as WS channel.
    // HTTP clearinghouseState returns the same JSON structure:
    //   {"assetPositions":[...],"marginSummary":{...},"crossMarginSummary":{...},"withdrawable":"..."}
    // The parser handles both marginSummary and crossMarginSummary,
    // using the higher accountValue to support unified accounts (Issue #23).
    if (g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
        hl::ws::parseClearinghouseState(*cache, resp.body.c_str(),
                                        g_config.diagLevel, g_logger.callback);
    }

    // Log response at level 1 so it's visible even with SET_DIAGNOSTICS=1
    if (g_config.diagLevel >= 1) {
        // Truncate to first 512 chars for logging
        char snippet[512];
        strncpy_s(snippet, resp.body.c_str(), sizeof(snippet) - 1);
        snippet[sizeof(snippet) - 1] = '\0';
        g_logger.logf(1, "refreshBalance: HTTP response (%zu bytes): %s",
                       resp.body.length(), snippet);
    }

    return true;
}

double refreshSpotBalance() {
    // Fetch spotClearinghouseState via HTTP
    // Response: {"balances":[{"coin":"USDC","token":0,"hold":"0.0","total":"204.50",...},...]
    char body[256];
    sprintf_s(body, "{\"type\":\"spotClearinghouseState\",\"user\":\"%s\"}", g_config.walletAddress);

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        logMsg(1, "refreshSpotBalance", "HTTP request failed or empty");
        return 0.0;
    }

    // Log raw response at level 1
    if (g_config.diagLevel >= 1) {
        char snippet[512];
        strncpy_s(snippet, resp.body.c_str(), sizeof(snippet) - 1);
        snippet[sizeof(snippet) - 1] = '\0';
        g_logger.logf(1, "refreshSpotBalance: HTTP response (%zu bytes): %s",
                       resp.body.length(), snippet);
    }

    // Parse with yyjson
    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.length(), 0);
    if (!doc) {
        logMsg(1, "refreshSpotBalance", "JSON parse error");
        return 0.0;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* balances = json::getArray(root, "balances");

    double usdcTotal = 0.0;
    if (balances) {
        size_t idx, max;
        yyjson_val* item;
        yyjson_arr_foreach(balances, idx, max, item) {
            char coin[32] = {0};
            if (json::getString(item, "coin", coin, sizeof(coin))) {
                if (_stricmp(coin, "USDC") == 0) {
                    usdcTotal = json::getDouble(item, "total");
                    break;
                }
            }
        }
    }
    yyjson_doc_free(doc);

    // Store in PriceCache
    if (g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
        cache->setSpotUSDC(usdcTotal);
    }

    if (g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "Spot USDC balance: %.2f", usdcTotal);
        logMsg(1, "refreshSpotBalance", msg);
    }

    return usdcTotal;
}

// =============================================================================
// PERPDEX POSITION QUERIES [OPM-212]
// =============================================================================

// PerpDex names the strategy has explicitly subscribed to via BrokerAsset. [OPM-439]
// Main-thread-only (populated from BrokerAsset, read from ensurePositionData on the
// same thread), so no synchronization is required.
static std::set<std::string> s_activePerpDexes;

void markPerpDexActive(const char* perpDex) {
    if (!perpDex || !*perpDex) return;
    s_activePerpDexes.insert(perpDex);
}

// Return perpDex names the strategy has asked to subscribe to. [OPM-439]
// Previously scanned the whole asset registry, which caused 8 clearinghouseState
// subs for single-account strategies that never touched a perpDex.
static std::set<std::string> getActivePerpDexNames() {
    return s_activePerpDexes;
}

// [OPM-218] Subscribe to WS clearinghouseState for each known perpDex.
// Called when perpDex assets are discovered (after asset loading).
static bool s_perpDexWsSubscribed = false;

static void subscribePerpDexChannels() {
    if (s_perpDexWsSubscribed) return;
    if (!g_config.enableWebSocket || !g_wsManager) return;

    auto dexes = getActivePerpDexNames();
    if (dexes.empty()) return;

    auto* mgr = reinterpret_cast<hl::ws::WebSocketManager*>(g_wsManager);
    for (const auto& dex : dexes)
        mgr->subscribeClearinghouseStateDex(dex);

    s_perpDexWsSubscribed = true;
    if (g_config.diagLevel >= 1)
        g_logger.logf(1, "subscribePerpDexChannels: subscribed %d perpDex(es)", (int)dexes.size());
}

// Fetch clearinghouseState for each known perpDex via HTTP.
// Each perpDex has its own position set (not returned by the default query).
static bool s_perpDexPositionsFetched = false;

static void refreshPerpDexPositions() {
    auto dexes = getActivePerpDexNames();
    if (dexes.empty()) {
        // Expected steady state when no perpDex subscribed — verbose only.
        // Was level 1 → flooded the async log queue every BrokerAccount tick,
        // risking real-message drops. [OPM-550 follow-up]
        if (g_config.diagLevel >= 3) {
            g_logger.logf(3, "refreshPerpDexPositions: no active perpDex names (g_assets has %d assets)",
                         g_assets.count);
        }
        return;
    }

    for (const auto& dex : dexes) {
        char body[256];
        sprintf_s(body, "{\"type\":\"clearinghouseState\",\"user\":\"%s\",\"dex\":\"%s\"}",
                  g_config.walletAddress, dex.c_str());

        if (g_config.diagLevel >= 1) {
            g_logger.logf(1, "refreshPerpDexPositions: querying dex=%s", dex.c_str());
        }

        http::Response resp = http::infoPost(body, false);
        if (!resp.success() || resp.body.empty()) {
            g_logger.logf(1, "refreshPerpDexPositions: dex=%s HTTP failed", dex.c_str());
            continue;
        }

        if (g_priceCache) {
            auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
            hl::ws::parseClearinghouseState(*cache, resp.body.c_str(),
                                            g_config.diagLevel, g_logger.callback,
                                            dex.c_str());
        }

        // [OPM-226] Log response snippet and resulting cache state
        if (g_config.diagLevel >= 1) {
            char snippet[256];
            strncpy_s(snippet, resp.body.c_str(), sizeof(snippet) - 1);
            snippet[sizeof(snippet) - 1] = '\0';
            g_logger.logf(1, "refreshPerpDexPositions: dex=%s response (%zu bytes): %.200s",
                         dex.c_str(), resp.body.length(), snippet);
        }
    }
    s_perpDexPositionsFetched = true;
}

// =============================================================================
// POSITION IMPLEMENTATION
// =============================================================================

// Rate-limited HTTP fallback for position data [OPM-134].
// refreshBalance() calls parseClearinghouseState which populates both
// balance AND positions in the PriceCache.
static bool ensurePositionData() {
    if (!g_priceCache) return false;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    DWORD posAge = cache->getPositionsAge();
    bool hasMainDexData = (posAge != MAXDWORD);

    // [OPM-219] PerpDex position fetching is decoupled from main-dex gate.
    // Subaccounts that only trade on perpDex may have empty main-dex data,
    // so perpDex positions must be fetched independently.
    subscribePerpDexChannels();
    if (!s_perpDexPositionsFetched) {
        refreshPerpDexPositions();
    }

    if (hasMainDexData) {
        return true;
    }

    // No main-dex position snapshot — rate-limited HTTP fallback
    static DWORD lastAttempt = 0;
    DWORD now = GetTickCount();
    DWORD elapsed = (now >= lastAttempt) ? (now - lastAttempt) : MAXDWORD;
    if (elapsed < config::POSITION_CACHE_MS) {
        return s_perpDexPositionsFetched;  // Have perpDex data at least
    }

    logMsg(1, "ensurePositionData", "No WS position data, HTTP fallback");
    lastAttempt = GetTickCount();
    bool ok = refreshBalance();
    return ok || s_perpDexPositionsFetched;
}

PositionInfo getPosition(const char* coin) {
    PositionInfo result;
    if (!coin) return result;

    // [OPM-226] Log the lookup key so we can trace what getPosition receives
    if (g_config.diagLevel >= 3) {
        g_logger.logf(3, "getPosition: lookup coin='%s'", coin);
    }

    // Ensure we have position data (WS or HTTP fallback) [OPM-134]
    ensurePositionData();

    // Read from cache (works for both WS and HTTP-populated data).
    //
    // [OPM-555] Cache keys are normalized at write time:
    //   - Main-dex positions stored as bare "COIN" (ws_parsers.cpp)
    //   - perpDex positions stored as "dex:COIN" (ws_parsers.cpp:202-209)
    // Callers in hl_broker.cpp use buildCoinForApi() which produces the same
    // format. An exact match is therefore the only correct lookup — earlier
    // OPM-219 fallbacks that stripped/added prefixes returned WRONG-DEX
    // positions when an asset existed on one dex but not the other.
    if (g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
        ws::PositionData wsPos = cache->getPosition(std::string(coin));

        if (wsPos.size != 0) {
            result.coin = wsPos.coin;
            result.size = wsPos.size;
            result.entryPrice = wsPos.entryPx;
            result.unrealizedPnl = wsPos.unrealizedPnl;
            result.leverage = wsPos.leverage;
            result.liquidationPrice = wsPos.liquidationPx;
            result.marginUsed = wsPos.marginUsed;
            result.timestamp = wsPos.timestamp;
            return result;
        }

        // [OPM-226] Position not found — dump cache keys for diagnosis
        if (g_config.diagLevel >= 1) {
            auto allPos = cache->getAllPositions();
            if (allPos.empty()) {
                g_logger.logf(1, "getPosition: '%s' NOT FOUND — cache is EMPTY", coin);
            } else {
                g_logger.logf(1, "getPosition: '%s' NOT FOUND — cache has %d positions:",
                              coin, (int)allPos.size());
                for (const auto& p : allPos) {
                    g_logger.logf(1, "  cache key='%s' size=%.6f dex='%s'",
                                  p.coin.c_str(), p.size, p.dex.c_str());
                }
            }
        }
    }

    // No position found
    result.coin = coin;
    return result;
}

std::vector<PositionInfo> getAllPositions() {
    std::vector<PositionInfo> positions;

    // Ensure we have position data (WS or HTTP fallback) [OPM-134]
    ensurePositionData();

    if (!g_priceCache) return positions;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    std::vector<ws::PositionData> wsPositions = cache->getAllPositions();

    for (const auto& wsPos : wsPositions) {
        if (wsPos.size == 0) continue;

        PositionInfo pos;
        pos.coin = wsPos.coin;
        pos.size = wsPos.size;
        pos.entryPrice = wsPos.entryPx;
        pos.unrealizedPnl = wsPos.unrealizedPnl;
        pos.leverage = wsPos.leverage;
        pos.liquidationPrice = wsPos.liquidationPx;
        pos.marginUsed = wsPos.marginUsed;
        pos.timestamp = wsPos.timestamp;
        positions.push_back(pos);
    }

    return positions;
}

bool hasPosition(const char* coin) {
    PositionInfo pos = getPosition(coin);
    return pos.size != 0;
}

double getPositionSize(const char* coin) {
    PositionInfo pos = getPosition(coin);
    return pos.size;
}

void applyFill(const char* coin, double fillSize, double fillPx, bool isBuy) {
    if (!coin || fillSize <= 0) return;
    if (!g_config.enableWebSocket || !g_priceCache) return;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    double signedFill = isBuy ? fillSize : -fillSize;

    ws::PositionData existing = cache->getPosition(std::string(coin));

    if (existing.size == 0) {
        // New position
        ws::PositionData pos;
        pos.coin = coin;
        pos.size = signedFill;
        pos.entryPx = fillPx;
        cache->setPosition(pos);
    } else {
        bool sameDirection = (existing.size > 0) == isBuy;

        if (sameDirection) {
            // Adding to position: weighted average entry price
            double oldNotional = fabs(existing.size) * existing.entryPx;
            double newNotional = fillSize * fillPx;
            double totalSize = fabs(existing.size) + fillSize;
            double avgEntry = (oldNotional + newNotional) / totalSize;

            ws::PositionData pos = existing;
            pos.size = (existing.size > 0) ? totalSize : -totalSize;
            pos.entryPx = avgEntry;
            cache->setPosition(pos);
        } else {
            // Reducing position: keep original entry price
            double remaining = fabs(existing.size) - fillSize;

            if (remaining <= 1e-12) {
                // Position fully closed
                ws::PositionData pos;
                pos.coin = coin;
                pos.size = 0;
                pos.entryPx = 0;
                cache->setPosition(pos);
            } else {
                ws::PositionData pos = existing;
                pos.size = (existing.size > 0) ? remaining : -remaining;
                cache->setPosition(pos);
            }
        }
    }

    if (g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "Applied fill: %s %s %.6f @ %.2f",
                  isBuy ? "BUY" : "SELL", coin, fillSize, fillPx);
        logMsg(1, "applyFill", msg);
    }
}

// =============================================================================
// ZORRO HELPERS
// =============================================================================

bool getZorroAccountValues(double* outBalance, double* outTradeVal, double* outMarginVal) {
    Balance bal = getBalance();

    if (bal.accountValue <= 0) return false;

    // Zorro expects: Balance + TradeVal = Total Equity
    // - pBalance = cash balance (withdrawable)
    // - pTradeVal = unrealized PnL (equity - cash)
    if (outBalance) *outBalance = bal.withdrawable;
    if (outTradeVal) *outTradeVal = bal.unrealizedPnl();
    if (outMarginVal) *outMarginVal = bal.marginUsed;

    return true;
}

PositionInfo convertWsPosition(const std::string& coin, double size, double entryPx,
                               double unrealizedPnl, double leverage) {
    PositionInfo pos;
    pos.size = size;
    pos.entryPrice = entryPx;
    pos.unrealizedPnl = unrealizedPnl;
    pos.leverage = leverage;
    pos.timestamp = GetTickCount();

    // Handle perpDex index format (e.g., "@110001")
    if (!coin.empty() && coin[0] == '@') {
        char friendlyCoin[64];
        if (meta::convertPerpDexIndexToCoin(coin.c_str(), friendlyCoin, sizeof(friendlyCoin))) {
            pos.coin = friendlyCoin;
        } else {
            pos.coin = coin;
        }
    } else {
        pos.coin = coin;
    }

    return pos;
}

// =============================================================================
// CACHE MANAGEMENT
// =============================================================================

void clearCache() {
    // Reset perpDex flags so positions are re-fetched/re-subscribed [OPM-212, OPM-218]
    s_perpDexPositionsFetched = false;
    s_perpDexWsSubscribed = false;
    // Drop the strategy-activated perpDex set so a fresh session starts clean [OPM-439]
    s_activePerpDexes.clear();
}

uint32_t getAccountDataAge() {
    if (!g_priceCache) return MAXDWORD;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    return cache->getAccountDataAge();
}

uint32_t getPositionsAge() {
    if (!g_priceCache) return MAXDWORD;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    return cache->getPositionsAge();
}

// =============================================================================
// SUBSCRIPTION
// =============================================================================

void subscribeAccountData() {
    if (!g_config.enableWebSocket || !g_wsManager) return;

    auto* mgr = reinterpret_cast<hl::ws::WebSocketManager*>(g_wsManager);
    mgr->subscribeAllAccountData();

    if (g_config.diagLevel >= 1) {
        logMsg(1, "subscribeAccountData", "Subscribed to clearinghouseState, openOrders, userFills");
    }
}

// =============================================================================
// ACCOUNT ABSTRACTION MODE [OPM-200]
// =============================================================================

AbstractionMode queryAbstractionMode() {
    if (!g_config.walletAddress[0]) {
        s_abstractionMode = AbstractionMode::Unknown;
        return s_abstractionMode;
    }

    char body[256];
    sprintf_s(body, "{\"type\":\"userAbstraction\",\"user\":\"%s\"}", g_config.walletAddress);

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        logMsg(1, "queryAbstractionMode", "HTTP request failed, assuming Unified (default)");
        s_abstractionMode = AbstractionMode::Unknown;
        return s_abstractionMode;
    }

    if (g_config.diagLevel >= 1) {
        g_logger.logf(1, "queryAbstractionMode: response=%s", resp.body.c_str());
    }

    // Response is a JSON string value: "disabled", "unifiedAccount", "portfolioMargin"
    if (resp.body.find("disabled") != std::string::npos) {
        s_abstractionMode = AbstractionMode::Standard;
    } else if (resp.body.find("portfolioMargin") != std::string::npos) {
        s_abstractionMode = AbstractionMode::PortfolioMargin;
    } else if (resp.body.find("unifiedAccount") != std::string::npos) {
        s_abstractionMode = AbstractionMode::Unified;
    } else {
        logMsg(1, "queryAbstractionMode", "Unrecognized response, assuming Unified");
        s_abstractionMode = AbstractionMode::Unknown;
    }

    const char* modeStr =
        (s_abstractionMode == AbstractionMode::Standard) ? "Standard" :
        (s_abstractionMode == AbstractionMode::Unified) ? "Unified" :
        (s_abstractionMode == AbstractionMode::PortfolioMargin) ? "PortfolioMargin" : "Unknown";
    g_logger.logf(1, "Account abstraction mode: %s", modeStr);

    return s_abstractionMode;
}

AbstractionMode getAbstractionMode() {
    return s_abstractionMode;
}

// =============================================================================
// ADDRESS VALIDATION
// =============================================================================

UserRole checkUserRole() {
    if (!g_config.walletAddress[0]) return UserRole::Unknown;

    char body[256];
    sprintf_s(body, "{\"type\":\"userRole\",\"user\":\"%s\"}", g_config.walletAddress);

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        logMsg(1, "checkUserRole", "HTTP request failed");
        return UserRole::Unknown;
    }

    if (g_config.diagLevel >= 2) {
        g_logger.logf(2, "checkUserRole: response=%s", resp.body.c_str());
    }

    // API returns {"role":"<value>"} with lowercase/camelCase values [OPM-202]
    // Order matters: "agent" response contains "user" in data field, so check "agent" first
    if (resp.body.find("agent") != std::string::npos) return UserRole::Agent;
    if (resp.body.find("subAccount") != std::string::npos) return UserRole::Subaccount;
    if (resp.body.find("vault") != std::string::npos) return UserRole::Vault;
    if (resp.body.find("missing") != std::string::npos) return UserRole::Missing;
    if (resp.body.find("user") != std::string::npos) return UserRole::User;

    return UserRole::Unknown;
}

} // namespace account
} // namespace hl
