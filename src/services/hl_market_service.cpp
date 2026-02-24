//=============================================================================
// hl_market_service.cpp - Market data service implementation
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, hl_meta.h, ws_price_cache.h, hl_http.h
//=============================================================================

#include "hl_market_service.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include "../transport/ws_price_cache.h"
#include "../transport/ws_manager.h"
#include <map>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace hl {
namespace market {

// =============================================================================
// INTERNAL STATE
// =============================================================================

// HTTP seed cooldown tracking (coin -> last seed time in GetTickCount)
static std::map<std::string, DWORD> s_seedTimes;
static CRITICAL_SECTION s_seedCs;
static bool s_seedCsInit = false;

// Initialize critical section on first use
static void ensureSeedCsInit() {
    if (!s_seedCsInit) {
        InitializeCriticalSection(&s_seedCs);
        s_seedCsInit = true;
    }
}

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
// HTTP SEED COOLDOWN
// =============================================================================

bool canSeedHttp(const char* coin) {
    if (!coin || !g_config.enableHttpSeed) return false;

    ensureSeedCsInit();
    EnterCriticalSection(&s_seedCs);

    auto it = s_seedTimes.find(std::string(coin));
    DWORD now = GetTickCount();
    bool canSeed = true;

    if (it != s_seedTimes.end()) {
        DWORD elapsed = now - it->second;
        canSeed = (elapsed >= (DWORD)g_config.httpSeedCooldownMs);
    }

    LeaveCriticalSection(&s_seedCs);
    return canSeed;
}

void recordSeed(const char* coin) {
    if (!coin) return;

    ensureSeedCsInit();
    EnterCriticalSection(&s_seedCs);
    s_seedTimes[std::string(coin)] = GetTickCount();
    LeaveCriticalSection(&s_seedCs);
}

void clearSeedCooldowns() {
    ensureSeedCsInit();
    EnterCriticalSection(&s_seedCs);
    s_seedTimes.clear();
    LeaveCriticalSection(&s_seedCs);
}

void cleanup() {
    if (s_seedCsInit) {
        EnterCriticalSection(&s_seedCs);
        s_seedTimes.clear();
        LeaveCriticalSection(&s_seedCs);
        DeleteCriticalSection(&s_seedCs);
        s_seedCsInit = false;
    }
}

// =============================================================================
// PRICE ACCESS
// =============================================================================

PriceData getPrice(const char* coin, uint32_t maxAgeMs) {
    PriceData result;
    if (!coin || !*coin) return result;

    // Check if this is a perpDex coin (has colon separator)
    char perpDex[32] = {0};
    char coinOnly[64] = {0};
    if (http::parsePerpDex(coin, perpDex, sizeof(perpDex), coinOnly, sizeof(coinOnly))) {
        return getPerpDexPrice(perpDex, coinOnly, maxAgeMs);
    }

    // Build API coin name (no prefix for regular perps)
    std::string apiCoin = coin;

    // Try WebSocket cache first
    // Track stale data as fallback: prices under PRICE_STALE_MS (5s) are usable
    // when HTTP cooldown blocks a refresh. Beyond 5s, data is unreliable.
    double staleBid = 0.0, staleAsk = 0.0;

    if (g_config.enableWebSocket && g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);

        double bid = cache->getBid(apiCoin);
        double ask = cache->getAsk(apiCoin);
        DWORD age = cache->getAge(apiCoin);

        if (bid > 0.0 && ask > 0.0 && age < maxAgeMs) {
            result.bid = bid;
            result.ask = ask;
            result.mid = (bid + ask) / 2.0;
            result.timestamp = GetTickCount();
            return result;
        }

        // Remember stale data if under hard staleness cap (PRICE_STALE_MS = 5s)
        // Beyond 5s the data is unreliable — return empty and let Zorro handle it
        if (bid > 0.0 && ask > 0.0 && age < config::PRICE_STALE_MS) {
            staleBid = bid;
            staleAsk = ask;
        }

        if (g_config.diagLevel >= 2) {
            char msg[128];
            if (bid > 0.0 && ask > 0.0) {
                sprintf_s(msg, "%s stale (age=%dms, max=%d)", coin, age, maxAgeMs);
            } else {
                sprintf_s(msg, "%s no WS data", coin);
            }
            logMsg(2, "getPrice", msg);
        }

        // Wait for WS data to arrive (subscription may still be in flight).
        // Typical arrival: 100-200ms. Avoids unnecessary HTTP fallback storm
        // that correlates with WS error 12030. [OPM-106]
        //
        // Health shortcut: skip the wait when WS is unhealthy (no message in
        // 60s). If nothing arrived in 60s, 1.5s more won't help — go straight
        // to HTTP fallback.
        if (bid <= 0.0 || ask <= 0.0) {
            bool wsHealthy = g_wsManager &&
                reinterpret_cast<hl::ws::WebSocketManager*>(g_wsManager)->isHealthy();

            if (wsHealthy) {
                DWORD waitStart = GetTickCount();
                while (GetTickCount() - waitStart < config::WS_FIRST_DATA_WAIT_MS) {
                    Sleep(50);
                    bid = cache->getBid(apiCoin);
                    ask = cache->getAsk(apiCoin);
                    if (bid > 0.0 && ask > 0.0) {
                        DWORD waited = GetTickCount() - waitStart;
                        char msg[128];
                        sprintf_s(msg, "%s WS data arrived after %dms", coin, waited);
                        logMsg(1, "getPrice", msg);
                        result.bid = bid;
                        result.ask = ask;
                        result.mid = (bid + ask) / 2.0;
                        result.timestamp = GetTickCount();
                        return result;
                    }
                }
                if (g_config.diagLevel >= 1) {
                    char msg[128];
                    sprintf_s(msg, "%s WS wait timed out (%dms)", coin, config::WS_FIRST_DATA_WAIT_MS);
                    logMsg(1, "getPrice", msg);
                }
            } else {
                logMsg(1, "getPrice", "WS unhealthy, skipping wait — direct HTTP fallback");
            }
        }
    }

    // HTTP fallback - only if cooldown allows
    if (!canSeedHttp(coin)) {
        if (g_config.diagLevel >= 2) {
            char msg[128];
            sprintf_s(msg, "HTTP seed skipped (cooldown) for %s", coin);
            logMsg(2, "getPrice", msg);
        }
        // Return stale WS data (< 5s old) rather than empty — prevents BrokerAsset
        // returning 0 which triggers Error 053 and disables the asset for the session
        if (staleBid > 0.0 && staleAsk > 0.0) {
            result.bid = staleBid;
            result.ask = staleAsk;
            result.mid = (staleBid + staleAsk) / 2.0;
            result.timestamp = GetTickCount();
        }
        return result;
    }

    // Fetch via HTTP l2Book
    char payload[128];
    sprintf_s(payload, "{\"type\":\"l2Book\",\"coin\":\"%s\",\"nSigFigs\":5}", coin);

    http::Response resp = http::infoPost(payload, true);
    if (!resp.success()) {
        logMsg(1, "getPrice", "HTTP l2Book fetch failed");
        return result;
    }

    // Parse bid/ask from response using yyjson
    // Format: {"levels":[[{"px":"50000",...}],[{"px":"50001",...}]]}
    const char* jsonStr = resp.body.c_str();
    yyjson_doc* doc = yyjson_read(jsonStr, resp.body.size(), 0);
    if (!doc) return result;
    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* levels = json::getArray(root, "levels");
    if (!levels) { yyjson_doc_free(doc); return result; }

    double httpBid = 0.0, httpAsk = 0.0;
    yyjson_val* bids = yyjson_arr_get(levels, 0);
    yyjson_val* asks = yyjson_arr_get(levels, 1);
    if (bids && yyjson_is_arr(bids)) {
        yyjson_val* topBid = yyjson_arr_get(bids, 0);
        if (topBid) httpBid = json::getDouble(topBid, "px");
    }
    if (asks && yyjson_is_arr(asks)) {
        yyjson_val* topAsk = yyjson_arr_get(asks, 0);
        if (topAsk) httpAsk = json::getDouble(topAsk, "px");
    }
    yyjson_doc_free(doc);

    if (httpBid > 0.0 && httpAsk > 0.0) {
        result.bid = httpBid;
        result.ask = httpAsk;
        result.mid = (httpBid + httpAsk) / 2.0;
        result.timestamp = GetTickCount();

        // Update WS cache for consistency
        if (g_priceCache) {
            auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
            cache->setBidAsk(apiCoin, httpBid, httpAsk);
        }

        recordSeed(coin);

        if (g_config.diagLevel >= 1) {
            char msg[128];
            sprintf_s(msg, "HTTP fallback for %s: bid=%.4f ask=%.4f", coin, httpBid, httpAsk);
            logMsg(1, "getPrice", msg);
        }
    }

    return result;
}

PriceData getPerpDexPrice(const char* perpDex, const char* coin, uint32_t maxAgeMs) {
    PriceData result;
    if (!perpDex || !coin) return result;

    // Build API coin name with prefix
    char apiCoin[64];
    sprintf_s(apiCoin, "%s:%s", perpDex, coin);

    // Try WebSocket cache first
    if (g_config.enableWebSocket && g_priceCache) {
        auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);

        double bid = cache->getBid(std::string(apiCoin));
        double ask = cache->getAsk(std::string(apiCoin));
        DWORD age = cache->getAge(std::string(apiCoin));

        if (bid > 0.0 && ask > 0.0 && age < maxAgeMs) {
            result.bid = bid;
            result.ask = ask;
            result.mid = (bid + ask) / 2.0;
            result.timestamp = GetTickCount();
            return result;
        }
    }

    // HTTP fallback for perpDex
    if (!canSeedHttp(apiCoin)) return result;

    char payload[128];
    sprintf_s(payload, "{\"type\":\"l2Book\",\"coin\":\"%s\",\"nSigFigs\":5}", apiCoin);

    http::Response resp = http::infoPost(payload, true);
    if (!resp.success()) return result;

    // Parse bid/ask from l2Book response using yyjson
    // Format: {"levels":[[{"px":"50000",...}],[{"px":"50001",...}]]}
    const char* jsonStr = resp.body.c_str();
    yyjson_doc* doc = yyjson_read(jsonStr, resp.body.size(), 0);
    if (!doc) return result;
    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* levels = json::getArray(root, "levels");
    if (!levels) { yyjson_doc_free(doc); return result; }

    double httpBid = 0.0, httpAsk = 0.0;
    yyjson_val* bids = yyjson_arr_get(levels, 0);
    yyjson_val* asks = yyjson_arr_get(levels, 1);
    if (bids && yyjson_is_arr(bids)) {
        yyjson_val* topBid = yyjson_arr_get(bids, 0);
        if (topBid) httpBid = json::getDouble(topBid, "px");
    }
    if (asks && yyjson_is_arr(asks)) {
        yyjson_val* topAsk = yyjson_arr_get(asks, 0);
        if (topAsk) httpAsk = json::getDouble(topAsk, "px");
    }
    yyjson_doc_free(doc);

    if (httpBid > 0.0 && httpAsk > 0.0) {
        result.bid = httpBid;
        result.ask = httpAsk;
        result.mid = (httpBid + httpAsk) / 2.0;
        result.timestamp = GetTickCount();
        recordSeed(apiCoin);
    } else if (httpBid > 0.0) {
        // Fallback: only bid available (thin order book)
        result.bid = httpBid;
        result.ask = httpBid;
        result.mid = httpBid;
        result.timestamp = GetTickCount();
        recordSeed(apiCoin);
    }

    return result;
}

bool hasRealtimePrice(const char* coin, uint32_t maxAgeMs) {
    if (!coin || !g_config.enableWebSocket || !g_priceCache) return false;

    auto* cache = reinterpret_cast<hl::ws::PriceCache*>(g_priceCache);
    return cache->isFresh(std::string(coin), maxAgeMs);
}

void subscribePrice(const char* coin) {
    if (!coin || !g_config.enableWebSocket || !g_wsManager) return;

    // Cast to correct type and subscribe
    auto* mgr = reinterpret_cast<hl::ws::WebSocketManager*>(g_wsManager);
    mgr->subscribeL2Book(std::string(coin));
}

// =============================================================================
// CANDLE INTERVAL HELPERS
// =============================================================================

CandleInterval minutesToInterval(int minutes) {
    switch (minutes) {
        case 1:    return CandleInterval::M1;
        case 3:    return CandleInterval::M3;
        case 5:    return CandleInterval::M5;
        case 15:   return CandleInterval::M15;
        case 30:   return CandleInterval::M30;
        case 60:   return CandleInterval::H1;
        case 120:  return CandleInterval::H2;
        case 240:  return CandleInterval::H4;
        case 1440: return CandleInterval::D1;
        default:   return CandleInterval::M1;
    }
}

const char* intervalToString(CandleInterval interval) {
    switch (interval) {
        case CandleInterval::M1:  return "1m";
        case CandleInterval::M3:  return "3m";
        case CandleInterval::M5:  return "5m";
        case CandleInterval::M15: return "15m";
        case CandleInterval::M30: return "30m";
        case CandleInterval::H1:  return "1h";
        case CandleInterval::H2:  return "2h";
        case CandleInterval::H4:  return "4h";
        case CandleInterval::D1:  return "1d";
        default: return "1m";
    }
}

int intervalToMinutes(CandleInterval interval) {
    switch (interval) {
        case CandleInterval::M1:  return 1;
        case CandleInterval::M3:  return 3;
        case CandleInterval::M5:  return 5;
        case CandleInterval::M15: return 15;
        case CandleInterval::M30: return 30;
        case CandleInterval::H1:  return 60;
        case CandleInterval::H2:  return 120;
        case CandleInterval::H4:  return 240;
        case CandleInterval::D1:  return 1440;
        default: return 1;
    }
}

// =============================================================================
// HISTORICAL DATA
// =============================================================================

std::vector<Candle> getCandles(const char* coin, CandleInterval interval,
                               int64_t startTimeMs, int64_t endTimeMs,
                               int maxCandles) {
    std::vector<Candle> candles;
    if (!coin) return candles;

    const char* intervalStr = intervalToString(interval);
    int barMinutes = intervalToMinutes(interval);

    // Build request payload
    char payload[512];
    sprintf_s(payload,
        "{\"type\":\"candleSnapshot\",\"req\":{\"coin\":\"%s\",\"interval\":\"%s\","
        "\"startTime\":%lld,\"endTime\":%lld}}",
        coin, intervalStr, startTimeMs, endTimeMs);

    if (g_config.diagLevel >= 2) {
        char msg[256];
        sprintf_s(msg, "Fetching candles: %s %s from %lld to %lld",
                 coin, intervalStr, startTimeMs, endTimeMs);
        logMsg(2, "getCandles", msg);
    }

    http::Response resp = http::infoPost(payload, false);
    if (!resp.success()) {
        logMsg(1, "getCandles", "API request failed");
        return candles;
    }

    // Parse response: [{"t":12345,"o":"100","h":"101","l":"99","c":"100.5","v":"1000"}]
    const char* jsonStr = resp.body.c_str();
    yyjson_doc* candleDoc = yyjson_read(jsonStr, resp.body.size(), 0);
    if (!candleDoc) return candles;
    yyjson_val* candleRoot = yyjson_doc_get_root(candleDoc);
    if (!yyjson_is_arr(candleRoot)) { yyjson_doc_free(candleDoc); return candles; }

    size_t apiCount = yyjson_arr_size(candleRoot);
    candles.reserve(maxCandles);

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(candleRoot, idx, max, item) {
        if (candles.size() >= (size_t)maxCandles) break;

        // Parse timestamp (milliseconds, bar OPEN time)
        int64_t tMs = json::getInt64(item, "t");
        if (tMs == 0) continue;

        // Convert to bar END time
        int64_t endTime = tMs + ((int64_t)barMinutes * 60 * 1000);

        // Parse OHLCV
        Candle candle;
        candle.open   = json::getDouble(item, "o");
        candle.high   = json::getDouble(item, "h");
        candle.low    = json::getDouble(item, "l");
        candle.close  = json::getDouble(item, "c");
        candle.volume = json::getDouble(item, "v");
        candle.timestamp = endTime;

        candles.push_back(candle);
    }
    yyjson_doc_free(candleDoc);

    if (g_config.diagLevel >= 1) {
        char msg[256];
        if (candles.empty()) {
            sprintf_s(msg, "getCandles: %s %s — API returned %zu items, parsed 0",
                     coin, intervalStr, apiCount);
        } else {
            sprintf_s(msg, "getCandles: %s %s — API returned %zu, parsed %zu (first=%lld last=%lld)",
                     coin, intervalStr, apiCount, candles.size(),
                     candles.front().timestamp, candles.back().timestamp);
        }
        logMsg(1, "getCandles", msg);
    }

    return candles;
}

std::vector<Candle> getCandlesByMinutes(const char* coin, int barMinutes,
                                        int64_t startTimeMs, int64_t endTimeMs,
                                        int maxCandles) {
    return getCandles(coin, minutesToInterval(barMinutes), startTimeMs, endTimeMs, maxCandles);
}

// =============================================================================
// ASSET METADATA (delegates to hl_meta)
// =============================================================================

const AssetInfo* getAsset(const char* name) {
    if (!name) return nullptr;
    int idx = meta::findAssetIndex(name);
    if (idx < 0) return nullptr;
    return g_assets.getByIndex(idx);
}

const AssetInfo* getAssetByIndex(int index) {
    return g_assets.getByIndex(index);
}

int findAssetIndex(const char* coin) {
    return meta::findAssetIndex(coin);
}

int getAssetCount() {
    return meta::getTotalAssetCount();
}

int refreshMeta() {
    int mainCount = meta::fetchMeta();
    if (mainCount <= 0) return 0;

    // Also fetch perpDex metadata
    int perpDexCount = meta::fetchAllPerpDexMeta();

    // Fetch spot pair metadata
    int spotCount = meta::fetchSpotMeta();

    // Populate WS index mappings if available
    meta::populateWsIndexMappings();

    return mainCount + perpDexCount + spotCount;
}

bool isMetaLoaded() {
    return meta::isMetaLoaded();
}

} // namespace market
} // namespace hl
