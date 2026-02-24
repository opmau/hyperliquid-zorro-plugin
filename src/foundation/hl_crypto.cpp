//=============================================================================
// hl_crypto.cpp - Cryptographic operations implementation
//=============================================================================
// LAYER: Foundation
// DEPENDENCIES: secp256k1, keccak256
//=============================================================================

#include "hl_crypto.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

// secp256k1 library (bundled header-only version)
// Enable recovery module for Ethereum-style signatures
#define ENABLE_MODULE_RECOVERY
#define BT_SECP256K1_IMPLEMENTATION
#include "../../Source/HyperliquidPlugin/crypto/bt_secp256k1.h"

// Keccak-256 for Ethereum address derivation
extern "C" {
#include "../../Source/HyperliquidPlugin/crypto/keccak256.h"
}

namespace hl {
namespace crypto {

// =============================================================================
// STATIC STATE
// =============================================================================

static secp256k1_context* s_ctx = nullptr;

// =============================================================================
// INITIALIZATION
// =============================================================================

bool init() {
    if (s_ctx) {
        return true;  // Already initialized
    }

    s_ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    return s_ctx != nullptr;
}

void cleanup() {
    if (s_ctx) {
        secp256k1_context_destroy(s_ctx);
        s_ctx = nullptr;
    }
}

bool isInitialized() {
    return s_ctx != nullptr;
}

// =============================================================================
// INTERNAL HELPERS
// =============================================================================

// Parse hex private key into 32-byte array
// Returns true on success
static bool parsePrivateKey(const char* hexKey, uint8_t* out) {
    const char* p = hexKey;

    // Skip "0x" prefix if present
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }

    // Check length (should be 64 hex chars = 32 bytes)
    size_t len = strlen(p);
    if (len != 64) {
        return false;
    }

    // Parse hex bytes
    for (int i = 0; i < 32; i++) {
        char byte[3] = {p[i * 2], p[i * 2 + 1], 0};
        char* end;
        unsigned long val = strtoul(byte, &end, 16);
        if (*end != '\0') {
            return false;  // Invalid hex character
        }
        out[i] = static_cast<uint8_t>(val);
    }

    return true;
}

// =============================================================================
// ADDRESS DERIVATION
// =============================================================================

bool deriveAddress(const char* privateKeyHex, char* addressOut, size_t addressOutSize) {
    if (!s_ctx) {
        return false;
    }

    if (!privateKeyHex || !addressOut || addressOutSize < 43) {
        return false;
    }

    // Parse private key
    uint8_t privkey[32];
    if (!parsePrivateKey(privateKeyHex, privkey)) {
        return false;
    }

    // Verify private key is valid for secp256k1
    if (!secp256k1_ec_seckey_verify(s_ctx, privkey)) {
        return false;
    }

    // Derive public key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(s_ctx, &pubkey, privkey)) {
        return false;
    }

    // Serialize to uncompressed format (65 bytes: 0x04 prefix + 64 bytes)
    uint8_t pubkey_serialized[65];
    size_t pubkey_len = 65;
    secp256k1_ec_pubkey_serialize(s_ctx, pubkey_serialized, &pubkey_len,
                                   &pubkey, SECP256K1_EC_UNCOMPRESSED);

    // Ethereum address = keccak256(pubkey[1:65])[-20:]
    // Hash the 64-byte public key (skip the 0x04 prefix)
    uint8_t hash[32];
    ::keccak256(pubkey_serialized + 1, 64, hash);

    // Format address: take last 20 bytes of hash
    snprintf(addressOut, addressOutSize, "0x");
    for (int i = 12; i < 32; i++) {
        snprintf(addressOut + 2 + (i - 12) * 2, 3, "%02x", hash[i]);
    }

    // Clear sensitive data
    memset(privkey, 0, sizeof(privkey));

    return true;
}

// =============================================================================
// HASH SIGNING
// =============================================================================

std::string Signature::toJson() const {
    char buf[256];
    snprintf(buf, sizeof(buf), "{\"r\":\"%s\",\"s\":\"%s\",\"v\":%d}", r, s, v);
    return std::string(buf);
}

bool signHash(const uint8_t* hash, const char* privateKeyHex, Signature& sigOut) {
    if (!s_ctx) {
        return false;
    }

    if (!hash || !privateKeyHex) {
        return false;
    }

    // Parse private key
    uint8_t privkey[32];
    if (!parsePrivateKey(privateKeyHex, privkey)) {
        return false;
    }

    // Verify private key
    if (!secp256k1_ec_seckey_verify(s_ctx, privkey)) {
        memset(privkey, 0, sizeof(privkey));
        return false;
    }

    // Sign with recoverable signature
    secp256k1_ecdsa_recoverable_signature sig;
    if (!secp256k1_ecdsa_sign_recoverable(s_ctx, &sig, hash, privkey, nullptr, nullptr)) {
        memset(privkey, 0, sizeof(privkey));
        return false;
    }

    // Serialize to r, s, and recovery ID
    uint8_t serialized[64];  // r (32 bytes) + s (32 bytes)
    int recid = 0;
    secp256k1_ecdsa_recoverable_signature_serialize_compact(s_ctx, serialized, &recid, &sig);

    // Format r as hex string
    snprintf(sigOut.r, sizeof(sigOut.r), "0x");
    for (int i = 0; i < 32; i++) {
        snprintf(sigOut.r + 2 + i * 2, 3, "%02x", serialized[i]);
    }

    // Format s as hex string
    snprintf(sigOut.s, sizeof(sigOut.s), "0x");
    for (int i = 0; i < 32; i++) {
        snprintf(sigOut.s + 2 + i * 2, 3, "%02x", serialized[32 + i]);
    }

    // Ethereum v = recid + 27
    sigOut.v = recid + 27;

    // Clear sensitive data
    memset(privkey, 0, sizeof(privkey));

    return true;
}

bool signHashToJson(const uint8_t* hash, const char* privateKeyHex,
                    char* jsonOut, size_t jsonOutSize) {
    Signature sig;
    if (!signHash(hash, privateKeyHex, sig)) {
        return false;
    }

    std::string json = sig.toJson();
    if (json.length() >= jsonOutSize) {
        return false;
    }

    strncpy(jsonOut, json.c_str(), jsonOutSize - 1);
    jsonOut[jsonOutSize - 1] = '\0';
    return true;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

size_t hexToBytes(const char* hex, uint8_t* out, size_t outSize) {
    if (!hex || !out) {
        return 0;
    }

    const char* p = hex;

    // Skip "0x" prefix if present
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }

    size_t len = strlen(p);
    size_t byteLen = len / 2;

    if (byteLen > outSize) {
        return 0;
    }

    for (size_t i = 0; i < byteLen; i++) {
        char byte[3] = {p[i * 2], p[i * 2 + 1], 0};
        char* end;
        unsigned long val = strtoul(byte, &end, 16);
        if (*end != '\0') {
            return 0;
        }
        out[i] = static_cast<uint8_t>(val);
    }

    return byteLen;
}

void bytesToHex(const uint8_t* data, size_t len, char* out, size_t outSize) {
    if (!data || !out || outSize < 3) {
        if (out && outSize > 0) out[0] = '\0';
        return;
    }

    size_t needed = len * 2 + 3;  // "0x" + hex + null
    if (outSize < needed) {
        out[0] = '\0';
        return;
    }

    snprintf(out, outSize, "0x");
    for (size_t i = 0; i < len; i++) {
        snprintf(out + 2 + i * 2, 3, "%02x", data[i]);
    }
}

void keccak256(const uint8_t* data, size_t len, uint8_t* out) {
    ::keccak256(data, len, out);
}

} // namespace crypto
} // namespace hl
