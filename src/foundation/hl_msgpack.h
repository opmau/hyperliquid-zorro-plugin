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

/// Pack a Hyperliquid cancel action for signing.
/// Key order must match Python SDK exactly for signature compatibility.
ByteArray packCancelAction(int asset, uint64_t orderId);

} // namespace msgpack
} // namespace hl
