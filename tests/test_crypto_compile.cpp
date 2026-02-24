//=============================================================================
// test_crypto_compile.cpp - Compilation and basic functionality test for hl_crypto
//=============================================================================

#include "hl_crypto.h"
#include <cstdio>
#include <cstring>

// Test private key (DO NOT USE IN PRODUCTION - this is a well-known test key)
// This is the first test account private key from Hardhat/Ganache
static const char* TEST_PRIVATE_KEY = "0xac0974bec39a17e36ba4a6b4d238ff944bacb478cbed5efcae784d7bf4f2ff80";
// Expected address: 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266

int main() {
    printf("=== hl_crypto module test ===\n\n");
    int passed = 0;
    int failed = 0;

    // Test 1: Initialization
    printf("[1] Testing crypto::init()...\n");
    if (hl::crypto::init()) {
        printf("    PASSED: Crypto initialized\n");
        passed++;
    } else {
        printf("    FAILED: Could not initialize crypto\n");
        failed++;
        return 1;
    }

    // Test 2: isInitialized
    printf("[2] Testing crypto::isInitialized()...\n");
    if (hl::crypto::isInitialized()) {
        printf("    PASSED: isInitialized returns true\n");
        passed++;
    } else {
        printf("    FAILED: isInitialized returns false\n");
        failed++;
    }

    // Test 3: Address derivation
    printf("[3] Testing crypto::deriveAddress()...\n");
    char address[64];
    if (hl::crypto::deriveAddress(TEST_PRIVATE_KEY, address, sizeof(address))) {
        printf("    Derived address: %s\n", address);
        // Check format (should start with 0x and be 42 chars)
        if (strlen(address) == 42 && address[0] == '0' && address[1] == 'x') {
            printf("    PASSED: Address format valid\n");
            passed++;
        } else {
            printf("    FAILED: Invalid address format\n");
            failed++;
        }
    } else {
        printf("    FAILED: Could not derive address\n");
        failed++;
    }

    // Test 4: hexToBytes
    printf("[4] Testing crypto::hexToBytes()...\n");
    uint8_t bytes[4];
    size_t len = hl::crypto::hexToBytes("0xdeadbeef", bytes, sizeof(bytes));
    if (len == 4 && bytes[0] == 0xde && bytes[1] == 0xad &&
        bytes[2] == 0xbe && bytes[3] == 0xef) {
        printf("    PASSED: hexToBytes works correctly\n");
        passed++;
    } else {
        printf("    FAILED: hexToBytes returned wrong values\n");
        failed++;
    }

    // Test 5: bytesToHex
    printf("[5] Testing crypto::bytesToHex()...\n");
    uint8_t testData[] = {0xca, 0xfe, 0xba, 0xbe};
    char hexOut[16];
    hl::crypto::bytesToHex(testData, 4, hexOut, sizeof(hexOut));
    if (strcmp(hexOut, "0xcafebabe") == 0) {
        printf("    PASSED: bytesToHex works correctly (%s)\n", hexOut);
        passed++;
    } else {
        printf("    FAILED: bytesToHex returned '%s', expected '0xcafebabe'\n", hexOut);
        failed++;
    }

    // Test 6: keccak256
    printf("[6] Testing crypto::keccak256()...\n");
    // keccak256("") = c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470
    uint8_t emptyHash[32];
    hl::crypto::keccak256(nullptr, 0, emptyHash);
    // Check first few bytes
    if (emptyHash[0] == 0xc5 && emptyHash[1] == 0xd2 && emptyHash[2] == 0x46) {
        printf("    PASSED: keccak256 of empty string matches expected\n");
        passed++;
    } else {
        printf("    FAILED: keccak256 hash mismatch\n");
        failed++;
    }

    // Test 7: Hash signing
    printf("[7] Testing crypto::signHash()...\n");
    uint8_t testHash[32] = {0};  // Zero hash for testing
    memset(testHash, 0x42, 32);  // Fill with 0x42 for test
    hl::crypto::Signature sig;
    if (hl::crypto::signHash(testHash, TEST_PRIVATE_KEY, sig)) {
        printf("    Signature r: %s\n", sig.r);
        printf("    Signature s: %s\n", sig.s);
        printf("    Signature v: %d\n", sig.v);
        // Verify format
        if (strlen(sig.r) == 66 && strlen(sig.s) == 66 &&
            (sig.v == 27 || sig.v == 28)) {
            printf("    PASSED: Signature format valid\n");
            passed++;
        } else {
            printf("    FAILED: Invalid signature format\n");
            failed++;
        }
    } else {
        printf("    FAILED: Could not sign hash\n");
        failed++;
    }

    // Test 8: Signature toJson
    printf("[8] Testing Signature::toJson()...\n");
    std::string json = sig.toJson();
    if (json.find("\"r\":") != std::string::npos &&
        json.find("\"s\":") != std::string::npos &&
        json.find("\"v\":") != std::string::npos) {
        printf("    JSON: %s\n", json.c_str());
        printf("    PASSED: JSON format valid\n");
        passed++;
    } else {
        printf("    FAILED: Invalid JSON format\n");
        failed++;
    }

    // Cleanup
    printf("[9] Testing crypto::cleanup()...\n");
    hl::crypto::cleanup();
    if (!hl::crypto::isInitialized()) {
        printf("    PASSED: Cleanup successful\n");
        passed++;
    } else {
        printf("    FAILED: Still initialized after cleanup\n");
        failed++;
    }

    // Summary
    printf("\n=== RESULTS ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("================\n");

    return failed > 0 ? 1 : 0;
}
