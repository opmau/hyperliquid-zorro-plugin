//=============================================================================
// test_margin_actions.cpp - updateLeverage / updateIsolatedMargin signing
//=============================================================================
// PREVENTS BUGS: [OPM-1584]
//   - Field order differing from the Python SDK, so the exchange recovers a
//     different signer and rejects the action
//   - A negative ntli (margin removal) packed as an unsigned integer
//   - USD scaled by anything but 1e6, or the sign lost
//   - A noCross / strictIsolated asset read as allowing cross margin
//
// VERIFIED AGAINST:
//   - msgpack-python 1.1.2 msgpack.packb() of each action (byte vectors)
//   - hyperliquid-python-sdk 0.24.0 utils.signing.sign_l1_action() with the
//     SDK's published test key, nonce 1700000000000, no vault (signatures)
//   - Field order: SDK exchange.py update_leverage / update_isolated_margin
//=============================================================================

#include "../test_framework.h"
#include "hl_msgpack.h"
#include "hl_eip712.h"
#include "hl_crypto.h"
#include "hl_trading_margin.h"
#include <cstdio>
#include <cstring>

using namespace hl::test;
using namespace hl::msgpack;

static std::string toHex(const ByteArray& data) {
    std::string out;
    char buf[3];
    for (size_t i = 0; i < data.size(); i++) {
        sprintf_s(buf, "%02x", data[i]);
        out += buf;
    }
    return out;
}

// check-public-safety: allow private-key (published SDK signing vectors)
static const char* SDK_TEST_KEY =
    "0x0123456789012345678901234567890123456789012345678901234567890123";
static const uint64_t NONCE = 1700000000000ULL;

static std::string signPacked(const ByteArray& packed, bool isMainnet) {
    hl::eip712::ByteArray msgHash =
        hl::eip712::hashBatchModifyForSigning(packed, isMainnet, NONCE, "");
    if (msgHash.size() != 32) return "";
    hl::crypto::Signature sig;
    if (!hl::crypto::signHash(msgHash.data(), SDK_TEST_KEY, sig)) return "";
    return sig.toJson();
}

static bool has(const std::string& s, const char* needle) {
    return s.find(needle) != std::string::npos;
}

//=============================================================================
// MSGPACK BYTES (msgpack-python packb)
//=============================================================================

TEST_CASE(pack_update_leverage_main_isolated) {
    // {"type":"updateLeverage","asset":0,"isCross":False,"leverage":3}
    ByteArray p = packUpdateLeverageAction(0, false, 3);
    ASSERT_EQ(toHex(p), std::string(
        "84a474797065ae7570646174654c65766572616765a5617373657400"
        "a7697343726f7373c2a86c6576657261676503"));
}

TEST_CASE(pack_update_leverage_builder_dex_cross) {
    // {"type":"updateLeverage","asset":110021,"isCross":True,"leverage":10}
    ByteArray p = packUpdateLeverageAction(110021, true, 10);
    ASSERT_EQ(toHex(p), std::string(
        "84a474797065ae7570646174654c65766572616765a56173736574ce0001adc5"
        "a7697343726f7373c3a86c657665726167650a"));
}

TEST_CASE(pack_update_isolated_margin_add) {
    // {"type":"updateIsolatedMargin","asset":110021,"isBuy":False,"ntli":1000000000}
    ByteArray p = packUpdateIsolatedMarginAction(110021, false, 1000000000LL);
    ASSERT_EQ(toHex(p), std::string(
        "84a474797065b475706461746549736f6c617465644d617267696e"
        "a56173736574ce0001adc5a56973427579c2a46e746c69ce3b9aca00"));
}

TEST_CASE(pack_update_isolated_margin_remove_is_signed) {
    // {"type":"updateIsolatedMargin","asset":110021,"isBuy":True,"ntli":-250500000}
    // Negative ntli must be int32 (0xd2), not uint32 (0xce).
    ByteArray p = packUpdateIsolatedMarginAction(110021, true, -250500000LL);
    ASSERT_EQ(toHex(p), std::string(
        "84a474797065b475706461746549736f6c617465644d617267696e"
        "a56173736574ce0001adc5a56973427579c3a46e746c69d2f111ac60"));
}

//=============================================================================
// SIGNATURES (sign_l1_action, SDK test key, nonce 1700000000000)
//=============================================================================

TEST_CASE(sign_update_leverage_mainnet_and_testnet) {
    ByteArray p = packUpdateLeverageAction(0, false, 3);
    std::string main = signPacked(p, true);
    ASSERT_TRUE(has(main, "0x30637c567b49780978ac4498962ae7f51855e5448be98e03410ac4a1733b1189"));
    ASSERT_TRUE(has(main, "0x6b2dd735a279cf44734daf629db1a2fdd0eb93448388734675f3a317e0673fe7"));
    ASSERT_TRUE(has(main, "\"v\":27"));

    std::string test = signPacked(p, false);
    // The SDK prints r without its leading zero nibble (Python hex()).
    ASSERT_TRUE(has(test, "da58dc15c4b58e876fea27d34d011ccbc84a6397b933e2621a3d8b4977669b6"));
    ASSERT_TRUE(has(test, "0x53148c49651617ac357777607576ab18ef5b1e7501811c93f61454cef29dadb4"));
    ASSERT_TRUE(has(test, "\"v\":28"));
}

TEST_CASE(sign_update_leverage_builder_dex_cross) {
    ByteArray p = packUpdateLeverageAction(110021, true, 10);
    std::string main = signPacked(p, true);
    ASSERT_TRUE(has(main, "0x70e5a6ccfcefae64fe3e50ca2d17e7e8bd0f404e2ed9c624ee2b651d08dc7167"));
    ASSERT_TRUE(has(main, "0x10e2e00b946690a47f341616fcf2f3f8c4d935eaed6e1f655503feca738c317e"));
    ASSERT_TRUE(has(main, "\"v\":27"));
}

TEST_CASE(sign_update_isolated_margin_add) {
    ByteArray p = packUpdateIsolatedMarginAction(110021, false, 1000000000LL);
    std::string main = signPacked(p, true);
    ASSERT_TRUE(has(main, "a8e1a88c226926ef701dc8f46409d1e5c049d813e90057ea9f6db0d8a245b75"));
    ASSERT_TRUE(has(main, "0x6d2417f5b0e1a8b39d7213d0ee6972473850354b134acfb6450dd95f9ff4a401"));
    ASSERT_TRUE(has(main, "\"v\":28"));

    std::string test = signPacked(p, false);
    ASSERT_TRUE(has(test, "0x909472465ceb8ca7ef5557e51f87a253cf7e5d15aa070a074ed227cdd8ef69f9"));
    ASSERT_TRUE(has(test, "0x125f95dac48b3a0cd94f950d0da6925e2d40af63f892a8bc7d9c83fd01c9730c"));
    ASSERT_TRUE(has(test, "\"v\":27"));
}

TEST_CASE(sign_update_isolated_margin_remove) {
    ByteArray p = packUpdateIsolatedMarginAction(110021, true, -250500000LL);
    std::string main = signPacked(p, true);
    ASSERT_TRUE(has(main, "0xed46db1ded03e76b4dedf5c19d85b3f6c6b6c8d1355f813e7fd9ecc18e7293b1"));
    ASSERT_TRUE(has(main, "0x6f69fbce9bcb6155284cf58d38a09744892b519f7a59a16c42327aa71fa6df66"));
    ASSERT_TRUE(has(main, "\"v\":27"));
}

//=============================================================================
// USD -> ntli
//=============================================================================

TEST_CASE(usd_to_ntli_scales_and_keeps_sign) {
    ASSERT_EQ(hl::trading::usdToNtli(1000.0), (int64_t)1000000000LL);
    ASSERT_EQ(hl::trading::usdToNtli(-250.5), (int64_t)-250500000LL);
    ASSERT_EQ(hl::trading::usdToNtli(0.1), (int64_t)100000LL);   // not 99999
    ASSERT_EQ(hl::trading::usdToNtli(0.0000004), (int64_t)0);    // below 1e-6
}

//=============================================================================
// MARGIN MODE (meta flags as the live xyz and main dex report them)
//=============================================================================

static hl::AssetInfo assetWith(const char* marginMode, bool onlyIsolated) {
    hl::AssetInfo a;
    strncpy_s(a.marginMode, marginMode, _TRUNCATE);
    a.onlyIsolated = onlyIsolated;
    return a;
}

TEST_CASE(margin_mode_from_meta_flags) {
    using namespace hl::trading;
    ASSERT_EQ(marginModeOf(assetWith("", false)), (int)MARGIN_MODE_CROSS_ALLOWED);
    ASSERT_EQ(marginModeOf(assetWith("normal", false)), (int)MARGIN_MODE_CROSS_ALLOWED);
    ASSERT_EQ(marginModeOf(assetWith("noCross", true)), (int)MARGIN_MODE_ISOLATED_ONLY);
    ASSERT_EQ(marginModeOf(assetWith("strictIsolated", true)), (int)MARGIN_MODE_STRICT_ISOLATED);
    // Legacy flag alone cannot say which isolated mode: read as the stricter.
    ASSERT_EQ(marginModeOf(assetWith("", true)), (int)MARGIN_MODE_STRICT_ISOLATED);
    // A mode this build does not know is not guessed at.
    ASSERT_EQ(marginModeOf(assetWith("someFutureMode", false)), (int)MARGIN_MODE_UNKNOWN);
}

TEST_CASE(margin_mode_field_holds_longest_value) {
    hl::AssetInfo a = assetWith("strictIsolated", false);
    ASSERT_EQ(std::string(a.marginMode), std::string("strictIsolated"));
}

int main() {
    printf("=== Leverage / isolated margin Tests [OPM-1584] ===\n\n");

    if (!hl::crypto::init()) {
        printf("FATAL: crypto::init() failed\n");
        return 1;
    }

    // msgpack bytes
    RUN_TEST(pack_update_leverage_main_isolated);
    RUN_TEST(pack_update_leverage_builder_dex_cross);
    RUN_TEST(pack_update_isolated_margin_add);
    RUN_TEST(pack_update_isolated_margin_remove_is_signed);

    // Signing vectors (Python SDK cross-validation)
    RUN_TEST(sign_update_leverage_mainnet_and_testnet);
    RUN_TEST(sign_update_leverage_builder_dex_cross);
    RUN_TEST(sign_update_isolated_margin_add);
    RUN_TEST(sign_update_isolated_margin_remove);

    // Helpers
    RUN_TEST(usd_to_ntli_scales_and_keeps_sign);
    RUN_TEST(margin_mode_from_meta_flags);
    RUN_TEST(margin_mode_field_holds_longest_value);

    hl::crypto::cleanup();
    return printTestSummary();
}
