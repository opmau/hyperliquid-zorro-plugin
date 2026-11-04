//=============================================================================
// hl_trade_uuid.h - Self-describing trade identifiers for Zorro's UUID mode
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: hl_types.h
// THREAD SAFETY: Stateless, all functions thread-safe
//
// Zorro identifies a trade either by an int or, when BrokerBuy2 returns -1, by
// a string it reads with GET_UUID and hands back with SET_UUID before every
// later close, query or cancel of that trade. Zorro stores that string with
// the trade, so it survives a Zorro restart, which an in-memory int mapping
// does not.
//
// The string carries everything needed to act on the trade without any
// plugin-side state:
//
//   HL1|<coin>|<L|S>|<cloid>      e.g.  HL1|xyz:TSLA|L|0x0000000200000000...
//
// coin  - the coin exactly as the exchange API names it (may contain ':' or '/')
// L|S   - side of the opening order
// cloid - the client order ID the opening order was placed with
//=============================================================================

#pragma once

#include "hl_types.h"
#include <cstdio>
#include <cstring>
#include <cctype>

namespace hl {
namespace uuid {

struct TradeUuid {
    char coin[32] = {0};
    OrderSide side = OrderSide::Buy;
    char cloid[64] = {0};
};

static const char UUID_PREFIX[] = "HL1|";

// A cloid as the exchange accepts it: "0x" followed by 32 hex digits.
inline bool isCloid(const char* s) {
    if (!s || s[0] != '0' || (s[1] != 'x' && s[1] != 'X')) return false;
    for (int i = 2; i < 34; i++) {
        if (!isxdigit((unsigned char)s[i])) return false;
    }
    return s[34] == '\0';
}

// Build the identifier. False (and an empty `out`) when an input is unusable
// or the result does not fit.
inline bool format(const char* coin, OrderSide side, const char* cloid,
                   char* out, size_t outSize) {
    if (!out || outSize == 0) return false;
    out[0] = '\0';
    if (!coin || !coin[0] || strlen(coin) >= sizeof(TradeUuid::coin)) return false;
    if (strchr(coin, '|')) return false;
    if (!isCloid(cloid)) return false;
    int n = snprintf(out, outSize, "%s%s|%c|%s", UUID_PREFIX, coin,
                     side == OrderSide::Buy ? 'L' : 'S', cloid);
    if (n < 0 || (size_t)n >= outSize) {
        out[0] = '\0';
        return false;
    }
    return true;
}

// Parse an identifier made by format(). Strict: anything else is rejected, so
// a foreign or corrupted string can never be acted on as a trade.
inline bool parse(const char* s, TradeUuid& out) {
    out = TradeUuid();
    if (!s || strncmp(s, UUID_PREFIX, sizeof(UUID_PREFIX) - 1) != 0) return false;
    const char* coin = s + sizeof(UUID_PREFIX) - 1;
    const char* bar = strchr(coin, '|');
    if (!bar) return false;
    size_t coinLen = (size_t)(bar - coin);
    if (coinLen == 0 || coinLen >= sizeof(out.coin)) return false;

    char sideCh = bar[1];
    if ((sideCh != 'L' && sideCh != 'S') || bar[2] != '|') return false;
    const char* cloid = bar + 3;
    if (!isCloid(cloid)) return false;

    memcpy(out.coin, coin, coinLen);
    out.coin[coinLen] = '\0';
    out.side = (sideCh == 'L') ? OrderSide::Buy : OrderSide::Sell;
    memcpy(out.cloid, cloid, 35);  // "0x" + 32 hex + terminator
    return true;
}

} // namespace uuid
} // namespace hl
