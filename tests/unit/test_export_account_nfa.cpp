//=============================================================================
// test_export_account_nfa.cpp - HL_EXPORT_ACCOUNT NFA column [OPM-801]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// DEPENDENCIES: hl_config.h, test_framework.h
// THREAD SAFETY: Not thread-safe (single-threaded test execution)
//=============================================================================
//
// HL_EXPORT_ACCOUNT (50003) writes an Accounts.csv template row. Its NFA
// column is the Zorro account-compliance bitfield, documented identically in
// two places:
//
//   docs/zorro_docs/Functions/brokercommand.md:41 (GET_COMPLIANCE)
//     1 = no partial closing, 2 = no hedging, 4 = FIFO compliance,
//     8 = no closing of trades, 15 = full NFA compliant account
//
//   docs/zorro_docs/Main_Topics/account.md:315 (Accounts.csv NFA column)
//     0 = no restrictions, 2 = Hedge=0 (no hedging),
//     14 or 15 = NFA = on (full NFA compliance)
//
// The column is the USER's setting - chosen in Accounts.csv or in the strategy
// script via set(NFA) / Hedge. The plugin must not bake an opinion into the
// template it emits, so the exported value is the neutral 0 ("no
// restrictions"), matching all live Hyperliquid rows in Accounts.csv.
//
// Regression guarded: OPM-213 changed the exported value to 2 to mean "no
// hedging, since HL doesn't support hedging". That silently imposed Hedge=0 on
// anyone regenerating Accounts.csv from the template. HL's lack of a hedge mode
// is a broker-API property that belongs in GET_COMPLIANCE, not in the user's
// account-config template.
//=============================================================================

#include "../test_framework.h"
#include "hl_config.h"

using namespace hl::test;

//-----------------------------------------------------------------------------
// Zorro account-compliance bitfield (brokercommand.md:41)
//-----------------------------------------------------------------------------
static const int ZORRO_NO_PARTIAL_CLOSING = 1;
static const int ZORRO_NO_HEDGING         = 2;
static const int ZORRO_FIFO_COMPLIANCE    = 4;
static const int ZORRO_NO_TRADE_CLOSING   = 8;

//-----------------------------------------------------------------------------
// TEST 1: The exported NFA column imposes no restrictions.
//-----------------------------------------------------------------------------
void test_export_nfa_is_neutral() {
    ASSERT_EQ(hl::config::EXPORT_ACCOUNT_NFA, 0);
}

//-----------------------------------------------------------------------------
// TEST 2: No individual compliance bit is set.
//
// Spelled out per-bit so a future edit to a non-zero value fails with a message
// naming the restriction it would silently impose on the user's account.
//-----------------------------------------------------------------------------
void test_export_nfa_sets_no_compliance_bits() {
    const int nfa = hl::config::EXPORT_ACCOUNT_NFA;

    ASSERT_MSG((nfa & ZORRO_NO_PARTIAL_CLOSING) == 0,
               "exported NFA must not force 'no partial closing' on the user");
    ASSERT_MSG((nfa & ZORRO_NO_HEDGING) == 0,
               "exported NFA must not force Hedge=0 on the user [OPM-801]");
    ASSERT_MSG((nfa & ZORRO_FIFO_COMPLIANCE) == 0,
               "exported NFA must not force FIFO compliance on the user");
    ASSERT_MSG((nfa & ZORRO_NO_TRADE_CLOSING) == 0,
               "exported NFA must not force 'no closing of trades' on the user");
}

//-----------------------------------------------------------------------------
// TEST 3: The value never reaches 14/15 (full NFA compliance).
//
// account.md:315 reserves 14/15 for NFA=on. brokerplugin.md:470-473: when the
// NFA flag is set, Zorro stops calling BrokerSell2 and closes via
// BrokerBuy2(StopDist=-1) instead. BrokerSell2 is this plugin's live close
// path, so exporting 14/15 would reroute closing onto the other path.
//-----------------------------------------------------------------------------
void test_export_nfa_does_not_enable_nfa_mode() {
    ASSERT_NE(hl::config::EXPORT_ACCOUNT_NFA, 14);
    ASSERT_NE(hl::config::EXPORT_ACCOUNT_NFA, 15);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("\n===========================================\n");
    printf(" HL_EXPORT_ACCOUNT NFA column [OPM-801]\n");
    printf("===========================================\n\n");

    RUN_TEST(export_nfa_is_neutral);
    RUN_TEST(export_nfa_sets_no_compliance_bits);
    RUN_TEST(export_nfa_does_not_enable_nfa_mode);

    return printTestSummary();
}
