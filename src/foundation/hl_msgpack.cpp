//=============================================================================
// hl_msgpack.cpp - Minimal MessagePack encoder implementation
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Foundation
// DEPENDENCIES: hl_msgpack.h
// THREAD SAFETY: Packer instances are not thread-safe; use separate instances
//=============================================================================

#include "hl_msgpack.h"

namespace hl {
namespace msgpack {

// --- Packer public methods ---

void Packer::packString(const std::string& str) {
    size_t len = str.size();
    if (len <= 31) {
        // fixstr: 101xxxxx where xxxxx is length
        packUint8(0xa0 | static_cast<uint8_t>(len));
    } else if (len <= 255) {
        // str8
        packUint8(0xd9);
        packUint8(static_cast<uint8_t>(len));
    } else {
        // str16
        packUint8(0xda);
        packUint8((len >> 8) & 0xFF);
        packUint8(len & 0xFF);
    }
    data_.insert(data_.end(), str.begin(), str.end());
}

void Packer::packInt(int64_t v) {
    if (v >= 0) {
        packPositiveInt(static_cast<uint64_t>(v));
    } else {
        packNegativeInt(v);
    }
}

void Packer::packBool(bool v) {
    packUint8(v ? 0xc3 : 0xc2);
}

void Packer::packMapHeader(size_t size) {
    if (size <= 15) {
        // fixmap: 1000xxxx where xxxx is size
        packUint8(0x80 | static_cast<uint8_t>(size));
    } else if (size <= 0xFFFF) {
        packUint8(0xde);
        packUint8((size >> 8) & 0xFF);
        packUint8(size & 0xFF);
    } else {
        packUint8(0xdf);
        packUint32(static_cast<uint32_t>(size));
    }
}

void Packer::packArrayHeader(size_t size) {
    if (size <= 15) {
        // fixarray: 1001xxxx where xxxx is size
        packUint8(0x90 | static_cast<uint8_t>(size));
    } else if (size <= 0xFFFF) {
        packUint8(0xdc);
        packUint8((size >> 8) & 0xFF);
        packUint8(size & 0xFF);
    } else {
        packUint8(0xdd);
        packUint32(static_cast<uint32_t>(size));
    }
}

// --- Packer private methods ---

void Packer::packUint8(uint8_t v) {
    data_.push_back(v);
}

void Packer::packUint32(uint32_t v) {
    data_.push_back((v >> 24) & 0xFF);
    data_.push_back((v >> 16) & 0xFF);
    data_.push_back((v >> 8) & 0xFF);
    data_.push_back(v & 0xFF);
}

void Packer::packPositiveInt(uint64_t v) {
    if (v <= 127) {
        // positive fixint: 0xxxxxxx
        packUint8(static_cast<uint8_t>(v));
    } else if (v <= 0xFF) {
        packUint8(0xcc);
        packUint8(static_cast<uint8_t>(v));
    } else if (v <= 0xFFFF) {
        packUint8(0xcd);
        packUint8((v >> 8) & 0xFF);
        packUint8(v & 0xFF);
    } else if (v <= 0xFFFFFFFF) {
        packUint8(0xce);
        packUint32(static_cast<uint32_t>(v));
    } else {
        // uint64
        packUint8(0xcf);
        for (int i = 7; i >= 0; --i) {
            packUint8((v >> (i * 8)) & 0xFF);
        }
    }
}

void Packer::packNegativeInt(int64_t v) {
    if (v >= -32) {
        // negative fixint: 111xxxxx
        packUint8(static_cast<uint8_t>(v));
    } else if (v >= -128) {
        packUint8(0xd0);
        packUint8(static_cast<uint8_t>(v));
    } else if (v >= -32768) {
        packUint8(0xd1);
        packUint8((v >> 8) & 0xFF);
        packUint8(v & 0xFF);
    } else if (v >= -2147483648LL) {
        packUint8(0xd2);
        packUint32(static_cast<uint32_t>(v));
    } else {
        // int64
        packUint8(0xd3);
        for (int i = 7; i >= 0; --i) {
            packUint8((v >> (i * 8)) & 0xFF);
        }
    }
}

// --- Hyperliquid-specific order encoding ---

ByteArray packOrderAction(
    int asset,
    bool isBuy,
    const std::string& price,
    const std::string& size,
    bool reduceOnly,
    const std::string& tif,
    const std::string& cloid,
    const std::string& grouping
) {
    Packer packer;

    // Main map with 3 keys: "type", "orders", "grouping"
    // MUST be in this exact order to match Python SDK
    packer.packMapHeader(3);

    // 1. "type": "order" (FIRST!)
    packer.packString("type");
    packer.packString("order");

    // 2. "orders": [{order}] (SECOND!)
    packer.packString("orders");
    packer.packArrayHeader(1);

    // Pack the individual order inline
    int mapSize = cloid.empty() ? 6 : 7;
    packer.packMapHeader(mapSize);

    packer.packString("a");
    packer.packInt(asset);

    packer.packString("b");
    packer.packBool(isBuy);

    packer.packString("p");
    packer.packString(price);

    packer.packString("s");
    packer.packString(size);

    packer.packString("r");
    packer.packBool(reduceOnly);

    // "t": {"limit":{"tif":"Ioc"}}
    packer.packString("t");
    packer.packMapHeader(1);
    packer.packString("limit");
    packer.packMapHeader(1);
    packer.packString("tif");
    packer.packString(tif);

    if (!cloid.empty()) {
        packer.packString("c");
        packer.packString(cloid);
    }

    // 3. "grouping": "na" (THIRD!)
    packer.packString("grouping");
    packer.packString(grouping);

    return packer.data();
}

ByteArray packTriggerOrderAction(
    int asset,
    bool isBuy,
    const std::string& price,
    const std::string& size,
    bool reduceOnly,
    bool isMarket,
    const std::string& triggerPx,
    const std::string& tpsl,
    const std::string& cloid,
    const std::string& grouping
) {
    Packer packer;

    // Main map with 3 keys: "type", "orders", "grouping"
    // MUST be in this exact order to match Python SDK
    packer.packMapHeader(3);

    // 1. "type": "order" (FIRST!)
    packer.packString("type");
    packer.packString("order");

    // 2. "orders": [{order}] (SECOND!)
    packer.packString("orders");
    packer.packArrayHeader(1);

    // Pack the individual order inline
    int mapSize = cloid.empty() ? 6 : 7;
    packer.packMapHeader(mapSize);

    packer.packString("a");
    packer.packInt(asset);

    packer.packString("b");
    packer.packBool(isBuy);

    packer.packString("p");
    packer.packString(price);

    packer.packString("s");
    packer.packString(size);

    packer.packString("r");
    packer.packBool(reduceOnly);

    // "t": {"trigger":{"isMarket":bool,"triggerPx":"...","tpsl":"..."}}
    // Key order inside "trigger" must match Python SDK dict literal order
    packer.packString("t");
    packer.packMapHeader(1);
    packer.packString("trigger");
    packer.packMapHeader(3);
    packer.packString("isMarket");
    packer.packBool(isMarket);
    packer.packString("triggerPx");
    packer.packString(triggerPx);
    packer.packString("tpsl");
    packer.packString(tpsl);

    if (!cloid.empty()) {
        packer.packString("c");
        packer.packString(cloid);
    }

    // 3. "grouping": "na" (THIRD!)
    packer.packString("grouping");
    packer.packString(grouping);

    return packer.data();
}

ByteArray packCancelAction(int asset, uint64_t orderId) {
    Packer packer;

    // Main map with 2 keys: "type", "cancels"
    packer.packMapHeader(2);

    // 1. "type": "cancel"
    packer.packString("type");
    packer.packString("cancel");

    // 2. "cancels": [{"a":asset,"o":orderId}]
    packer.packString("cancels");
    packer.packArrayHeader(1);

    // Inner cancel object: 2 keys
    packer.packMapHeader(2);
    packer.packString("a");
    packer.packInt(asset);
    packer.packString("o");
    packer.packInt(static_cast<int64_t>(orderId));

    return packer.data();
}

} // namespace msgpack
} // namespace hl
