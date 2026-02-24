//=============================================================================
// test_spot_asset.cpp - Spot asset formula and normalizeCoin validation
//=============================================================================
// PREVENTS: Incorrect spot decimal handling (MAX_DECIMALS=8 vs perps' 6)
//
// HYPERLIQUID SPOT SPECIFICS:
//   - Spot total precision is 8 decimals (perps use 6)
//   - pxDecimals = 8 - szDecimals
//   - PIP = 10^(-pxDecimals)
//   - LotAmount = 10^(-szDecimals)
//   - PIPCost = PIP * LotAmount = 10^(-8) (NOT the same as perps' 10^(-6))
//   - Asset index = 10000 + spotIndex
//   - normalizeCoin preserves '/' (spot pairs) and '@' (non-canonical)
//
// SOURCE: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/tick-and-lot-size
//   "MAX_DECIMALS is 6 for perps and 8 for spot"
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_utils.h"
#include <cmath>
#include <cstring>

using namespace hl::test;

//=============================================================================
// SPOT ASSET FORMULAS (Extracted from hl_meta_spot.cpp)
//=============================================================================

namespace SpotFormulas {

const int SPOT_MAX_DECIMALS = 8;
const int PERP_MAX_DECIMALS = 6;
const int SPOT_INDEX_BASE = 10000;

inline int pxDecimals(int szDecimals) {
    int px = SPOT_MAX_DECIMALS - szDecimals;
    return px < 0 ? 0 : px;
}

inline double pip(int szDecimals) {
    return pow(10.0, -pxDecimals(szDecimals));
}

inline double lotAmount(int szDecimals) {
    return pow(10.0, -szDecimals);
}

inline double pipCost(int szDecimals) {
    return pip(szDecimals) * lotAmount(szDecimals);
}

inline int assetIndex(int spotIndex) {
    return SPOT_INDEX_BASE + spotIndex;
}

} // namespace SpotFormulas

//=============================================================================
// TEST CASES: Spot Formulas
//=============================================================================

void test_spot_hype_calculations() {
    // HYPE/USDC: szDecimals=2 (from spotMeta)
    // pxDecimals = 8 - 2 = 6
    // LotAmount = 0.01, PIP = 0.000001
    const int szDecimals = 2;

    double pip = SpotFormulas::pip(szDecimals);
    double lotAmt = SpotFormulas::lotAmount(szDecimals);
    double pipCost = SpotFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.000001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 0.01, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.00000001, 1e-15);  // 10^(-8), NOT 10^(-6)!
}

void test_spot_btc_calculations() {
    // BTC spot: szDecimals=5 (from spotMeta tokens)
    // pxDecimals = 8 - 5 = 3
    // LotAmount = 0.00001, PIP = 0.001
    const int szDecimals = 5;

    double pip = SpotFormulas::pip(szDecimals);
    double lotAmt = SpotFormulas::lotAmount(szDecimals);
    double pipCost = SpotFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 0.00001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.00000001, 1e-15);
}

void test_spot_purr_calculations() {
    // PURR/USDC: szDecimals=0 (whole tokens only)
    // pxDecimals = 8 - 0 = 8
    // LotAmount = 1.0, PIP = 0.00000001
    const int szDecimals = 0;

    double pip = SpotFormulas::pip(szDecimals);
    double lotAmt = SpotFormulas::lotAmount(szDecimals);
    double pipCost = SpotFormulas::pipCost(szDecimals);

    ASSERT_FLOAT_EQ_TOL(pip, 0.00000001, 1e-15);
    ASSERT_FLOAT_EQ_TOL(lotAmt, 1.0, 1e-15);
    ASSERT_FLOAT_EQ_TOL(pipCost, 0.00000001, 1e-15);
}

void test_spot_pipcost_is_constant() {
    // CRITICAL: Spot PIPCost = 10^(-8) for ALL spot assets
    // This is because pxDecimals + szDecimals = 8 always (for spot)
    //
    // Note: This is 100x smaller than perps' 10^(-6)!
    // If broker code uses perp PIPCost for spot, profits will be off by 100x.

    const double EXPECTED_SPOT_PIPCOST = 0.00000001;  // 10^(-8)

    for (int sz = 0; sz <= 8; sz++) {
        double pipCost = SpotFormulas::pipCost(sz);
        ASSERT_FLOAT_EQ_TOL(pipCost, EXPECTED_SPOT_PIPCOST, 1e-15);
    }
}

void test_spot_pipcost_differs_from_perp() {
    // Spot PIPCost (10^-8) must NOT equal perp PIPCost (10^-6)
    const double PERP_PIPCOST = 0.000001;

    for (int sz = 0; sz <= 6; sz++) {
        double spotPipCost = SpotFormulas::pipCost(sz);
        ASSERT_FLOAT_NE(spotPipCost, PERP_PIPCOST);
    }
}

//=============================================================================
// TEST CASES: Asset Index Mapping
//=============================================================================

void test_spot_index_mapping() {
    // Spot assets use index = 10000 + spotIndex
    ASSERT_EQ(SpotFormulas::assetIndex(0), 10000);   // First spot pair
    ASSERT_EQ(SpotFormulas::assetIndex(1), 10001);
    ASSERT_EQ(SpotFormulas::assetIndex(107), 10107);  // @107 example

    // Must not collide with perp indices (0-999) or perpDex (110000+)
    ASSERT_GT(SpotFormulas::assetIndex(0), 999);
    ASSERT_LT(SpotFormulas::assetIndex(0), 110000);
}

void test_spot_pxdecimals_clamp() {
    // If szDecimals >= 8, pxDecimals should be 0 (not negative)
    ASSERT_EQ(SpotFormulas::pxDecimals(8), 0);
    ASSERT_EQ(SpotFormulas::pxDecimals(9), 0);   // Hypothetical edge case
    ASSERT_EQ(SpotFormulas::pxDecimals(10), 0);
}

//=============================================================================
// TEST CASES: normalizeCoin for spot symbols
//=============================================================================

void test_normalize_preserves_slash() {
    // Spot pairs like "BTC/USDC" should keep the '/' — it's the pair name
    char out[64];
    hl::utils::normalizeCoin("BTC/USDC", out, sizeof(out));
    ASSERT_STREQ(out, "BTC/USDC");
}

void test_normalize_preserves_at_format() {
    // Non-canonical spot coins use "@107" format — '@' must be preserved
    char out[64];
    hl::utils::normalizeCoin("@107", out, sizeof(out));
    ASSERT_STREQ(out, "@107");
}

void test_normalize_strips_dash_for_perps() {
    // Perp symbols like "BTC-USDC" should strip after '-' → "BTC"
    char out[64];
    hl::utils::normalizeCoin("BTC-USDC", out, sizeof(out));
    ASSERT_STREQ(out, "BTC");
}

void test_normalize_uppercases_spot() {
    // Spot pairs should be uppercased
    char out[64];
    hl::utils::normalizeCoin("hype/usdc", out, sizeof(out));
    ASSERT_STREQ(out, "HYPE/USDC");
}

void test_normalize_spot_with_dash_in_quote() {
    // Edge case: "HYPE/USDC-EXTRA" — strip after '-' but keep '/'
    // After strncpy: "HYPE/USDC-EXTRA"
    // After dash strip: "HYPE/USDC"
    char out[64];
    hl::utils::normalizeCoin("HYPE/USDC-EXTRA", out, sizeof(out));
    ASSERT_STREQ(out, "HYPE/USDC");
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  Spot Asset Formula & normalizeCoin Tests\n");
    printf("  Validates: MAX_DECIMALS=8, index=10000+spot,\n");
    printf("             '/' and '@' preservation\n");
    printf("=================================================\n\n");

    // Spot formulas
    RUN_TEST(spot_hype_calculations);
    RUN_TEST(spot_btc_calculations);
    RUN_TEST(spot_purr_calculations);
    RUN_TEST(spot_pipcost_is_constant);
    RUN_TEST(spot_pipcost_differs_from_perp);

    // Index mapping
    RUN_TEST(spot_index_mapping);
    RUN_TEST(spot_pxdecimals_clamp);

    // normalizeCoin for spot
    RUN_TEST(normalize_preserves_slash);
    RUN_TEST(normalize_preserves_at_format);
    RUN_TEST(normalize_strips_dash_for_perps);
    RUN_TEST(normalize_uppercases_spot);
    RUN_TEST(normalize_spot_with_dash_in_quote);

    return printTestSummary();
}
