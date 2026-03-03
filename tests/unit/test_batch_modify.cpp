//=============================================================================
// test_batch_modify.cpp - batchModify msgpack encoding tests [OPM-80]
//=============================================================================
// PREVENTS BUGS:
//   - Incorrect msgpack field ordering causing signature mismatch
//   - Wrong action type string ("batchModify") causing exchange rejection
//   - Numeric vs string oid encoding mismatch
//
// VERIFIED AGAINST:
//   - Python SDK bulk_modify_orders_new() (exchange.py:195-223)
//   - Python SDK order_request_to_order_wire() (signing.py:504-515)
//   - Hyperliquid API docs (06-exchange-endpoint.md)
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
// TEST: packBatchModifyAction produces non-empty output
//=============================================================================

TEST_CASE(batch_modify_pack_nonempty) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,  // numeric oid
        0, true, "50000", "0.1", false, "Gtc", "");
    ASSERT_TRUE(result.size() > 0);
}

//=============================================================================
// TEST: packBatchModifyAction contains correct action type string
//=============================================================================

TEST_CASE(batch_modify_contains_type) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "batchModify"));
}

//=============================================================================
// TEST: packBatchModifyAction outer map has 2 entries (type + modifies)
//=============================================================================

TEST_CASE(batch_modify_outer_map_size) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    // fixmap with 2 entries = 0x82
    ASSERT_TRUE(result.size() > 0);
    ASSERT_EQ(result[0], (uint8_t)0x82);
}

//=============================================================================
// TEST: packBatchModifyAction "type" key comes first
//=============================================================================

TEST_CASE(batch_modify_type_key_first) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    // After 0x82 (map2), first key should be "type" (fixstr len 4 = 0xa4)
    ASSERT_TRUE(result.size() > 5);
    ASSERT_EQ(result[1], (uint8_t)0xa4);  // fixstr len 4
    ASSERT_EQ(result[2], (uint8_t)'t');
    ASSERT_EQ(result[3], (uint8_t)'y');
    ASSERT_EQ(result[4], (uint8_t)'p');
    ASSERT_EQ(result[5], (uint8_t)'e');
}

//=============================================================================
// TEST: packBatchModifyAction contains "modifies" key
//=============================================================================

TEST_CASE(batch_modify_contains_modifies_key) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "modifies"));
}

//=============================================================================
// TEST: packBatchModifyAction contains "oid" key
//=============================================================================

TEST_CASE(batch_modify_contains_oid_key) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "oid"));
}

//=============================================================================
// TEST: packBatchModifyAction contains "order" key
//=============================================================================

TEST_CASE(batch_modify_contains_order_key) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "order"));
}

//=============================================================================
// TEST: packBatchModifyAction encodes price as string
//=============================================================================

TEST_CASE(batch_modify_price_is_string) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50123.45", "0.1", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "50123.45"));
}

//=============================================================================
// TEST: packBatchModifyAction encodes size as string
//=============================================================================

TEST_CASE(batch_modify_size_is_string) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1234", false, "Gtc", "");
    ASSERT_TRUE(containsString(result, "0.1234"));
}

//=============================================================================
// TEST: packBatchModifyAction encodes boolean fields
//=============================================================================

TEST_CASE(batch_modify_booleans) {
    // isBuy=true, reduceOnly=false
    ByteArray result = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    bool hasTrue = false, hasFalse = false;
    for (uint8_t b : result) {
        if (b == 0xc3) hasTrue = true;
        if (b == 0xc2) hasFalse = true;
    }
    ASSERT_TRUE(hasTrue);   // isBuy=true
    ASSERT_TRUE(hasFalse);  // reduceOnly=false
}

//=============================================================================
// TEST: Cloid-based oid is encoded as string, not integer
//=============================================================================

TEST_CASE(batch_modify_cloid_oid_is_string) {
    const char* cloidHex = "0x1234abcd";
    ByteArray result = packBatchModifyAction(
        0, cloidHex, true,  // cloid oid
        0, true, "50000", "0.1", false, "Gtc", "");
    // The cloid hex string should appear literally in the packed data
    ASSERT_TRUE(containsString(result, cloidHex));
}

//=============================================================================
// TEST: Numeric oid does NOT appear as string
//=============================================================================

TEST_CASE(batch_modify_numeric_oid_not_string) {
    ByteArray result = packBatchModifyAction(
        12345, "", false,  // numeric oid
        0, true, "50000", "0.1", false, "Gtc", "");
    // "12345" as a string should NOT appear — it should be packed as an integer
    ASSERT_TRUE(!containsString(result, "12345"));
}

//=============================================================================
// TEST: Different inputs produce different outputs
//=============================================================================

TEST_CASE(batch_modify_different_params) {
    ByteArray a = packBatchModifyAction(
        12345, "", false,
        0, true, "50000", "0.1", false, "Gtc", "");
    ByteArray b = packBatchModifyAction(
        99999, "", false,
        1, false, "60000", "0.2", true, "Ioc", "");
    ASSERT_TRUE(a != b);
}

//=============================================================================
// TEST: packOrderWire produces expected field keys (a, b, p, s, r, t)
//=============================================================================

TEST_CASE(order_wire_contains_field_keys) {
    Packer packer;
    packOrderWire(packer, 0, true, "50000", "0.1", false, "Gtc", "");
    ByteArray result = packer.data();

    // Single-char keys encoded as fixstr len 1 = 0xa1 followed by the char
    // Check for key "a" (0xa1 0x61)
    bool foundA = false;
    for (size_t i = 0; i + 1 < result.size(); i++) {
        if (result[i] == 0xa1 && result[i+1] == 'a') { foundA = true; break; }
    }
    ASSERT_TRUE(foundA);

    // Check for key "b" (0xa1 0x62)
    bool foundB = false;
    for (size_t i = 0; i + 1 < result.size(); i++) {
        if (result[i] == 0xa1 && result[i+1] == 'b') { foundB = true; break; }
    }
    ASSERT_TRUE(foundB);

    // Check for key "p" (0xa1 0x70)
    bool foundP = false;
    for (size_t i = 0; i + 1 < result.size(); i++) {
        if (result[i] == 0xa1 && result[i+1] == 'p') { foundP = true; break; }
    }
    ASSERT_TRUE(foundP);

    // Check for key "s" (0xa1 0x73)
    bool foundS = false;
    for (size_t i = 0; i + 1 < result.size(); i++) {
        if (result[i] == 0xa1 && result[i+1] == 's') { foundS = true; break; }
    }
    ASSERT_TRUE(foundS);
}

//=============================================================================
// TEST: packOrderWire without cloid has 6 fields, with cloid has 7
//=============================================================================

TEST_CASE(order_wire_field_count) {
    // Without cloid — 6 fields {a, b, p, s, r, t}
    Packer p1;
    packOrderWire(p1, 0, true, "50000", "0.1", false, "Gtc", "");
    ByteArray r1 = p1.data();
    // First byte should be fixmap(6) = 0x86
    ASSERT_EQ(r1[0], (uint8_t)0x86);

    // With cloid — 7 fields {a, b, p, s, r, t, c}
    Packer p2;
    packOrderWire(p2, 0, true, "50000", "0.1", false, "Gtc", "0xabcdef");
    ByteArray r2 = p2.data();
    // First byte should be fixmap(7) = 0x87
    ASSERT_EQ(r2[0], (uint8_t)0x87);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== batchModify Msgpack Tests [OPM-80] ===\n\n");

    RUN_TEST(batch_modify_pack_nonempty);
    RUN_TEST(batch_modify_contains_type);
    RUN_TEST(batch_modify_outer_map_size);
    RUN_TEST(batch_modify_type_key_first);
    RUN_TEST(batch_modify_contains_modifies_key);
    RUN_TEST(batch_modify_contains_oid_key);
    RUN_TEST(batch_modify_contains_order_key);
    RUN_TEST(batch_modify_price_is_string);
    RUN_TEST(batch_modify_size_is_string);
    RUN_TEST(batch_modify_booleans);
    RUN_TEST(batch_modify_cloid_oid_is_string);
    RUN_TEST(batch_modify_numeric_oid_not_string);
    RUN_TEST(batch_modify_different_params);
    RUN_TEST(order_wire_contains_field_keys);
    RUN_TEST(order_wire_field_count);

    return printTestSummary();
}
