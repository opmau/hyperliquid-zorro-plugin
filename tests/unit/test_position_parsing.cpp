//=============================================================================
// test_position_parsing.cpp - Multi-asset JSON position parsing validation
//=============================================================================
// PREVENTS BUG: 81db4b6
//   - JSON search found szi/entryPx from WRONG asset in multi-asset response
//   - Risk: $325k wrong delta calculated
//
// HYPERLIQUID API RESPONSE FORMAT:
// {
//   "assetPositions": [
//     {"position": {"coin": "ETH", "szi": "-3.6059", "entryPx": "2972.5"}},
//     {"position": {"coin": "BTC", "szi": "0.1234", "entryPx": "45000.0"}}
//   ]
// }
//
// The bug was: naive strstr() search for "szi" found the FIRST occurrence,
// not the one inside the correct asset's object.
//=============================================================================

#include "../test_framework.h"
#include <cstring>
#include <string>

using namespace hl::test;

//=============================================================================
// SIMULATED POSITION PARSING (extracted from legacy code pattern)
// This represents the CORRECT implementation that should pass these tests
//=============================================================================

namespace PositionParser {

// Simulates finding an asset's position in multi-asset JSON
// Returns true if found, fills outSize and outEntryPx
bool findAssetPosition(const char* json, const char* targetCoin,
                       double* outSize, double* outEntryPx) {
    if (!json || !targetCoin) return false;

    // Find the assetPositions array
    const char* arr = strstr(json, "\"assetPositions\"");
    if (!arr) return false;

    // Find "coin" key whose value matches targetCoin (whitespace-tolerant)
    // Handles both "coin":"BTC" and "coin": "BTC" formats
    const char* coinPos = nullptr;
    const char* search = arr;
    while ((search = strstr(search, "\"coin\"")) != nullptr) {
        const char* p = search + 6;  // skip past "coin"
        while (*p == ' ' || *p == '\t') p++;  // skip whitespace
        if (*p != ':') { search++; continue; }
        p++;  // skip colon
        while (*p == ' ' || *p == '\t') p++;  // skip whitespace
        if (*p != '"') { search++; continue; }
        p++;  // skip opening quote
        // Compare coin name
        size_t len = strlen(targetCoin);
        if (strncmp(p, targetCoin, len) == 0 && p[len] == '"') {
            coinPos = search;
            break;
        }
        search++;
    }
    if (!coinPos) return false;

    // Now find the position object that CONTAINS this coin
    // Search backwards for the opening brace of the position object
    const char* searchStart = arr;
    const char* positionStart = nullptr;

    // Find "position":{ that contains our coin
    const char* pos = strstr(searchStart, "\"position\"");
    while (pos && pos < coinPos) {
        positionStart = pos;
        pos = strstr(pos + 1, "\"position\"");
    }

    if (!positionStart) return false;

    // Find the closing brace of this position object
    // This is a simplified parser - real code would count braces
    const char* posEnd = strstr(coinPos, "}");
    if (!posEnd) return false;

    // Now extract szi and entryPx from WITHIN this bounded region
    // Search only between positionStart and posEnd
    std::string region(positionStart, posEnd - positionStart + 1);

    // Find szi
    const char* sziPos = strstr(region.c_str(), "\"szi\":");
    if (!sziPos) {
        // Alternative format: "szi": with space
        sziPos = strstr(region.c_str(), "\"szi\": ");
    }
    if (!sziPos) return false;

    // Skip past "szi": to get the value
    sziPos += 6;  // strlen("\"szi\":")
    while (*sziPos == ' ' || *sziPos == '"') sziPos++;
    if (outSize) *outSize = atof(sziPos);

    // Find entryPx
    const char* entryPos = strstr(region.c_str(), "\"entryPx\":");
    if (!entryPos) {
        entryPos = strstr(region.c_str(), "\"entryPx\": ");
    }
    if (!entryPos) return false;

    entryPos += 10;  // strlen("\"entryPx\":")
    while (*entryPos == ' ' || *entryPos == '"') entryPos++;
    if (outEntryPx) *outEntryPx = atof(entryPos);

    return true;
}

// WRONG implementation (simulates the bug)
// This is what caused bug 81db4b6 - it finds the FIRST occurrence
bool findAssetPosition_BUGGY(const char* json, const char* targetCoin,
                              double* outSize, double* outEntryPx) {
    // BUG: Ignores targetCoin, just finds first szi/entryPx
    (void)targetCoin;  // Unused in buggy version!

    const char* sziPos = strstr(json, "\"szi\":");
    if (!sziPos) return false;

    sziPos += 6;
    while (*sziPos == ' ' || *sziPos == '"') sziPos++;
    if (outSize) *outSize = atof(sziPos);

    const char* entryPos = strstr(json, "\"entryPx\":");
    if (!entryPos) return false;

    entryPos += 10;
    while (*entryPos == ' ' || *entryPos == '"') entryPos++;
    if (outEntryPx) *outEntryPx = atof(entryPos);

    return true;
}

} // namespace PositionParser

//=============================================================================
// TEST JSON DATA (mimics real Hyperliquid API response)
//=============================================================================

const char* MULTI_ASSET_JSON = R"({
  "assetPositions": [
    {"position": {"coin": "ETH", "szi": "-3.6059", "entryPx": "2972.5"}},
    {"position": {"coin": "BTC", "szi": "0.1234", "entryPx": "45000.0"}},
    {"position": {"coin": "SOL", "szi": "10.5", "entryPx": "125.0"}}
  ]
})";

const char* SINGLE_ASSET_JSON = R"({
  "assetPositions": [
    {"position": {"coin": "BTC", "szi": "0.5", "entryPx": "50000.0"}}
  ]
})";

const char* EMPTY_POSITIONS_JSON = R"({
  "assetPositions": []
})";

//=============================================================================
// TEST CASES
//=============================================================================

void test_correct_btc_in_multi_asset() {
    // BTC is the SECOND asset in the array
    // Correct parser should return BTC's data, not ETH's
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(MULTI_ASSET_JSON, "BTC", &size, &entryPx);

    ASSERT_TRUE(found);
    ASSERT_FLOAT_EQ_TOL(size, 0.1234, 1e-6);
    ASSERT_FLOAT_EQ_TOL(entryPx, 45000.0, 1e-6);
}

void test_correct_eth_in_multi_asset() {
    // ETH is the FIRST asset in the array
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(MULTI_ASSET_JSON, "ETH", &size, &entryPx);

    ASSERT_TRUE(found);
    ASSERT_FLOAT_EQ_TOL(size, -3.6059, 1e-6);
    ASSERT_FLOAT_EQ_TOL(entryPx, 2972.5, 1e-6);
}

void test_correct_sol_in_multi_asset() {
    // SOL is the THIRD asset in the array
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(MULTI_ASSET_JSON, "SOL", &size, &entryPx);

    ASSERT_TRUE(found);
    ASSERT_FLOAT_EQ_TOL(size, 10.5, 1e-6);
    ASSERT_FLOAT_EQ_TOL(entryPx, 125.0, 1e-6);
}

void test_buggy_parser_fails() {
    // Demonstrate that the BUGGY parser returns wrong data for BTC
    // It should return ETH's data (first in array) regardless of what we ask for
    double size, entryPx;
    bool found = PositionParser::findAssetPosition_BUGGY(MULTI_ASSET_JSON, "BTC", &size, &entryPx);

    ASSERT_TRUE(found);
    // BUGGY parser returns ETH's data (-3.6059) even when asking for BTC
    ASSERT_FLOAT_EQ_TOL(size, -3.6059, 1e-6);  // WRONG! Should be 0.1234
    // This test proves the bug exists - if this passes, the buggy pattern is broken
}

void test_buggy_vs_correct_differ() {
    // The CRITICAL test: buggy and correct parsers must return DIFFERENT values
    // for the second asset (BTC)
    double size1, entryPx1;
    double size2, entryPx2;

    PositionParser::findAssetPosition_BUGGY(MULTI_ASSET_JSON, "BTC", &size1, &entryPx1);
    PositionParser::findAssetPosition(MULTI_ASSET_JSON, "BTC", &size2, &entryPx2);

    // If these are equal, the correct parser has the same bug!
    ASSERT_FLOAT_NE(size1, size2);
}

void test_asset_not_found() {
    // Request an asset that doesn't exist
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(MULTI_ASSET_JSON, "DOGE", &size, &entryPx);

    ASSERT_FALSE(found);
}

void test_single_asset() {
    // Single asset case should work correctly
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(SINGLE_ASSET_JSON, "BTC", &size, &entryPx);

    ASSERT_TRUE(found);
    ASSERT_FLOAT_EQ_TOL(size, 0.5, 1e-6);
    ASSERT_FLOAT_EQ_TOL(entryPx, 50000.0, 1e-6);
}

void test_empty_positions() {
    // Empty positions array should return not found
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(EMPTY_POSITIONS_JSON, "BTC", &size, &entryPx);

    ASSERT_FALSE(found);
}

void test_negative_size_short_position() {
    // ETH has negative size (short position)
    double size, entryPx;
    bool found = PositionParser::findAssetPosition(MULTI_ASSET_JSON, "ETH", &size, &entryPx);

    ASSERT_TRUE(found);
    ASSERT_LT(size, 0);  // Short position = negative size
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  Multi-Asset Position Parsing Tests\n");
    printf("  Prevents bug: 81db4b6 (wrong asset data)\n");
    printf("=================================================\n\n");

    RUN_TEST(correct_btc_in_multi_asset);
    RUN_TEST(correct_eth_in_multi_asset);
    RUN_TEST(correct_sol_in_multi_asset);
    RUN_TEST(buggy_parser_fails);
    RUN_TEST(buggy_vs_correct_differ);
    RUN_TEST(asset_not_found);
    RUN_TEST(single_asset);
    RUN_TEST(empty_positions);
    RUN_TEST(negative_size_short_position);

    return printTestSummary();
}