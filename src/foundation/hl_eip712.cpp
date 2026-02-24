//=============================================================================
// hl_eip712.cpp - EIP-712 Typed Data Encoding implementation
//=============================================================================
// LAYER: Foundation
// DEPENDENCIES: hl_crypto.h (keccak256), hl_msgpack.h (order encoding)
//=============================================================================

#include "hl_eip712.h"
#include "hl_crypto.h"
#include "hl_msgpack.h"

#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace hl {
namespace eip712 {

// =============================================================================
// Constructors
// =============================================================================

HyperliquidDomain::HyperliquidDomain()
    : name("Exchange")
    , version("1")
    , chainId(1337)
    , verifyingContract("0x0000000000000000000000000000000000000000")
{
}

OrderAction::OrderAction()
    : asset(0)
    , isBuy(true)
    , price("0")
    , size("0")
    , reduceOnly(false)
    , orderType("Ioc")
    , cloid("")
{
}

CancelAction::CancelAction()
    : asset(0)
    , orderId(0)
{
}

// =============================================================================
// Internal Helpers
// =============================================================================

namespace {

// Keccak-256 wrapper that returns ByteArray
ByteArray keccak(const ByteArray& data) {
    ByteArray hash(32);
    crypto::keccak256(data.data(), data.size(), hash.data());
    return hash;
}

// Keccak-256 for string
ByteArray keccak(const std::string& str) {
    ByteArray data(str.begin(), str.end());
    return keccak(data);
}

// Encode uint256 as 32 bytes (big-endian, left-padded with zeros)
ByteArray encodeUint256(uint64_t value) {
    ByteArray encoded(32, 0);
    for (int i = 31; i >= 24; --i) {
        encoded[i] = value & 0xFF;
        value >>= 8;
    }
    return encoded;
}

// Encode address as 32 bytes (left-padded with 12 zeros)
ByteArray encodeAddress(const std::string& address) {
    ByteArray addrBytes = hexToBytes(address);
    ByteArray encoded(32, 0);

    // Addresses are 20 bytes, left-pad with 12 zeros
    if (addrBytes.size() == 20) {
        memcpy(encoded.data() + 12, addrBytes.data(), 20);
    }

    return encoded;
}

// Normalize order type string to match API format
std::string normalizeOrderType(const std::string& tif) {
    if (tif == "IOC" || tif == "ioc") return "Ioc";
    if (tif == "GTC" || tif == "gtc") return "Gtc";
    if (tif == "ALO" || tif == "alo") return "Alo";
    return tif;  // Already normalized
}

} // anonymous namespace

// =============================================================================
// Utility Functions
// =============================================================================

std::string formatNumber(double value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.8f", value);

    std::string s(buf);

    // Remove trailing zeros after decimal point
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t end = s.find_last_not_of('0');
        if (end != std::string::npos) {
            s = s.substr(0, end + 1);
        }
        // Remove trailing decimal point
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }

    return s;
}

ByteArray hexToBytes(const std::string& hex) {
    ByteArray bytes;
    std::string clean = hex;

    // Remove 0x prefix if present
    if (clean.size() >= 2 && clean[0] == '0' && (clean[1] == 'x' || clean[1] == 'X')) {
        clean = clean.substr(2);
    }

    // Pad to even length
    if (clean.length() % 2 != 0) {
        clean = "0" + clean;
    }

    for (size_t i = 0; i < clean.length(); i += 2) {
        std::string byteString = clean.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtoul(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }

    return bytes;
}

std::string bytesToHex(const ByteArray& bytes) {
    std::ostringstream oss;
    oss << "0x";
    for (uint8_t byte : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

uint64_t getCurrentTimestampMs() {
#ifdef _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert to Unix time (milliseconds since 1970)
    uint64_t time64 = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    time64 -= 116444736000000000ULL;  // Convert Windows epoch to Unix epoch
    return time64 / 10000;  // Convert 100ns intervals to milliseconds
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
#endif
}

// =============================================================================
// EIP-712 Encoding Functions
// =============================================================================

ByteArray encodeDomainSeparator(const HyperliquidDomain& domain) {
    // EIP712Domain type hash
    const char* domainTypeStr =
        "EIP712Domain(string name,string version,uint256 chainId,address verifyingContract)";
    ByteArray typeHash = keccak(domainTypeStr);

    // Hash domain parameters
    ByteArray nameHash = keccak(domain.name);
    ByteArray versionHash = keccak(domain.version);
    ByteArray chainIdEncoded = encodeUint256(domain.chainId);
    ByteArray contractEncoded = encodeAddress(domain.verifyingContract);

    // Concatenate: typeHash + nameHash + versionHash + chainId + verifyingContract
    ByteArray encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), nameHash.begin(), nameHash.end());
    encoded.insert(encoded.end(), versionHash.begin(), versionHash.end());
    encoded.insert(encoded.end(), chainIdEncoded.begin(), chainIdEncoded.end());
    encoded.insert(encoded.end(), contractEncoded.begin(), contractEncoded.end());

    return keccak(encoded);
}

ByteArray encodeAgentType(const std::string& source, const ByteArray& connectionId) {
    // Agent type hash
    const char* agentTypeStr = "Agent(string source,bytes32 connectionId)";
    ByteArray typeHash = keccak(agentTypeStr);

    // Hash source string
    ByteArray sourceHash = keccak(source);

    // connectionId should be 32 bytes
    ByteArray connectionIdPadded(32, 0);
    if (connectionId.size() >= 32) {
        memcpy(connectionIdPadded.data(), connectionId.data(), 32);
    } else if (!connectionId.empty()) {
        memcpy(connectionIdPadded.data(), connectionId.data(), connectionId.size());
    }

    // Concatenate: typeHash + sourceHash + connectionId
    ByteArray encoded;
    encoded.insert(encoded.end(), typeHash.begin(), typeHash.end());
    encoded.insert(encoded.end(), sourceHash.begin(), sourceHash.end());
    encoded.insert(encoded.end(), connectionIdPadded.begin(), connectionIdPadded.end());

    return keccak(encoded);
}

ByteArray hashOrderAction(const OrderAction& action, uint64_t nonce,
                          const std::string& vaultAddress) {
    // Use MessagePack encoding (matches Python SDK implementation)
    ByteArray actionData;

    if (action.isTrigger) {
        // Trigger order: different msgpack encoding [OPM-77]
        actionData = msgpack::packTriggerOrderAction(
            action.asset,
            action.isBuy,
            action.price,
            action.size,
            action.reduceOnly,
            action.triggerIsMarket,
            action.triggerPx,
            action.tpsl,
            action.cloid
        );
    } else {
        // Limit order: existing path
        std::string tif = normalizeOrderType(action.orderType);
        actionData = msgpack::packOrderAction(
            action.asset,
            action.isBuy,
            action.price,
            action.size,
            action.reduceOnly,
            tif,
            action.cloid
        );
    }

    // Append nonce (8 bytes, big-endian)
    for (int i = 7; i >= 0; --i) {
        actionData.push_back((nonce >> (i * 8)) & 0xFF);
    }

    // Append vault address flag and bytes (matches Python SDK)
    // Python: if vault_address is None: data += b"\x00"
    //         else: data += b"\x01" + address_to_bytes(vault_address)
    if (vaultAddress.empty() ||
        vaultAddress == "0x0000000000000000000000000000000000000000") {
        actionData.push_back(0x00);  // No vault flag
    } else {
        actionData.push_back(0x01);  // Vault present flag
        ByteArray vaultBytes = hexToBytes(vaultAddress);
        if (vaultBytes.size() == 20) {
            actionData.insert(actionData.end(), vaultBytes.begin(), vaultBytes.end());
        }
    }

    return keccak(actionData);
}

ByteArray hashCancelAction(const CancelAction& action, uint64_t nonce,
                           const std::string& vaultAddress) {
    // Use MessagePack encoding (matches Python SDK implementation)
    // Full action: {"type":"cancel","cancels":[{"a":asset,"o":orderId}]}
    ByteArray actionData = msgpack::packCancelAction(action.asset, action.orderId);

    // Append nonce (8 bytes, big-endian)
    for (int i = 7; i >= 0; --i) {
        actionData.push_back((nonce >> (i * 8)) & 0xFF);
    }

    // Append vault address flag and bytes (matches Python SDK)
    if (vaultAddress.empty() ||
        vaultAddress == "0x0000000000000000000000000000000000000000") {
        actionData.push_back(0x00);  // No vault flag
    } else {
        actionData.push_back(0x01);  // Vault present flag
        ByteArray vaultBytes = hexToBytes(vaultAddress);
        if (vaultBytes.size() == 20) {
            actionData.insert(actionData.end(), vaultBytes.begin(), vaultBytes.end());
        }
    }

    return keccak(actionData);
}

ByteArray generateMessageHash(const ByteArray& domainSeparator,
                              const ByteArray& structHash) {
    // EIP-712 final message: "\x19\x01" + domainSeparator + structHash
    ByteArray message;
    message.push_back(0x19);
    message.push_back(0x01);
    message.insert(message.end(), domainSeparator.begin(), domainSeparator.end());
    message.insert(message.end(), structHash.begin(), structHash.end());

    return keccak(message);
}

// =============================================================================
// High-Level Convenience Functions
// =============================================================================

ByteArray hashOrderForSigning(const OrderAction& action, bool isMainnet,
                              uint64_t nonce,
                              const std::string& vaultAddress) {
    if (nonce == 0) {
        nonce = getCurrentTimestampMs();
    }

    // Create domain
    HyperliquidDomain domain;
    ByteArray domainSep = encodeDomainSeparator(domain);

    // Hash action -> connectionId
    ByteArray connectionId = hashOrderAction(action, nonce, vaultAddress);

    // Source: "a" for mainnet, "b" for testnet (per HL Python SDK)
    std::string source = isMainnet ? "a" : "b";
    ByteArray structHash = encodeAgentType(source, connectionId);

    // Generate final message hash
    return generateMessageHash(domainSep, structHash);
}

ByteArray hashCancelForSigning(const CancelAction& action, bool isMainnet,
                               uint64_t nonce,
                               const std::string& vaultAddress) {
    if (nonce == 0) {
        nonce = getCurrentTimestampMs();
    }

    HyperliquidDomain domain;
    ByteArray domainSep = encodeDomainSeparator(domain);

    // Hash action -> connectionId
    ByteArray connectionId = hashCancelAction(action, nonce, vaultAddress);

    std::string source = isMainnet ? "a" : "b";
    ByteArray structHash = encodeAgentType(source, connectionId);

    return generateMessageHash(domainSep, structHash);
}

} // namespace eip712
} // namespace hl
