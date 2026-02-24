//=============================================================================
// test_eip712_source.cpp - Verify EIP-712 source differs for mainnet/testnet
//=============================================================================
// PREVENTS BUG: OPM-22 (e392a43)
//   EIP-712 signing hardcoded source='a' (mainnet-only). Testnet requires
//   source='b'. Using the wrong source produces a different message hash,
//   causing the exchange to recover a completely different address from the
//   signature — all orders rejected with "User does not exist."
//
// KEY PROPERTY:
//   hashOrderForSigning(action, isMainnet=true, nonce)
//     != hashOrderForSigning(action, isMainnet=false, nonce)
//
//   Same order, same nonce, different network → different hash.
//   If these are equal, testnet signing is broken.
//=============================================================================

#include "../test_framework.h"
#include "hl_eip712.h"

using namespace hl::test;
using namespace hl::eip712;

//=============================================================================
// HELPERS
//=============================================================================

// Fixed nonce so tests are deterministic (not wall-clock dependent)
static const uint64_t TEST_NONCE = 1700000000000ULL;

static OrderAction makeTestOrder() {
    OrderAction action;
    action.asset = 3;
    action.isBuy = true;
    action.price = "50000";
    action.size = "0.1";
    action.reduceOnly = false;
    action.orderType = "Ioc";
    action.cloid = "";
    return action;
}

static CancelAction makeTestCancel() {
    CancelAction action;
    action.asset = 3;
    action.orderId = 123456789;
    return action;
}

//=============================================================================
// TEST CASES
//=============================================================================

void test_order_hash_mainnet_vs_testnet() {
    // THE core regression test for OPM-22:
    // Same order signed for mainnet vs testnet MUST produce different hashes.
    OrderAction action = makeTestOrder();

    ByteArray mainnetHash = hashOrderForSigning(action, true, TEST_NONCE);
    ByteArray testnetHash = hashOrderForSigning(action, false, TEST_NONCE);

    ASSERT_EQ(mainnetHash.size(), (size_t)32);
    ASSERT_EQ(testnetHash.size(), (size_t)32);
    ASSERT_TRUE(mainnetHash != testnetHash);
}

void test_cancel_hash_mainnet_vs_testnet() {
    CancelAction action = makeTestCancel();

    ByteArray mainnetHash = hashCancelForSigning(action, true, TEST_NONCE);
    ByteArray testnetHash = hashCancelForSigning(action, false, TEST_NONCE);

    ASSERT_EQ(mainnetHash.size(), (size_t)32);
    ASSERT_EQ(testnetHash.size(), (size_t)32);
    ASSERT_TRUE(mainnetHash != testnetHash);
}

void test_agent_source_a_vs_b() {
    // Direct test of encodeAgentType with different source values.
    ByteArray connectionId(32, 0x42);

    ByteArray hashA = encodeAgentType("a", connectionId);
    ByteArray hashB = encodeAgentType("b", connectionId);

    ASSERT_EQ(hashA.size(), (size_t)32);
    ASSERT_EQ(hashB.size(), (size_t)32);
    ASSERT_TRUE(hashA != hashB);
}

void test_order_hash_deterministic() {
    OrderAction action = makeTestOrder();

    ByteArray hash1 = hashOrderForSigning(action, true, TEST_NONCE);
    ByteArray hash2 = hashOrderForSigning(action, true, TEST_NONCE);

    ASSERT_EQ(hash1.size(), (size_t)32);
    ASSERT_TRUE(hash1 == hash2);
}

void test_cancel_hash_deterministic() {
    CancelAction action = makeTestCancel();

    ByteArray hash1 = hashCancelForSigning(action, false, TEST_NONCE);
    ByteArray hash2 = hashCancelForSigning(action, false, TEST_NONCE);

    ASSERT_EQ(hash1.size(), (size_t)32);
    ASSERT_TRUE(hash1 == hash2);
}

void test_different_nonce_different_hash() {
    OrderAction action = makeTestOrder();

    ByteArray hash1 = hashOrderForSigning(action, true, TEST_NONCE);
    ByteArray hash2 = hashOrderForSigning(action, true, TEST_NONCE + 1);

    ASSERT_EQ(hash1.size(), (size_t)32);
    ASSERT_EQ(hash2.size(), (size_t)32);
    ASSERT_TRUE(hash1 != hash2);
}

void test_domain_separator_constant() {
    HyperliquidDomain domain;
    ByteArray sep1 = encodeDomainSeparator(domain);
    ByteArray sep2 = encodeDomainSeparator(domain);

    ASSERT_EQ(sep1.size(), (size_t)32);
    ASSERT_TRUE(sep1 == sep2);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  EIP-712 Source (Mainnet vs Testnet) Tests\n");
    printf("  Prevents bug: OPM-22 (e392a43)\n");
    printf("=================================================\n\n");

    RUN_TEST(order_hash_mainnet_vs_testnet);
    RUN_TEST(cancel_hash_mainnet_vs_testnet);
    RUN_TEST(agent_source_a_vs_b);
    RUN_TEST(order_hash_deterministic);
    RUN_TEST(cancel_hash_deterministic);
    RUN_TEST(different_nonce_different_hash);
    RUN_TEST(domain_separator_constant);

    return printTestSummary();
}
