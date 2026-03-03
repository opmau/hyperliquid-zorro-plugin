//=============================================================================
// hl_msgpack.h - Minimal MessagePack encoder for Hyperliquid order signing
//=============================================================================
// LAYER: Foundation
// DEPENDENCIES: None
// THREAD SAFETY: Packer instances are not thread-safe; use separate instances
//
// This provides a minimal MessagePack encoder implementing only the subset
// needed for Hyperliquid order action encoding. The format must match the
// Python SDK exactly for signature compatibility.
//=============================================================================

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace hl {
namespace msgpack {

using ByteArray = std::vector<uint8_t>;

/// Minimal MessagePack packer for Hyperliquid order encoding
class Packer {
public:
    void packString(const std::string& str);
    void packInt(int64_t v);
    void packBool(bool v);
    void packMapHeader(size_t size);
    void packArrayHeader(size_t size);

    const ByteArray& data() const { return data_; }
    void clear() { data_.clear(); }

private:
    ByteArray data_;

    void packUint8(uint8_t v);
    void packUint32(uint32_t v);
    void packPositiveInt(uint64_t v);
    void packNegativeInt(int64_t v);
};

// --- Hyperliquid-specific order encoding ---

/// Pack a Hyperliquid order action for signing.
/// Key order must match Python SDK exactly for signature compatibility.
ByteArray packOrderAction(
    int asset,
    bool isBuy,
    const std::string& price,
    const std::string& size,
    bool reduceOnly,
    const std::string& tif,
    const std::string& cloid,
    const std::string& grouping = "na"
);

/// Pack a Hyperliquid trigger (stop-loss/take-profit) order action for signing.
/// Uses "trigger" type instead of "limit" for the "t" field. [OPM-77]
/// Key order must match Python SDK exactly for signature compatibility.
ByteArray packTriggerOrderAction(
    int asset,
    bool isBuy,
    const std::string& price,       // Slippage-protected worst-case price
    const std::string& size,
    bool reduceOnly,
    bool isMarket,                   // true = market execution when triggered
    const std::string& triggerPx,    // Trigger activation price
    const std::string& tpsl,         // "tp" or "sl"
    const std::string& cloid,
    const std::string& grouping = "na"
);

/// Pack the inner OrderWire fields into an existing Packer.
/// Shared helper for both order placement and batchModify.
/// Field order: a, b, p, s, r, t, [c]  (matches Python SDK order_request_to_order_wire)
/// @param isTrigger  If true, packs trigger type instead of limit type
/// @param triggerIsMarket  Only used when isTrigger=true
/// @param triggerPx  Only used when isTrigger=true
/// @param tpsl  Only used when isTrigger=true ("tp" or "sl")
void packOrderWire(
    Packer& packer,
    int asset,
    bool isBuy,
    const std::string& price,
    const std::string& size,
    bool reduceOnly,
    const std::string& tif,
    const std::string& cloid,
    bool isTrigger = false,
    bool triggerIsMarket = true,
    const std::string& triggerPx = "",
    const std::string& tpsl = ""
);

/// Pack a Hyperliquid cancel action for signing.
/// Key order must match Python SDK exactly for signature compatibility.
ByteArray packCancelAction(int asset, uint64_t orderId);

/// Pack a Hyperliquid batchModify action for signing. [OPM-80]
/// Field order: {"type":"batchModify","modifies":[{"oid":N,"order":{...}}]}
/// Verified against Python SDK bulk_modify_orders_new() in exchange.py:195-223.
/// @param oidIsCloid  If true, oid is packed as string (cloid hex); else as int.
ByteArray packBatchModifyAction(
    uint64_t oidNumeric,
    const std::string& oidCloid,
    bool oidIsCloid,
    int asset,
    bool isBuy,
    const std::string& price,
    const std::string& size,
    bool reduceOnly,
    const std::string& tif,
    const std::string& cloid
);

/// Pack a Hyperliquid TWAP order action for signing. [OPM-81]
/// Field order: {"type":"twapOrder","twap":{"a","b","s","r","m","t"}}
/// Verified against Chainstack TWAP guide + HL API docs.
ByteArray packTwapOrderAction(
    int asset,
    bool isBuy,
    const std::string& size,
    bool reduceOnly,
    int minutes,
    bool randomize
);

/// Pack a Hyperliquid TWAP cancel action for signing. [OPM-81]
/// Field order: {"type":"twapCancel","a":asset,"t":twapId}
ByteArray packTwapCancelAction(int asset, uint64_t twapId);

/// Pack a Hyperliquid scheduleCancel action for signing. [OPM-83]
/// @param time  Cancel time in UTC milliseconds (0 = clear/unschedule)
ByteArray packScheduleCancelAction(uint64_t time);

/// Describes one order in a bracket group (entry, TP, or SL) [OPM-79]
struct BracketOrderWire {
    int asset;
    bool isBuy;
    std::string price;      // Formatted limit/slippage price
    std::string size;       // Formatted size
    bool reduceOnly;
    std::string tif;        // "Ioc","Gtc","Alo" — ignored for triggers
    std::string cloid;
    bool isTrigger;
    bool triggerIsMarket;
    std::string triggerPx;  // Formatted trigger price
    std::string tpsl;       // "tp" or "sl"
};

/// Pack a bracket (grouped) order action for signing. [OPM-79]
/// Encodes multiple orders in a single action with normalTpsl grouping.
/// {"type":"order","orders":[{entry},{tp},{sl}],"grouping":"normalTpsl"}
ByteArray packBracketOrderAction(
    const std::vector<BracketOrderWire>& orders,
    const std::string& grouping = "normalTpsl"
);

} // namespace msgpack
} // namespace hl
