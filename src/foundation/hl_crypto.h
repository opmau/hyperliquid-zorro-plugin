//=============================================================================
// hl_crypto.h - Cryptographic operations for Hyperliquid plugin
//=============================================================================
// LAYER: Foundation
// DEPENDENCIES: hl_types.h (ByteArray typedef from eip712.h)
// THREAD SAFETY: init/cleanup must be called from single thread;
//                other functions are thread-safe after init
//
// This module provides:
//   - secp256k1 context management (ECDSA signing)
//   - Ethereum address derivation from private key
//   - Hash signing with recovery (Ethereum-style)
//   - Integration with EIP-712 typed data encoding
//=============================================================================

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace hl {
namespace crypto {

// =============================================================================
// INITIALIZATION (call once at plugin startup/shutdown)
// =============================================================================

/// Initialize the cryptographic context (secp256k1)
/// Must be called before any signing operations
/// Returns true on success
bool init();

/// Clean up cryptographic context
/// Call during plugin shutdown
void cleanup();

/// Check if crypto is initialized
bool isInitialized();

// =============================================================================
// ADDRESS DERIVATION
// =============================================================================

/// Derive Ethereum address from a private key
/// @param privateKeyHex  Private key in hex format (with or without 0x prefix)
/// @param addressOut     Buffer to receive the address (minimum 43 bytes for "0x" + 40 hex + null)
/// @param addressOutSize Size of addressOut buffer
/// @return true on success, false on error (invalid key, etc.)
///
/// Example:
///   char addr[64];
///   if (deriveAddress("0x1234...abcd", addr, sizeof(addr))) {
///       printf("Address: %s\n", addr);  // "0xabc123..."
///   }
bool deriveAddress(const char* privateKeyHex, char* addressOut, size_t addressOutSize);

// =============================================================================
// HASH SIGNING (for EIP-712 messages)
// =============================================================================

/// Signature components (Ethereum-style r, s, v)
struct Signature {
    char r[67];     // "0x" + 64 hex digits
    char s[67];     // "0x" + 64 hex digits
    int v;          // Recovery ID (27 or 28)

    /// Build JSON representation: {"r":"0x...","s":"0x...","v":27}
    std::string toJson() const;
};

/// Sign a 32-byte hash using the private key
/// @param hash           32-byte hash to sign (typically EIP-712 message hash)
/// @param privateKeyHex  Private key in hex format
/// @param sigOut         Receives signature components
/// @return true on success
bool signHash(const uint8_t* hash, const char* privateKeyHex, Signature& sigOut);

/// Sign a 32-byte hash and return as JSON string
/// Convenience function combining signHash + Signature::toJson()
/// @param hash           32-byte hash to sign
/// @param privateKeyHex  Private key in hex format
/// @param jsonOut        Buffer to receive JSON signature
/// @param jsonOutSize    Size of jsonOut buffer
/// @return true on success
bool signHashToJson(const uint8_t* hash, const char* privateKeyHex,
                    char* jsonOut, size_t jsonOutSize);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/// Convert hex string to bytes
/// @param hex  Hex string (with or without 0x prefix)
/// @param out  Output buffer
/// @param outSize Size of output buffer
/// @return Number of bytes written, or 0 on error
size_t hexToBytes(const char* hex, uint8_t* out, size_t outSize);

/// Convert bytes to hex string
/// @param data  Input bytes
/// @param len   Number of bytes
/// @param out   Output buffer (must be at least len*2 + 3 for "0x" prefix + null)
/// @param outSize Size of output buffer
void bytesToHex(const uint8_t* data, size_t len, char* out, size_t outSize);

/// Compute Keccak-256 hash (Ethereum-style, not SHA3)
/// @param data   Input data
/// @param len    Input length
/// @param out    Output buffer (32 bytes)
void keccak256(const uint8_t* data, size_t len, uint8_t* out);

} // namespace crypto
} // namespace hl
