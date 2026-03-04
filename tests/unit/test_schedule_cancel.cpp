//=============================================================================
// test_schedule_cancel.cpp - scheduleCancel msgpack + EIP-712 signing tests
//=============================================================================
// PREVENTS BUGS: [OPM-83]
//   - Incorrect msgpack field ordering causing signature mismatch
//   - Wrong action type string causing exchange rejection
//   - Missing/extra "time" field in with-time vs without-time variants
//
// VERIFIED AGAINST:
//   - Python SDK tests/signing_test.py:248-273 (test_schedule_cancel_action)
//   - Python SDK hyperliquid/exchange.py:341-367 (schedule_cancel method)
//   - Hyperliquid API docs 06-exchange-endpoint.md:294-354
//=============================================================================

#include "../test_framework.h"
#include "hl_msgpack.h"
#include "hl_eip712.h"
#include "hl_crypto.h"
#include <cstdio>

using namespace hl::test;
using namespace hl::msgpack;

//=============================================================================
// HELPER: Print bytes as hex for debugging
//=============================================================================

static void printHex(const char* label, const hl::eip712::ByteArray& data) {
    printf("  %s (%zu bytes): ", label, data.size());
    for (size_t i = 0; i < data.size(); i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

//=============================================================================
// HELPER: Find a substring in byte array
//=============================================================================

static bool containsString(const ByteArray& data, const char* str) {
    size_t len = strlen(str);
    if (data.size() < len) return false;
    for (size_t i = 0; i <= data.size() - len; i++) {
        if (memcmp(&data[i], str, len) == 0) return true;
    }
    return false;
}

//=============================================================================
// MSGPACK TESTS: packScheduleCancelAction
//=============================================================================

// Without time: map with 1 key
TEST_CASE(schedule_cancel_no_time_nonempty) {
    ByteArray result = packScheduleCancelAction(0);
    ASSERT_TRUE(result.size() > 0);
}

TEST_CASE(schedule_cancel_no_time_contains_type) {
    ByteArray result = packScheduleCancelAction(0);
    ASSERT_TRUE(containsString(result, "scheduleCancel"));
}

TEST_CASE(schedule_cancel_no_time_map_size_1) {
    ByteArray result = packScheduleCancelAction(0);
    // fixmap with 1 entry = 0x81
    ASSERT_EQ(result[0], (uint8_t)0x81);
}

TEST_CASE(schedule_cancel_no_time_type_key_first) {
    ByteArray result = packScheduleCancelAction(0);
    // After 0x81 (map1), first key = "type" (fixstr len 4 = 0xa4 + "type")
    ASSERT_TRUE(result.size() > 5);
    ASSERT_EQ(result[1], (uint8_t)0xa4);  // fixstr len 4
    ASSERT_EQ(result[2], (uint8_t)'t');
    ASSERT_EQ(result[3], (uint8_t)'y');
    ASSERT_EQ(result[4], (uint8_t)'p');
    ASSERT_EQ(result[5], (uint8_t)'e');
}

TEST_CASE(schedule_cancel_no_time_no_time_key) {
    ByteArray result = packScheduleCancelAction(0);
    // Should NOT contain the "time" key
    ASSERT_FALSE(containsString(result, "time"));
    // But DOES contain "type" which has "t" in it — so we check
    // that "time" (4 chars) doesn't appear as a separate key.
    // The string "scheduleCancel" doesn't contain "time", and
    // "type" != "time", so this is a valid check.
}

// With time: map with 2 keys
TEST_CASE(schedule_cancel_with_time_nonempty) {
    ByteArray result = packScheduleCancelAction(123456789);
    ASSERT_TRUE(result.size() > 0);
}

TEST_CASE(schedule_cancel_with_time_map_size_2) {
    ByteArray result = packScheduleCancelAction(123456789);
    // fixmap with 2 entries = 0x82
    ASSERT_EQ(result[0], (uint8_t)0x82);
}

TEST_CASE(schedule_cancel_with_time_contains_type) {
    ByteArray result = packScheduleCancelAction(123456789);
    ASSERT_TRUE(containsString(result, "scheduleCancel"));
}

TEST_CASE(schedule_cancel_with_time_contains_time_key) {
    // With time=123456789, the "time" key should appear after "type"
    ByteArray result = packScheduleCancelAction(123456789);
    // Find position of "type" and "time" — "type" must come first
    size_t typePos = 0, timePos = 0;
    bool foundType = false, foundTime = false;
    for (size_t i = 0; i <= result.size() - 4; i++) {
        if (memcmp(&result[i], "type", 4) == 0 && !foundType) {
            typePos = i;
            foundType = true;
        }
        if (memcmp(&result[i], "time", 4) == 0 && i > typePos && foundType) {
            timePos = i;
            foundTime = true;
        }
    }
    ASSERT_TRUE(foundType);
    ASSERT_TRUE(foundTime);
    ASSERT_TRUE(timePos > typePos);
}

TEST_CASE(schedule_cancel_different_times_differ) {
    ByteArray a = packScheduleCancelAction(100000);
    ByteArray b = packScheduleCancelAction(200000);
    ASSERT_TRUE(a != b);
}

TEST_CASE(schedule_cancel_with_vs_without_time_differ) {
    ByteArray a = packScheduleCancelAction(0);
    ByteArray b = packScheduleCancelAction(123456789);
    ASSERT_TRUE(a != b);
    // Without-time should be shorter (fewer fields)
    ASSERT_TRUE(a.size() < b.size());
}

//=============================================================================
// EIP-712 TESTS: hashScheduleCancelForSigning
//=============================================================================

TEST_CASE(schedule_cancel_hash_nonempty) {
    hl::eip712::ByteArray hash = hl::eip712::hashScheduleCancelForSigning(
        0, true, 0);
    ASSERT_EQ(hash.size(), (size_t)32);
}

TEST_CASE(schedule_cancel_hash_deterministic) {
    uint64_t nonce = 1700000000000ULL;
    hl::eip712::ByteArray h1 = hl::eip712::hashScheduleCancelForSigning(0, true, nonce);
    hl::eip712::ByteArray h2 = hl::eip712::hashScheduleCancelForSigning(0, true, nonce);
    ASSERT_TRUE(h1 == h2);
}

TEST_CASE(schedule_cancel_hash_mainnet_vs_testnet) {
    uint64_t nonce = 1700000000000ULL;
    hl::eip712::ByteArray mainnet = hl::eip712::hashScheduleCancelForSigning(0, true, nonce);
    hl::eip712::ByteArray testnet = hl::eip712::hashScheduleCancelForSigning(0, false, nonce);
    ASSERT_EQ(mainnet.size(), (size_t)32);
    ASSERT_EQ(testnet.size(), (size_t)32);
    ASSERT_TRUE(mainnet != testnet);
}

TEST_CASE(schedule_cancel_hash_with_vs_without_time) {
    uint64_t nonce = 1700000000000ULL;
    hl::eip712::ByteArray noTime = hl::eip712::hashScheduleCancelForSigning(0, true, nonce);
    hl::eip712::ByteArray withTime = hl::eip712::hashScheduleCancelForSigning(123456789, true, nonce);
    ASSERT_TRUE(noTime != withTime);
}

//=============================================================================
// SIGNING VECTOR TESTS (verified against Python SDK signing_test.py:248-273)
//=============================================================================
// Private key: 0x0123456789012345678901234567890123456789012345678901234567890123
// These tests verify end-to-end: msgpack → keccak → EIP-712 → secp256k1 sign
//
// NOTE: hashScheduleCancelForSigning() auto-generates a timestamp when nonce=0.
// Python SDK test vectors use literal nonce=0. We call lower-level functions
// directly to bypass the auto-generation.

/// Helper: compute the full EIP-712 message hash with literal nonce=0
/// (bypasses hashScheduleCancelForSigning's nonce auto-generation)
static hl::eip712::ByteArray hashForTestVector(uint64_t time, bool isMainnet) {
    hl::eip712::HyperliquidDomain domain;
    hl::eip712::ByteArray domainSep = hl::eip712::encodeDomainSeparator(domain);
    hl::eip712::ByteArray connectionId = hl::eip712::hashScheduleCancelAction(time, 0, "");
    std::string source = isMainnet ? "a" : "b";
    hl::eip712::ByteArray structHash = hl::eip712::encodeAgentType(source, connectionId);
    return hl::eip712::generateMessageHash(domainSep, structHash);
}

TEST_CASE(schedule_cancel_sign_no_time_mainnet) {
    // Python SDK: action={"type":"scheduleCancel"}, nonce=0, mainnet
    // Expected: r=0x6cdf...5df9, s=0x6557...b2a0, v=27
    const char* privKey = "0x0123456789012345678901234567890123456789012345678901234567890123";

    hl::eip712::ByteArray msgHash = hashForTestVector(0, true);
    ASSERT_EQ(msgHash.size(), (size_t)32);

    hl::crypto::Signature sig;
    bool ok = hl::crypto::signHash(msgHash.data(), privKey, sig);
    ASSERT_TRUE(ok);

    std::string sigJson = sig.toJson();
    printf("  Mainnet no-time sig: %s\n", sigJson.c_str());

    // Verify r
    ASSERT_TRUE(sigJson.find("0x6cdfb286702f5917e76cd9b3b8bf678fcc49aec194c02a73e6d4f16891195df9") != std::string::npos);
    // Verify s
    ASSERT_TRUE(sigJson.find("0x6557ac307fa05d25b8d61f21fb8a938e703b3d9bf575f6717ba21ec61261b2a0") != std::string::npos);
    // Verify v=27
    ASSERT_TRUE(sigJson.find("\"v\":27") != std::string::npos);
}

TEST_CASE(schedule_cancel_sign_no_time_testnet) {
    // Python SDK: action={"type":"scheduleCancel"}, nonce=0, testnet
    // Expected: r=0xc75b...aeed, s=0x342f...33df, v=28
    const char* privKey = "0x0123456789012345678901234567890123456789012345678901234567890123";

    hl::eip712::ByteArray msgHash = hashForTestVector(0, false);

    hl::crypto::Signature sig;
    bool ok = hl::crypto::signHash(msgHash.data(), privKey, sig);
    ASSERT_TRUE(ok);

    std::string sigJson = sig.toJson();
    printf("  Testnet no-time sig: %s\n", sigJson.c_str());

    ASSERT_TRUE(sigJson.find("0xc75bb195c3f6a4e06b7d395acc20bbb224f6d23ccff7c6a26d327304e6efaeed") != std::string::npos);
    ASSERT_TRUE(sigJson.find("0x342f8ede109a29f2c0723bd5efb9e9100e3bbb493f8fb5164ee3d385908233df") != std::string::npos);
    ASSERT_TRUE(sigJson.find("\"v\":28") != std::string::npos);
}

TEST_CASE(schedule_cancel_sign_with_time_mainnet) {
    // Python SDK: action={"type":"scheduleCancel","time":123456789}, nonce=0, mainnet
    // Expected: r=0x609c...9d55, s=0x16c6...758b, v=28
    const char* privKey = "0x0123456789012345678901234567890123456789012345678901234567890123";

    hl::eip712::ByteArray msgHash = hashForTestVector(123456789, true);

    hl::crypto::Signature sig;
    bool ok = hl::crypto::signHash(msgHash.data(), privKey, sig);
    ASSERT_TRUE(ok);

    std::string sigJson = sig.toJson();
    printf("  Mainnet with-time sig: %s\n", sigJson.c_str());

    ASSERT_TRUE(sigJson.find("0x609cb20c737945d070716dcc696ba030e9976fcf5edad87afa7d877493109d55") != std::string::npos);
    ASSERT_TRUE(sigJson.find("0x16c685d63b5c7a04512d73f183b3d7a00da5406ff1f8aad33f8ae2163bab758b") != std::string::npos);
    ASSERT_TRUE(sigJson.find("\"v\":28") != std::string::npos);
}

TEST_CASE(schedule_cancel_sign_with_time_testnet) {
    // Python SDK: action={"type":"scheduleCancel","time":123456789}, nonce=0, testnet
    // Expected: r=0x4e4f...c5bc, s=0x706c...a678, v=27
    const char* privKey = "0x0123456789012345678901234567890123456789012345678901234567890123";

    hl::eip712::ByteArray msgHash = hashForTestVector(123456789, false);

    hl::crypto::Signature sig;
    bool ok = hl::crypto::signHash(msgHash.data(), privKey, sig);
    ASSERT_TRUE(ok);

    std::string sigJson = sig.toJson();
    printf("  Testnet with-time sig: %s\n", sigJson.c_str());

    ASSERT_TRUE(sigJson.find("0x4e4f2dbd4107c69783e251b7e1057d9f2b9d11cee213441ccfa2be63516dc5bc") != std::string::npos);
    ASSERT_TRUE(sigJson.find("0x706c656b23428c8ba356d68db207e11139ede1670481a9e01ae2dfcdb0e1a678") != std::string::npos);
    ASSERT_TRUE(sigJson.find("\"v\":27") != std::string::npos);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== scheduleCancel Tests [OPM-83] ===\n\n");

    // Initialize secp256k1 context (required before signing)
    if (!hl::crypto::init()) {
        printf("FATAL: crypto::init() failed\n");
        return 1;
    }

    // Msgpack encoding tests
    RUN_TEST(schedule_cancel_no_time_nonempty);
    RUN_TEST(schedule_cancel_no_time_contains_type);
    RUN_TEST(schedule_cancel_no_time_map_size_1);
    RUN_TEST(schedule_cancel_no_time_type_key_first);
    RUN_TEST(schedule_cancel_no_time_no_time_key);
    RUN_TEST(schedule_cancel_with_time_nonempty);
    RUN_TEST(schedule_cancel_with_time_map_size_2);
    RUN_TEST(schedule_cancel_with_time_contains_type);
    RUN_TEST(schedule_cancel_with_time_contains_time_key);
    RUN_TEST(schedule_cancel_different_times_differ);
    RUN_TEST(schedule_cancel_with_vs_without_time_differ);

    // EIP-712 hash tests
    RUN_TEST(schedule_cancel_hash_nonempty);
    RUN_TEST(schedule_cancel_hash_deterministic);
    RUN_TEST(schedule_cancel_hash_mainnet_vs_testnet);
    RUN_TEST(schedule_cancel_hash_with_vs_without_time);

    // Signing vector tests (Python SDK cross-validation)
    RUN_TEST(schedule_cancel_sign_no_time_mainnet);
    RUN_TEST(schedule_cancel_sign_no_time_testnet);
    RUN_TEST(schedule_cancel_sign_with_time_mainnet);
    RUN_TEST(schedule_cancel_sign_with_time_testnet);

    hl::crypto::cleanup();
    return printTestSummary();
}
