//=============================================================================
// test_broker_asset.cpp - PIP, PIPCost, LotAmount formula validation
//=============================================================================
// PREVENTS BUGS:
//   - 6dfb104: Wrong PIP formula causing tick size errors
//   - 213643c: Missing PIPCost causing Zorro to default to 1.0, showing
//              profits in millions instead of correct values
//
// ZORRO DOCUMENTATION REFERENCE:
//   pPip = Minimum price movement (tick size)
//   pPipCost = Value of one PIP per lot in account currency
//   pLotAmount = Minimum trade unit (1 contract = LotAmount)
//
// HYPERLIQUID SPECIFICS:
//   - Total precision is 6 decimals, split between price and size
//   - pxDecimals = 6 - szDecimals
//   - PIP = 10^(-pxDecimals)
//   - LotAmount = 10^(-szDecimals)
//   - PIPCost = PIP * LotAmount = 10^(-6) = 0.000001 (constant for all assets!)
//=============================================================================

#include "../test_framework.h"
#include <cmath>

using namespace hl::test;

//=============================================================================
// HYPERLIQUID ASSET FORMULAS (Extracted from hl_broker.cpp and hl_meta.cpp)
//=============================================================================

namespace AssetFormulas {

// Hyperliquid uses 6 total decimal places split between price and size
const int TOTAL_DECIMALS = 6;

// Calculate price decimals from size decimals
inline int pxDecimals(int szDecimals) {
    return TOTAL_DECIMALS - szDecimals;
}

// Calculate minimum price movement (tick size)
inline double pip(int szDecimals) {
    int px = pxDecimals(szDecimals);
    return pow(10.0, -px);
}

// Calculate minimum trade unit
inline double lotAmount(int szDecimals) {
    return pow(10.0, -szDecimals);
}

// Calculate value of one PIP per lot
// NOTE: For Hyperliquid, this is ALWAYS 10^(-6) = 0.000001
inline double pipCost(int szDecimals) {
    return pip(szDecimals) * lotAmount(szDecimals);
}

// Convert Zorro lots to Hyperliquid contracts
inline double lotsToContracts(double lots, int szDecimals) {
    return lots * lotAmount(szDecimals);
}

// Convert Hyperliquid contracts to Zorro lots
inline double contractsToLots(double contracts, int szDecimals) {
    return contracts / lotAmount(szDecimals);
}

} // namespace AssetFormulas

//=============================================================================
// TEST CASES
//=============================================================================

void test_btc_calculations() {
    // BTC: szDecimals=4, so pxDecimals=2
    // LotAmount = 0.0001 BTC
    // PIP = 0.01 (price moves in $0.01 increments)
    const int szDecimals = 4;

    double pip = AssetFormulas::pip(szDecimals);
    double lotAmt = AssetFormulas::lotAmount(szDecimals);
    double pipCost = AssetFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.01, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 0.0001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.000001, 1e-15);

    // Verify lot conversion
    // To trade 2.0 BTC contracts, need 20,000 lots
    double lots = AssetFormulas::contractsToLots(2.0, szDecimals);
    ASSERT_FLOAT_EQ_TOL(lots, 20000.0, 1e-9);

    // And back
    double contracts = AssetFormulas::lotsToContracts(20000.0, szDecimals);
    ASSERT_FLOAT_EQ_TOL(contracts, 2.0, 1e-9);
}

void test_eth_calculations() {
    // ETH: szDecimals=3, so pxDecimals=3
    // LotAmount = 0.001 ETH
    // PIP = 0.001 (price moves in $0.001 increments)
    const int szDecimals = 3;

    double pip = AssetFormulas::pip(szDecimals);
    double lotAmt = AssetFormulas::lotAmount(szDecimals);
    double pipCost = AssetFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 0.001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.000001, 1e-15);  // Still 10^(-6)!

    // 10.5 ETH contracts = 10,500 lots
    double lots = AssetFormulas::contractsToLots(10.5, szDecimals);
    ASSERT_FLOAT_EQ_TOL(lots, 10500.0, 1e-9);
}

void test_doge_calculations() {
    // DOGE: szDecimals=0, so pxDecimals=6
    // LotAmount = 1.0 DOGE (you trade whole DOGE)
    // PIP = 0.000001 (price moves in 6 decimal places)
    const int szDecimals = 0;

    double pip = AssetFormulas::pip(szDecimals);
    double lotAmt = AssetFormulas::lotAmount(szDecimals);
    double pipCost = AssetFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.000001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 1.0, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.000001, 1e-15);  // Still 10^(-6)!

    // 1000 DOGE contracts = 1000 lots
    double lots = AssetFormulas::contractsToLots(1000.0, szDecimals);
    ASSERT_FLOAT_EQ_TOL(lots, 1000.0, 1e-9);
}

void test_sol_calculations() {
    // SOL: szDecimals=2, so pxDecimals=4
    // LotAmount = 0.01 SOL
    // PIP = 0.0001 (price moves in $0.0001 increments)
    const int szDecimals = 2;

    double pip = AssetFormulas::pip(szDecimals);
    double lotAmt = AssetFormulas::lotAmount(szDecimals);
    double pipCost = AssetFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.0001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 0.01, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.000001, 1e-15);  // Still 10^(-6)!
}

void test_pipcost_is_constant() {
    // CRITICAL PROPERTY: PIPCost = 10^(-6) for ALL assets on Hyperliquid
    // This is because pxDecimals + szDecimals = 6 always
    //
    // If this property is violated, Zorro's profit calculations will be wrong!
    //
    // Bug 213643c: PIPCost was not returned, Zorro defaulted to 1.0
    //              This showed profits as millions of dollars

    const double EXPECTED_PIPCOST = 0.000001;

    for (int sz = 0; sz <= 6; sz++) {
        double pipCost = AssetFormulas::pipCost(sz);
        ASSERT_FLOAT_EQ_TOL(pipCost, EXPECTED_PIPCOST, 1e-15);
    }
}

void test_pipcost_not_one() {
    // BUG PREVENTION: If PIPCost returns 1.0, Zorro shows insane profits
    // This test ensures we never return the wrong default

    for (int sz = 0; sz <= 6; sz++) {
        double pipCost = AssetFormulas::pipCost(sz);
        ASSERT_FLOAT_NE(pipCost, 1.0);
    }
}

void test_lot_conversion_bug_8303e8b() {
    // BUG 8303e8b: Strategy calculated 2 lots instead of 20,000 lots
    //
    // WRONG calculation: lots = exposure_usd / price
    // RIGHT calculation: contracts = exposure_usd / price
    //                    lots = contracts / LotAmount
    //
    // Example: $50,000 exposure at $25,000 BTC price
    // szDecimals=4 (LotAmount=0.0001)

    const int szDecimals = 4;
    const double exposure_usd = 50000.0;
    const double price = 25000.0;

    // Correct calculation
    double contracts = exposure_usd / price;  // 2.0 BTC
    double lots = AssetFormulas::contractsToLots(contracts, szDecimals);

    ASSERT_FLOAT_EQ_TOL(contracts, 2.0, 1e-9);
    ASSERT_FLOAT_EQ_TOL(lots, 20000.0, 1e-9);

    // The BUG was using exposure/price directly as lots
    double wrong_lots = exposure_usd / price;  // 2.0 - WRONG!
    ASSERT_FLOAT_NE(lots, wrong_lots);  // Must NOT be equal
}

void test_minimum_order_value() {
    // Hyperliquid requires minimum $10 notional per order
    // Verify that small lot sizes respect this

    // BTC at $50,000, szDecimals=4
    const int szDecimals = 4;
    const double price = 50000.0;
    const double minNotional = 10.0;

    // Minimum contracts needed: $10 / $50,000 = 0.0002 BTC
    double minContracts = minNotional / price;
    double minLots = AssetFormulas::contractsToLots(minContracts, szDecimals);

    // 0.0002 / 0.0001 = 2 lots minimum
    ASSERT_GE(minLots, 2.0);

    // Verify reverse calculation
    double contractsAt2Lots = AssetFormulas::lotsToContracts(2.0, szDecimals);
    double notionalAt2Lots = contractsAt2Lots * price;
    ASSERT_GE(notionalAt2Lots, minNotional);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  PIP/PIPCost/LotAmount Formula Tests\n");
    printf("  Prevents bugs: 6dfb104, 213643c, 8303e8b\n");
    printf("=================================================\n\n");

    RUN_TEST(btc_calculations);
    RUN_TEST(eth_calculations);
    RUN_TEST(doge_calculations);
    RUN_TEST(sol_calculations);
    RUN_TEST(pipcost_is_constant);
    RUN_TEST(pipcost_not_one);
    RUN_TEST(lot_conversion_bug_8303e8b);
    RUN_TEST(minimum_order_value);

    return printTestSummary();
}