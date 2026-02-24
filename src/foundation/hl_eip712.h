//=============================================================================
// hl_eip712.h - EIP-712 Typed Data Encoding for Hyperliquid
//=============================================================================
// LAYER: Foundation
// DEPENDENCIES: hl_crypto.h (keccak256), hl_msgpack.h (order encoding)
// THREAD SAFETY: All functions are thread-safe (no shared state)
//
// EIP-712 is the Ethereum standard for typed structured data hashing and signing.
// Hyperliquid uses this for cryptographically signing orders and cancellations.
//
// Flow: OrderAction -> MsgPack encode -> Keccak256 -> EIP-712 wrap -> Sign
//=============================================================================

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace hl {
namespace eip712 {

// Convenience typedef for byte arrays
using ByteArray = std::vector<uint8_t>;

// =============================================================================
// Hyperliquid Domain Configuration
// =============================================================================

/// Hyperliquid L1 EIP-712 domain separator parameters
/// These are fixed values defined by the Hyperliquid protocol
struct HyperliquidDomain {
    std::string name;               // "Exchange"
    std::string version;            // "1"
    uint64_t chainId;               // 1337
    std::string verifyingContract;  // "0x0000000000000000000000000000000000000000"

    /// Constructor with Hyperliquid default values
    HyperliquidDomain();
};

// =============================================================================
// Action Structures
// =============================================================================

/// Order action for EIP-712 encoding
/// Maps to the Hyperliquid order wire format
struct OrderAction {
    int asset;              // a: asset index
    bool isBuy;             // b: buy or sell
    std::string price;      // p: limit price (formatted, no trailing zeros)
    std::string size;       // s: order size (formatted, no trailing zeros)
    bool reduceOnly;        // r: reduce-only flag
    std::string orderType;  // t: "Ioc", "Gtc", "Alo" (limit orders only)
    std::string cloid;      // c: client order ID (optional)

    // Trigger order fields [OPM-77]
    bool isTrigger = false;         // true = trigger order, false = limit order
    bool triggerIsMarket = true;    // true = market execution when triggered
    std::string triggerPx;          // Trigger activation price (formatted)
    std::string tpsl;               // "sl" for stop-loss, "tp" for take-profit

    OrderAction();
};

/// Cancel action for EIP-712 encoding
struct CancelAction {
    int asset;              // Asset to cancel
    uint64_t orderId;       // Order ID (oid) to cancel — numeric per HL API

    CancelAction();
};

// =============================================================================
// Core EIP-712 Functions
// =============================================================================

/// Encode EIP-712 domain separator hash
/// @param domain  Domain parameters (use HyperliquidDomain defaults)
/// @return 32-byte domain separator hash
ByteArray encodeDomainSeparator(const HyperliquidDomain& domain);

/// Encode Agent struct hash (Hyperliquid L1 specific)
/// @param source        Source identifier ("a" for mainnet, "b" for testnet)
/// @param connectionId  32-byte action hash (from hashOrderAction)
/// @return 32-byte struct hash
ByteArray encodeAgentType(const std::string& source, const ByteArray& connectionId);

/// Hash an order action for EIP-712
/// @param action       Order details
/// @param nonce        Timestamp in milliseconds (0 = use current time)
/// @param vaultAddress Optional vault address (empty string if not using vault)
/// @return 32-byte action hash (used as connectionId)
ByteArray hashOrderAction(const OrderAction& action, uint64_t nonce,
                          const std::string& vaultAddress = "");

/// Hash a cancel action for EIP-712
/// @param action       Cancel details
/// @param nonce        Timestamp in milliseconds (0 = use current time)
/// @param vaultAddress Optional vault address
/// @return 32-byte action hash
ByteArray hashCancelAction(const CancelAction& action, uint64_t nonce,
                           const std::string& vaultAddress = "");

/// Generate final EIP-712 message hash
/// Combines domain separator and struct hash per EIP-712 spec:
/// keccak256("\x19\x01" + domainSeparator + structHash)
///
/// @param domainSeparator  32 bytes from encodeDomainSeparator
/// @param structHash       32 bytes from encodeAgentType
/// @return 32-byte message hash ready for secp256k1 signing
ByteArray generateMessageHash(const ByteArray& domainSeparator,
                              const ByteArray& structHash);

// =============================================================================
// High-Level Convenience Functions
// =============================================================================

/// Hash complete order for signing (combines all steps)
/// @param action       Order action
/// @param isMainnet    true = mainnet (source "a"), false = testnet (source "b")
/// @param nonce        Timestamp in ms (0 = use current time)
/// @param vaultAddress Optional vault address
/// @return 32-byte hash ready for hl::crypto::signHash()
ByteArray hashOrderForSigning(const OrderAction& action,
                              bool isMainnet,
                              uint64_t nonce = 0,
                              const std::string& vaultAddress = "");

/// Hash complete cancel for signing (combines all steps)
/// @param action       Cancel action
/// @param isMainnet    true = mainnet (source "a"), false = testnet (source "b")
/// @param nonce        Timestamp in ms (0 = use current time)
/// @param vaultAddress Optional vault address
/// @return 32-byte hash ready for hl::crypto::signHash()
ByteArray hashCancelForSigning(const CancelAction& action,
                               bool isMainnet,
                               uint64_t nonce = 0,
                               const std::string& vaultAddress = "");

// =============================================================================
// Utility Functions
// =============================================================================

/// Format a number according to Hyperliquid rules:
/// - Maximum 8 decimal places
/// - No trailing zeros
/// - No trailing decimal point
/// @param value  Number to format
/// @return Formatted string
std::string formatNumber(double value);

/// Get current timestamp in milliseconds (for nonce generation)
uint64_t getCurrentTimestampMs();

/// Convert hex string to byte array
/// "0x1234..." -> {0x12, 0x34, ...}
ByteArray hexToBytes(const std::string& hex);

/// Convert byte array to hex string
/// {0x12, 0x34, ...} -> "0x1234..."
std::string bytesToHex(const ByteArray& bytes);

} // namespace eip712
} // namespace hl
