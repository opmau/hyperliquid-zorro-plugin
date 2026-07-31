//=============================================================================
// test_export_account_nfa.cpp - HL_EXPORT_ACCOUNT NFA column [OPM-801]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// DEPENDENCIES: hl_config.h, hl_globals.h, test_framework.h
// THREAD SAFETY: Not thread-safe (single-threaded test execution)
//=============================================================================
//
// HL_EXPORT_ACCOUNT (50003) writes an Accounts.csv template row. Its NFA column
// is the Zorro account-compliance bitfield, documented identically in two
// places:
//
//   docs/zorro_docs/Functions/brokercommand.md:41 (GET_COMPLIANCE)
//     1 = no partial closing, 2 = no hedging, 4 = FIFO compliance,
//     8 = no closing of trades, 15 = full NFA compliant account
//
//   docs/zorro_docs/Main_Topics/account.md:315 (Accounts.csv NFA column)
//     0 = no restrictions, 2 = Hedge=0 (no hedging),
//     14 or 15 = NFA = on (full NFA compliance)
//
// The column is the USER's setting - chosen in Accounts.csv, or in the strategy
// via set(NFA) / Hedge. Zorro never passes it to the plugin, so the plugin
// cannot know it and must not impose one. It is therefore a runtime parameter
// (RuntimeConfig::exportNfa, set by brokerCommand 50004) whose default is the
// neutral 0, matching every live Hyperliquid account row.
//
// Regression guarded: OPM-213 hardcoded the exported value to 2 ("no hedging,
// since HL doesn't support hedging"), silently forcing Hedge=0 on anyone
// regenerating Accounts.csv from the template. That reasoning is invalid twice
// over - NFA and Hedge drive Zorro's own trade management (virtual hedging,
// phantom/pool trades, FIFO), which nets down before the exchange sees an
// order and stays useful on a broker with no hedge mode; and exchange
// capabilities change. The value must never be hardcoded again.
//=============================================================================

#include "../test_framework.h"
#include "hl_config.h"
#include "hl_globals.h"

using namespace hl::test;

//-----------------------------------------------------------------------------
// Zorro account-compliance bitfield (brokercommand.md:41)
//-----------------------------------------------------------------------------
static const int ZORRO_NO_PARTIAL_CLOSING = 1;
static const int ZORRO_NO_HEDGING         = 2;
static const int ZORRO_FIFO_COMPLIANCE    = 4;
static const int ZORRO_NO_TRADE_CLOSING   = 8;

//-----------------------------------------------------------------------------
// TEST 1: A freshly constructed RuntimeConfig exports a neutral NFA column.
//
// This is the value the export actually uses, so it is the one that matters -
// not merely the constant it is seeded from.
//-----------------------------------------------------------------------------
void test_runtime_default_is_neutral() {
    hl::RuntimeConfig cfg;
    ASSERT_EQ(cfg.exportNfa, 0);
    ASSERT_EQ(cfg.exportNfa, hl::config::EXPORT_ACCOUNT_NFA_DEFAULT);
}

//-----------------------------------------------------------------------------
// TEST 2: The default sets no individual compliance bit.
//
// Spelled out per-bit so a future edit to a non-zero default fails with a
// message naming the restriction it would silently impose on the user.
//-----------------------------------------------------------------------------
void test_default_sets_no_compliance_bits() {
    hl::RuntimeConfig cfg;

    ASSERT_MSG((cfg.exportNfa & ZORRO_NO_PARTIAL_CLOSING) == 0,
               "default NFA must not force 'no partial closing' on the user");
    ASSERT_MSG((cfg.exportNfa & ZORRO_NO_HEDGING) == 0,
               "default NFA must not force Hedge=0 on the user [OPM-801]");
    ASSERT_MSG((cfg.exportNfa & ZORRO_FIFO_COMPLIANCE) == 0,
               "default NFA must not force FIFO compliance on the user");
    ASSERT_MSG((cfg.exportNfa & ZORRO_NO_TRADE_CLOSING) == 0,
               "default NFA must not force 'no closing of trades' on the user");
}

//-----------------------------------------------------------------------------
// TEST 3: The default never enables full NFA mode.
//
// account.md:315 reserves 14/15 for NFA=on. brokerplugin.md:470-473: when the
// NFA flag is set, Zorro stops calling BrokerSell2 and closes via
// BrokerBuy2(StopDist=-1) instead. BrokerSell2 is this plugin's live close
// path, so defaulting to 14/15 would reroute closing onto the other path.
//-----------------------------------------------------------------------------
void test_default_does_not_enable_nfa_mode() {
    hl::RuntimeConfig cfg;
    ASSERT_NE(cfg.exportNfa, 14);
    ASSERT_NE(cfg.exportNfa, 15);
}

//-----------------------------------------------------------------------------
// TEST 4: The column is a parameter - every legal compliance value round-trips.
//
// The user must be able to select any Zorro compliance setting, including the
// full-NFA values, because NFA drives Zorro-internal trade management and is
// useful regardless of what the exchange supports.
//-----------------------------------------------------------------------------
void test_every_compliance_value_is_settable() {
    hl::RuntimeConfig cfg;

    for (int nfa = 0; nfa <= 15; nfa++) {
        cfg.exportNfa = nfa;
        ASSERT_EQ(cfg.exportNfa, nfa);
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("\n===========================================\n");
    printf(" HL_EXPORT_ACCOUNT NFA column [OPM-801]\n");
    printf("===========================================\n\n");

    RUN_TEST(runtime_default_is_neutral);
    RUN_TEST(default_sets_no_compliance_bits);
    RUN_TEST(default_does_not_enable_nfa_mode);
    RUN_TEST(every_compliance_value_is_settable);

    return printTestSummary();
}
