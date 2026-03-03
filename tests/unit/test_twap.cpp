//=============================================================================
// test_twap.cpp - TWAP order msgpack encoding tests [OPM-81]
//=============================================================================
// PREVENTS BUGS:
//   - Incorrect msgpack field ordering causing signature mismatch
//   - Wrong action type strings causing exchange rejection
//
// VERIFIED AGAINST:
//   - Chainstack TWAP guide (action dict structure)
//   - Hyperliquid API docs (06-exchange-endpoint.md:1349-1501)
//   - Python SDK signing.py (sign_l1_action → msgpack.packb)
//=============================================================================

#include "../test_framework.h"
#include "hl_msgpack.h"
#include <cstdio>

using namespace hl::test;
using namespace hl::msgpack;

//=============================================================================
// HELPER: Print bytes as hex for debugging
//=============================================================================

static void printHex(const char* label, const ByteArray& data) {
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
// TEST: packTwapOrderAction produces non-empty output
//=============================================================================

TEST_CASE(twap_order_pack_nonempty) {
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    ASSERT_TRUE(result.size() > 0);
}

//=============================================================================
// TEST: packTwapOrderAction contains correct action type string
//=============================================================================

TEST_CASE(twap_order_contains_type) {
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    ASSERT_TRUE(containsString(result, "twapOrder"));
}

//=============================================================================
// TEST: packTwapOrderAction contains "twap" key
//=============================================================================

TEST_CASE(twap_order_contains_twap_key) {
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    ASSERT_TRUE(containsString(result, "twap"));
}

//=============================================================================
// TEST: packTwapOrderAction outer map has 2 entries (type + twap)
//=============================================================================

TEST_CASE(twap_order_outer_map_size) {
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    // fixmap with 2 entries = 0x82
    ASSERT_TRUE(result.size() > 0);
    ASSERT_EQ(result[0], (uint8_t)0x82);
}

//=============================================================================
// TEST: packTwapOrderAction "type" key comes first
//=============================================================================

TEST_CASE(twap_order_type_key_first) {
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    // After 0x82 (map2), first key should be "type" (fixstr len 4 = 0xa4)
    ASSERT_TRUE(result.size() > 5);
    ASSERT_EQ(result[1], (uint8_t)0xa4);  // fixstr len 4
    ASSERT_EQ(result[2], (uint8_t)'t');
    ASSERT_EQ(result[3], (uint8_t)'y');
    ASSERT_EQ(result[4], (uint8_t)'p');
    ASSERT_EQ(result[5], (uint8_t)'e');
}

//=============================================================================
// TEST: packTwapOrderAction encodes size as string (not number)
//=============================================================================

TEST_CASE(twap_order_size_is_string) {
    ByteArray result = packTwapOrderAction(0, true, "10.5", false, 30, true);
    // Size "10.5" should appear as a msgpack string in the output
    ASSERT_TRUE(containsString(result, "10.5"));
}

//=============================================================================
// TEST: packTwapOrderAction encodes boolean fields
//=============================================================================

TEST_CASE(twap_order_booleans) {
    // isBuy=true, reduceOnly=false, randomize=true
    ByteArray result = packTwapOrderAction(0, true, "10", false, 30, true);
    // Should contain both true (0xc3) and false (0xc2) bytes
    bool hasTrue = false, hasFalse = false;
    for (uint8_t b : result) {
        if (b == 0xc3) hasTrue = true;
        if (b == 0xc2) hasFalse = true;
    }
    ASSERT_TRUE(hasTrue);   // isBuy=true or randomize=true
    ASSERT_TRUE(hasFalse);  // reduceOnly=false
}

//=============================================================================
// TEST: packTwapCancelAction produces non-empty output
//=============================================================================

TEST_CASE(twap_cancel_pack_nonempty) {
    ByteArray result = packTwapCancelAction(0, 12345);
    ASSERT_TRUE(result.size() > 0);
}

//=============================================================================
// TEST: packTwapCancelAction contains correct action type
//=============================================================================

TEST_CASE(twap_cancel_contains_type) {
    ByteArray result = packTwapCancelAction(0, 12345);
    ASSERT_TRUE(containsString(result, "twapCancel"));
}

//=============================================================================
// TEST: packTwapCancelAction outer map has 3 entries (type + a + t)
//=============================================================================

TEST_CASE(twap_cancel_outer_map_size) {
    ByteArray result = packTwapCancelAction(0, 12345);
    // fixmap with 3 entries = 0x83
    ASSERT_TRUE(result.size() > 0);
    ASSERT_EQ(result[0], (uint8_t)0x83);
}

//=============================================================================
// TEST: packTwapCancelAction "type" key comes first
//=============================================================================

TEST_CASE(twap_cancel_type_key_first) {
    ByteArray result = packTwapCancelAction(0, 12345);
    ASSERT_TRUE(result.size() > 5);
    ASSERT_EQ(result[1], (uint8_t)0xa4);  // fixstr len 4
    ASSERT_EQ(result[2], (uint8_t)'t');
    ASSERT_EQ(result[3], (uint8_t)'y');
    ASSERT_EQ(result[4], (uint8_t)'p');
    ASSERT_EQ(result[5], (uint8_t)'e');
}

//=============================================================================
// TEST: Different inputs produce different outputs
//=============================================================================

TEST_CASE(twap_order_different_params) {
    ByteArray a = packTwapOrderAction(0, true, "10", false, 30, true);
    ByteArray b = packTwapOrderAction(1, false, "20", true, 60, false);
    ASSERT_TRUE(a != b);
}

TEST_CASE(twap_cancel_different_params) {
    ByteArray a = packTwapCancelAction(0, 12345);
    ByteArray b = packTwapCancelAction(1, 99999);
    ASSERT_TRUE(a != b);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== TWAP Order Msgpack Tests [OPM-81] ===\n\n");

    RUN_TEST(twap_order_pack_nonempty);
    RUN_TEST(twap_order_contains_type);
    RUN_TEST(twap_order_contains_twap_key);
    RUN_TEST(twap_order_outer_map_size);
    RUN_TEST(twap_order_type_key_first);
    RUN_TEST(twap_order_size_is_string);
    RUN_TEST(twap_order_booleans);
    RUN_TEST(twap_order_different_params);

    RUN_TEST(twap_cancel_pack_nonempty);
    RUN_TEST(twap_cancel_contains_type);
    RUN_TEST(twap_cancel_outer_map_size);
    RUN_TEST(twap_cancel_type_key_first);
    RUN_TEST(twap_cancel_different_params);

    return printTestSummary();
}
