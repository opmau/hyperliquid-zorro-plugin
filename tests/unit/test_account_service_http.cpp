//=============================================================================
// test_account_service_http.cpp - Account/trading service HTTP parsing [OPM-174]
//=============================================================================
// LAYER: Test | TESTS: hl_account_service, hl_trading_cancel (HTTP paths)
//
// Tests JSON parsing extracted from service functions that depend on HTTP:
//   1. spotClearinghouseState parsing (refreshSpotBalance)
//   2. userRole response parsing (checkUserRole)
//   3. orderStatus three-state parsing (queryOrderByCloid)
//
// Uses canned JSON fixtures + yyjson for deterministic parsing tests.
//=============================================================================

#include "../test_framework.h"
#include "json_helpers.h"
#include <cstring>
#include <cmath>

using namespace hl::test;

//=============================================================================
// EXTRACTED PARSING: spotClearinghouseState (from hl_account_service.cpp:170-193)
// refreshSpotBalance() parses balances array to find USDC total.
//=============================================================================

namespace AcctHttp {

struct SpotBalanceResult {
    double usdcTotal = 0.0;
    bool valid = false;
};

/// Parse spotClearinghouseState response to extract USDC balance.
/// Input: {"balances":[{"coin":"USDC","token":0,"hold":"0.0","total":"204.50",...},...]}
SpotBalanceResult parseSpotBalance(const char* json, size_t len) {
    SpotBalanceResult result;
    if (!json || len == 0) return result;

    yyjson_doc* doc = yyjson_read(json, len, 0);
    if (!doc) return result;

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* balances = hl::json::getArray(root, "balances");

    if (balances) {
        result.valid = true;  // We got a valid response, even if USDC is 0

        size_t idx, max;
        yyjson_val* item;
        yyjson_arr_foreach(balances, idx, max, item) {
            char coin[32] = {0};
            if (hl::json::getString(item, "coin", coin, sizeof(coin))) {
                if (_stricmp(coin, "USDC") == 0) {
                    result.usdcTotal = hl::json::getDouble(item, "total");
                    break;
                }
            }
        }
    }

    yyjson_doc_free(doc);
    return result;
}

//=============================================================================
// EXTRACTED PARSING: userRole (from hl_account_service.cpp:466-473)
// checkUserRole() parses the userRole response string.
//=============================================================================

enum class UserRole { Unknown, User, Agent, Vault, Subaccount, Missing };

/// Parse userRole response to determine wallet role.
/// Input is a JSON string like "\"User\"", "\"Agent\"", etc.
/// The response body contains the role type.
UserRole parseUserRole(const char* responseBody) {
    if (!responseBody || !*responseBody) return UserRole::Unknown;

    std::string body(responseBody);
    if (body.find("Agent") != std::string::npos) return UserRole::Agent;
    if (body.find("Subaccount") != std::string::npos) return UserRole::Subaccount;
    if (body.find("Vault") != std::string::npos) return UserRole::Vault;
    if (body.find("Missing") != std::string::npos) return UserRole::Missing;
    if (body.find("User") != std::string::npos) return UserRole::User;
    return UserRole::Unknown;
}

//=============================================================================
// EXTRACTED PARSING: orderStatus (from hl_trading_cancel.cpp:130-234)
// queryOrderByCloid() parses orderStatus to determine three-state outcome.
//=============================================================================

enum class QueryOutcome { Found, NotFound, Failed };

struct CloidQueryResult {
    char oid[64] = {0};
    char status[32] = {0};
    double filledSize = 0.0;
    double avgPrice = 0.0;
    QueryOutcome outcome = QueryOutcome::Failed;
};

/// Parse orderStatus response into three-state result.
/// Possible responses:
///   {"status":"unknownOid"} → NotFound
///   {"order":{"oid":12345,"status":"filled","sz":"0.001","avgPx":"60000",...}} → Found
///   {"order":{"status":"rejected"}} → NotFound
///   {"order":{"status":"open","oid":12345}} → Found
///   {"order":{"status":"canceled","oid":12345}} → Found
///   {"order":{"status":"marginCanceled"}} → NotFound
CloidQueryResult parseOrderStatus(const char* json, size_t len) {
    CloidQueryResult result;
    if (!json || len == 0) {
        strncpy_s(result.status, "invalid_input", _TRUNCATE);
        return result;
    }

    yyjson_doc* doc = yyjson_read(json, len, 0);
    if (!doc) {
        strncpy_s(result.status, "parse_error", _TRUNCATE);
        result.outcome = QueryOutcome::Failed;
        return result;
    }

    yyjson_val* root = yyjson_doc_get_root(doc);

    // Check for unknownOid — order definitely didn't land
    const char* statusStr = hl::json::getStringPtr(root, "status");
    if (statusStr && strcmp(statusStr, "unknownOid") == 0) {
        result.outcome = QueryOutcome::NotFound;
        strncpy_s(result.status, "unknownOid", _TRUNCATE);
        yyjson_doc_free(doc);
        return result;
    }

    // Navigate to order.status for the real order status
    yyjson_val* orderObj = hl::json::getObject(root, "order");
    const char* orderStatus = nullptr;
    if (orderObj) {
        orderStatus = hl::json::getStringPtr(orderObj, "status");
    }
    if (!orderStatus) orderStatus = statusStr;

    if (!orderStatus || !*orderStatus) {
        strncpy_s(result.status, "no_status", _TRUNCATE);
        result.outcome = QueryOutcome::Failed;
        yyjson_doc_free(doc);
        return result;
    }

    strncpy_s(result.status, orderStatus, _TRUNCATE);

    if (strcmp(orderStatus, "filled") == 0 ||
        strcmp(orderStatus, "open") == 0 ||
        strcmp(orderStatus, "canceled") == 0 ||
        strcmp(orderStatus, "triggered") == 0) {

        result.outcome = QueryOutcome::Found;

        // Extract oid (can be int, real, or string)
        if (orderObj) {
            yyjson_val* oidVal = yyjson_obj_get(orderObj, "oid");
            if (oidVal) {
                if (yyjson_is_int(oidVal))
                    sprintf_s(result.oid, "%lld", (long long)yyjson_get_sint(oidVal));
                else if (yyjson_is_real(oidVal))
                    sprintf_s(result.oid, "%.0f", yyjson_get_real(oidVal));
                else if (yyjson_is_str(oidVal))
                    strncpy_s(result.oid, yyjson_get_str(oidVal), _TRUNCATE);
            }
        }

        // Extract fill data for "filled" orders
        if (strcmp(orderStatus, "filled") == 0) {
            yyjson_val* orderNode = orderObj ? orderObj : root;
            result.filledSize = hl::json::getDouble(orderNode, "sz");
            result.avgPrice = hl::json::getDouble(orderNode, "avgPx");
        }

    } else if (strcmp(orderStatus, "rejected") == 0 ||
               strcmp(orderStatus, "marginCanceled") == 0) {
        result.outcome = QueryOutcome::NotFound;

    } else {
        result.outcome = QueryOutcome::Failed;
    }

    yyjson_doc_free(doc);
    return result;
}

} // namespace AcctHttp

//=============================================================================
// TEST CASES: spotClearinghouseState parsing
//=============================================================================

TEST_CASE(spot_balance_usdc_found) {
    const char* json = R"({"balances":[
        {"coin":"PURR","token":1,"hold":"0.0","total":"1000.0"},
        {"coin":"USDC","token":0,"hold":"0.0","total":"204.50"},
        {"coin":"HFUN","token":2,"hold":"0.0","total":"50.0"}
    ]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.usdcTotal, 204.50, 0.01);
}

TEST_CASE(spot_balance_usdc_first_entry) {
    const char* json = R"({"balances":[{"coin":"USDC","token":0,"hold":"0.0","total":"1500.00"}]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.usdcTotal, 1500.00, 0.01);
}

TEST_CASE(spot_balance_no_usdc) {
    const char* json = R"({"balances":[{"coin":"PURR","token":1,"total":"100.0"}]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ(r.usdcTotal, 0.0);
}

TEST_CASE(spot_balance_empty_array) {
    const char* json = R"({"balances":[]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ(r.usdcTotal, 0.0);
}

TEST_CASE(spot_balance_missing_balances_key) {
    const char* json = R"({"something":"else"})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_FALSE(r.valid);
}

TEST_CASE(spot_balance_malformed_json) {
    auto r = AcctHttp::parseSpotBalance("not json", 8);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(spot_balance_null_input) {
    auto r = AcctHttp::parseSpotBalance(nullptr, 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(spot_balance_case_insensitive_usdc) {
    // _stricmp should match case-insensitively
    const char* json = R"({"balances":[{"coin":"usdc","token":0,"total":"999.99"}]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.usdcTotal, 999.99, 0.01);
}

TEST_CASE(spot_balance_zero_usdc) {
    const char* json = R"({"balances":[{"coin":"USDC","token":0,"total":"0.0"}]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ(r.usdcTotal, 0.0);
}

TEST_CASE(spot_balance_large_amount) {
    const char* json = R"({"balances":[{"coin":"USDC","token":0,"total":"1234567.89"}]})";
    auto r = AcctHttp::parseSpotBalance(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.usdcTotal, 1234567.89, 0.01);
}

//=============================================================================
// TEST CASES: userRole parsing
//=============================================================================

TEST_CASE(user_role_user) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"User\""), (int)AcctHttp::UserRole::User);
}

TEST_CASE(user_role_agent) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"Agent\""), (int)AcctHttp::UserRole::Agent);
}

TEST_CASE(user_role_vault) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"Vault\""), (int)AcctHttp::UserRole::Vault);
}

TEST_CASE(user_role_subaccount) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"Subaccount\""), (int)AcctHttp::UserRole::Subaccount);
}

TEST_CASE(user_role_missing) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"Missing\""), (int)AcctHttp::UserRole::Missing);
}

TEST_CASE(user_role_unknown_string) {
    ASSERT_EQ((int)AcctHttp::parseUserRole("\"SomethingElse\""), (int)AcctHttp::UserRole::Unknown);
}

TEST_CASE(user_role_empty) {
    ASSERT_EQ((int)AcctHttp::parseUserRole(""), (int)AcctHttp::UserRole::Unknown);
}

TEST_CASE(user_role_null) {
    ASSERT_EQ((int)AcctHttp::parseUserRole(nullptr), (int)AcctHttp::UserRole::Unknown);
}

TEST_CASE(user_role_json_object) {
    // Some API versions might return JSON object
    ASSERT_EQ((int)AcctHttp::parseUserRole("{\"role\":\"Agent\"}"), (int)AcctHttp::UserRole::Agent);
}

//=============================================================================
// TEST CASES: orderStatus three-state parsing
//=============================================================================

TEST_CASE(order_status_unknown_oid) {
    const char* json = R"({"status":"unknownOid"})";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::NotFound);
    ASSERT_STREQ(r.status, "unknownOid");
}

TEST_CASE(order_status_filled_with_data) {
    const char* json = R"({
        "status":"order",
        "order":{
            "oid":12345678,
            "status":"filled",
            "sz":"0.001",
            "avgPx":"60000.50"
        }
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.status, "filled");
    ASSERT_STREQ(r.oid, "12345678");
    ASSERT_FLOAT_EQ_TOL(r.filledSize, 0.001, 1e-6);
    ASSERT_FLOAT_EQ_TOL(r.avgPrice, 60000.50, 0.01);
}

TEST_CASE(order_status_open) {
    const char* json = R"({
        "status":"order",
        "order":{
            "oid":99999,
            "status":"open"
        }
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.status, "open");
    ASSERT_STREQ(r.oid, "99999");
    // Open orders don't have fill data
    ASSERT_FLOAT_EQ(r.filledSize, 0.0);
    ASSERT_FLOAT_EQ(r.avgPrice, 0.0);
}

TEST_CASE(order_status_canceled) {
    const char* json = R"({
        "status":"order",
        "order":{
            "oid":55555,
            "status":"canceled"
        }
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.status, "canceled");
    ASSERT_STREQ(r.oid, "55555");
}

TEST_CASE(order_status_triggered) {
    const char* json = R"({
        "status":"order",
        "order":{
            "oid":77777,
            "status":"triggered"
        }
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.status, "triggered");
}

TEST_CASE(order_status_rejected) {
    const char* json = R"({
        "status":"order",
        "order":{"status":"rejected"}
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::NotFound);
    ASSERT_STREQ(r.status, "rejected");
}

TEST_CASE(order_status_margin_canceled) {
    const char* json = R"({
        "status":"order",
        "order":{"status":"marginCanceled"}
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::NotFound);
    ASSERT_STREQ(r.status, "marginCanceled");
}

TEST_CASE(order_status_unexpected_status) {
    const char* json = R"({
        "status":"order",
        "order":{"status":"somethingNew"}
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Failed);
    ASSERT_STREQ(r.status, "somethingNew");
}

TEST_CASE(order_status_no_order_object) {
    // Status at root level only
    const char* json = R"({"status":"filled"})";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.status, "filled");
}

TEST_CASE(order_status_no_status_field) {
    const char* json = R"({"something":"else"})";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Failed);
    ASSERT_STREQ(r.status, "no_status");
}

TEST_CASE(order_status_string_oid) {
    const char* json = R"({
        "order":{"oid":"abc123","status":"open"}
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.oid, "abc123");
}

TEST_CASE(order_status_real_oid) {
    // Some APIs return oid as floating point
    const char* json = R"({
        "order":{"oid":12345678.0,"status":"open"}
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_STREQ(r.oid, "12345678");
}

TEST_CASE(order_status_filled_partial) {
    // Partially filled order still returns "filled" status
    const char* json = R"({
        "order":{
            "oid":11111,
            "status":"filled",
            "sz":"0.005",
            "avgPx":"91234.56"
        }
    })";
    auto r = AcctHttp::parseOrderStatus(json, strlen(json));
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Found);
    ASSERT_FLOAT_EQ_TOL(r.filledSize, 0.005, 1e-6);
    ASSERT_FLOAT_EQ_TOL(r.avgPrice, 91234.56, 0.01);
}

TEST_CASE(order_status_malformed_json) {
    auto r = AcctHttp::parseOrderStatus("not json", 8);
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Failed);
    ASSERT_STREQ(r.status, "parse_error");
}

TEST_CASE(order_status_null_input) {
    auto r = AcctHttp::parseOrderStatus(nullptr, 0);
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Failed);
    ASSERT_STREQ(r.status, "invalid_input");
}

TEST_CASE(order_status_empty_input) {
    auto r = AcctHttp::parseOrderStatus("", 0);
    ASSERT_EQ((int)r.outcome, (int)AcctHttp::QueryOutcome::Failed);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-174: Account Service HTTP Parsing Tests ===\n\n");

    // spotClearinghouseState parsing
    RUN_TEST(spot_balance_usdc_found);
    RUN_TEST(spot_balance_usdc_first_entry);
    RUN_TEST(spot_balance_no_usdc);
    RUN_TEST(spot_balance_empty_array);
    RUN_TEST(spot_balance_missing_balances_key);
    RUN_TEST(spot_balance_malformed_json);
    RUN_TEST(spot_balance_null_input);
    RUN_TEST(spot_balance_case_insensitive_usdc);
    RUN_TEST(spot_balance_zero_usdc);
    RUN_TEST(spot_balance_large_amount);

    // userRole parsing
    RUN_TEST(user_role_user);
    RUN_TEST(user_role_agent);
    RUN_TEST(user_role_vault);
    RUN_TEST(user_role_subaccount);
    RUN_TEST(user_role_missing);
    RUN_TEST(user_role_unknown_string);
    RUN_TEST(user_role_empty);
    RUN_TEST(user_role_null);
    RUN_TEST(user_role_json_object);

    // orderStatus three-state parsing
    RUN_TEST(order_status_unknown_oid);
    RUN_TEST(order_status_filled_with_data);
    RUN_TEST(order_status_open);
    RUN_TEST(order_status_canceled);
    RUN_TEST(order_status_triggered);
    RUN_TEST(order_status_rejected);
    RUN_TEST(order_status_margin_canceled);
    RUN_TEST(order_status_unexpected_status);
    RUN_TEST(order_status_no_order_object);
    RUN_TEST(order_status_no_status_field);
    RUN_TEST(order_status_string_oid);
    RUN_TEST(order_status_real_oid);
    RUN_TEST(order_status_filled_partial);
    RUN_TEST(order_status_malformed_json);
    RUN_TEST(order_status_null_input);
    RUN_TEST(order_status_empty_input);

    return printTestSummary();
}
