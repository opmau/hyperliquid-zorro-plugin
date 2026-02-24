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

namespace hl {
namespace account {

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
            // [OPM-19] Perps equity + spot USDC = total balance visible in HL UI.
            // clearinghouseState only returns perps data; spot USDC is separate.
            result.accountValue = accData.accountValue + accData.spotUSDC;
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
// POSITION IMPLEMENTATION
// =============================================================================

PositionInfo getPosition(const char* coin) {
    PositionInfo result;
    if (!coin) return result;

    // Try WebSocket cache first
    if (g_config.enableWebSocket && g_priceCache) {
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
    }

    // No position found
    result.coin = coin;
    return result;
}

std::vector<PositionInfo> getAllPositions() {
    std::vector<PositionInfo> positions;

    if (!g_config.enableWebSocket || !g_priceCache) return positions;

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
    // No local cache to clear - data is in ws_price_cache
    // This would clear cached HTTP responses if we had them
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

    // Response is a JSON string like "User", "Agent", "Vault", "Subaccount", "Missing"
    if (resp.body.find("Agent") != std::string::npos) return UserRole::Agent;
    if (resp.body.find("Subaccount") != std::string::npos) return UserRole::Subaccount;
    if (resp.body.find("Vault") != std::string::npos) return UserRole::Vault;
    if (resp.body.find("Missing") != std::string::npos) return UserRole::Missing;
    if (resp.body.find("User") != std::string::npos) return UserRole::User;

    return UserRole::Unknown;
}

} // namespace account
} // namespace hl
