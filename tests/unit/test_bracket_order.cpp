//=============================================================================
// test_bracket_order.cpp - Bracket order msgpack encoding tests [OPM-79]
//=============================================================================
// PREVENTS BUGS:
//   - Incorrect msgpack field ordering causing signature mismatch
//   - Missing orders in bracket array
//   - Wrong grouping string
//   - Incorrect trigger order fields for TP/SL
//
// VERIFIED AGAINST:
//   - Hyperliquid API docs (06-exchange-endpoint.md:98)
//   - Python SDK signing.py (sign_l1_action → msgpack.packb)
//=============================================================================

#include "../test_framework.h"
#include "hl_msgpack.h"
#include <cstdio>

using namespace hl::test;
using namespace hl::msgpack;

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
// TEST: packBracketOrderAction produces non-empty output
//=============================================================================

TEST_CASE(bracket_pack_nonempty) {
    std::vector<BracketOrderWire> wires;

    // Entry
    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = true;
    entry.price = "50000";
    entry.size = "0.001";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "0x1234";
    entry.isTrigger = false;
    wires.push_back(entry);

    // TP
    BracketOrderWire tp;
    tp.asset = 0;
    tp.isBuy = false;
    tp.price = "55000";
    tp.size = "0.001";
    tp.reduceOnly = true;
    tp.cloid = "0x5678";
    tp.isTrigger = true;
    tp.triggerIsMarket = true;
    tp.triggerPx = "55000";
    tp.tpsl = "tp";
    wires.push_back(tp);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");
    ASSERT_TRUE(result.size() > 0);
}

//=============================================================================
// TEST: packBracketOrderAction contains "order" type string
//=============================================================================

TEST_CASE(bracket_contains_order_type) {
    std::vector<BracketOrderWire> wires;

    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = true;
    entry.price = "50000";
    entry.size = "0.001";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "";
    entry.isTrigger = false;
    wires.push_back(entry);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");
    ASSERT_TRUE(containsString(result, "order"));
}

//=============================================================================
// TEST: packBracketOrderAction contains "normalTpsl" grouping
//=============================================================================

TEST_CASE(bracket_contains_normalTpsl_grouping) {
    std::vector<BracketOrderWire> wires;

    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = true;
    entry.price = "50000";
    entry.size = "0.001";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "";
    entry.isTrigger = false;
    wires.push_back(entry);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");
    ASSERT_TRUE(containsString(result, "normalTpsl"));
}

//=============================================================================
// TEST: packBracketOrderAction contains "positionTpsl" when specified
//=============================================================================

TEST_CASE(bracket_contains_positionTpsl_grouping) {
    std::vector<BracketOrderWire> wires;

    BracketOrderWire sl;
    sl.asset = 0;
    sl.isBuy = false;
    sl.price = "45000";
    sl.size = "0.001";
    sl.reduceOnly = true;
    sl.cloid = "";
    sl.isTrigger = true;
    sl.triggerIsMarket = true;
    sl.triggerPx = "45000";
    sl.tpsl = "sl";
    wires.push_back(sl);

    ByteArray result = packBracketOrderAction(wires, "positionTpsl");
    ASSERT_TRUE(containsString(result, "positionTpsl"));
    ASSERT_FALSE(containsString(result, "normalTpsl"));
}

//=============================================================================
// TEST: Full bracket (entry + TP + SL) encodes all three
//=============================================================================

TEST_CASE(bracket_full_three_orders) {
    std::vector<BracketOrderWire> wires;

    // Entry
    BracketOrderWire entry;
    entry.asset = 3;
    entry.isBuy = true;
    entry.price = "50000";
    entry.size = "0.001";
    entry.reduceOnly = false;
    entry.tif = "Gtc";
    entry.cloid = "0xentry";
    entry.isTrigger = false;
    wires.push_back(entry);

    // TP
    BracketOrderWire tp;
    tp.asset = 3;
    tp.isBuy = false;
    tp.price = "55000";
    tp.size = "0.001";
    tp.reduceOnly = true;
    tp.cloid = "0xtp";
    tp.isTrigger = true;
    tp.triggerIsMarket = true;
    tp.triggerPx = "55000";
    tp.tpsl = "tp";
    wires.push_back(tp);

    // SL
    BracketOrderWire sl;
    sl.asset = 3;
    sl.isBuy = false;
    sl.price = "45000";
    sl.size = "0.001";
    sl.reduceOnly = true;
    sl.cloid = "0xsl";
    sl.isTrigger = true;
    sl.triggerIsMarket = true;
    sl.triggerPx = "45000";
    sl.tpsl = "sl";
    wires.push_back(sl);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");

    // Must contain all key strings
    ASSERT_TRUE(containsString(result, "type"));
    ASSERT_TRUE(containsString(result, "order"));
    ASSERT_TRUE(containsString(result, "orders"));
    ASSERT_TRUE(containsString(result, "grouping"));
    ASSERT_TRUE(containsString(result, "normalTpsl"));

    // Must contain trigger strings for TP/SL
    ASSERT_TRUE(containsString(result, "trigger"));
    ASSERT_TRUE(containsString(result, "triggerPx"));
    ASSERT_TRUE(containsString(result, "isMarket"));
    ASSERT_TRUE(containsString(result, "tp"));
    ASSERT_TRUE(containsString(result, "sl"));

    // Must contain CLOIDs
    ASSERT_TRUE(containsString(result, "0xentry"));
    ASSERT_TRUE(containsString(result, "0xtp"));
    ASSERT_TRUE(containsString(result, "0xsl"));

    // Must contain limit type for entry
    ASSERT_TRUE(containsString(result, "Gtc"));
}

//=============================================================================
// TEST: Bracket with entry + SL only (no TP)
//=============================================================================

TEST_CASE(bracket_entry_and_sl_only) {
    std::vector<BracketOrderWire> wires;

    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = true;
    entry.price = "50000";
    entry.size = "0.1";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "0xentry";
    entry.isTrigger = false;
    wires.push_back(entry);

    BracketOrderWire sl;
    sl.asset = 0;
    sl.isBuy = false;
    sl.price = "45000";
    sl.size = "0.1";
    sl.reduceOnly = true;
    sl.cloid = "0xsl";
    sl.isTrigger = true;
    sl.triggerIsMarket = true;
    sl.triggerPx = "45000";
    sl.tpsl = "sl";
    wires.push_back(sl);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");

    ASSERT_TRUE(containsString(result, "normalTpsl"));
    ASSERT_TRUE(containsString(result, "0xentry"));
    ASSERT_TRUE(containsString(result, "0xsl"));
    ASSERT_TRUE(containsString(result, "sl"));
    // Should NOT contain "tp" as tpsl value (only as substring of other strings)
}

//=============================================================================
// TEST: Msgpack field order is type → orders → grouping (matches Python SDK)
//=============================================================================

TEST_CASE(bracket_field_order_type_orders_grouping) {
    std::vector<BracketOrderWire> wires;

    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = true;
    entry.price = "100";
    entry.size = "1";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "";
    entry.isTrigger = false;
    wires.push_back(entry);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");

    // Find positions of key strings in the byte array
    auto findPos = [&](const char* str) -> int {
        size_t len = strlen(str);
        for (size_t i = 0; i <= result.size() - len; i++) {
            if (memcmp(&result[i], str, len) == 0) return (int)i;
        }
        return -1;
    };

    int posType = findPos("type");
    int posOrders = findPos("orders");
    int posGrouping = findPos("grouping");

    // All must exist
    ASSERT_TRUE(posType >= 0);
    ASSERT_TRUE(posOrders >= 0);
    ASSERT_TRUE(posGrouping >= 0);

    // Must be in order: type < orders < grouping
    ASSERT_TRUE(posType < posOrders);
    ASSERT_TRUE(posOrders < posGrouping);
}

//=============================================================================
// TEST: Short entry bracket has buy side for TP/SL
//=============================================================================

TEST_CASE(bracket_short_entry_tp_is_buy) {
    // For a short entry, TP should buy (close at lower price)
    // This tests the wire construction, not the service logic
    std::vector<BracketOrderWire> wires;

    // Short entry
    BracketOrderWire entry;
    entry.asset = 0;
    entry.isBuy = false;  // SELL entry
    entry.price = "50000";
    entry.size = "0.001";
    entry.reduceOnly = false;
    entry.tif = "Ioc";
    entry.cloid = "";
    entry.isTrigger = false;
    wires.push_back(entry);

    // TP (buy to close short)
    BracketOrderWire tp;
    tp.asset = 0;
    tp.isBuy = true;  // BUY to close
    tp.price = "45000";
    tp.size = "0.001";
    tp.reduceOnly = true;
    tp.cloid = "";
    tp.isTrigger = true;
    tp.triggerIsMarket = true;
    tp.triggerPx = "45000";
    tp.tpsl = "tp";
    wires.push_back(tp);

    ByteArray result = packBracketOrderAction(wires, "normalTpsl");
    ASSERT_TRUE(result.size() > 0);
    ASSERT_TRUE(containsString(result, "normalTpsl"));
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n=== Bracket Order Encoding Tests [OPM-79] ===\n\n");

    RUN_TEST(bracket_pack_nonempty);
    RUN_TEST(bracket_contains_order_type);
    RUN_TEST(bracket_contains_normalTpsl_grouping);
    RUN_TEST(bracket_contains_positionTpsl_grouping);
    RUN_TEST(bracket_full_three_orders);
    RUN_TEST(bracket_entry_and_sl_only);
    RUN_TEST(bracket_field_order_type_orders_grouping);
    RUN_TEST(bracket_short_entry_tp_is_buy);

    printf("\n--- Results: %d passed, %d failed ---\n",
           g_testsPassed, g_testsFailed);

    return g_testsFailed > 0 ? 1 : 0;
}
