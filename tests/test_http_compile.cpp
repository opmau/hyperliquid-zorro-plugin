//=============================================================================
// test_http_compile.cpp - Compilation test for hl_http module
//=============================================================================
// This file tests that hl_http.h/cpp compile correctly and link properly.
// Run via compile_http_test.bat
//=============================================================================

#include <cstdio>
#include <cstring>

// Include the module under test
#include "hl_http.h"
#include "hl_globals.h"

// =============================================================================
// MOCK ZORRO SDK FUNCTIONS
// The real function pointers are set by Zorro when loading the plugin.
// For standalone testing, we provide mock implementations.
// =============================================================================

extern "C" {
    // Mock implementations for standalone testing
    static int mock_http_request(const char* Url, const char* Data, const char* Header, const char* Method) {
        printf("  [MOCK] http_request: %s %s\n", Method, Url);
        return 1;  // Return fake request ID
    }

    static int mock_http_status(int id) {
        return 1;  // Non-zero = response ready
    }

    static size_t mock_http_result(int id, char* content, size_t size) {
        const char* mockResponse = "{\"status\":\"ok\"}";
        strncpy_s(content, size, mockResponse, _TRUNCATE);
        return strlen(mockResponse);
    }

    static int mock_http_free(int id) {
        return 1;
    }

    static int mock_nap(int ms) {
        return 1;  // Always succeed
    }

    // Function pointers that hl_http.cpp expects
    int (*http_request)(const char*, const char*, const char*, const char*) = mock_http_request;
    int (*http_status)(int) = mock_http_status;
    size_t (*http_result)(int, char*, size_t) = mock_http_result;
    int (*http_free)(int) = mock_http_free;
    int (*nap)(int) = mock_nap;
}

// A programmable http_result: serves a bare `null` for the first N calls, then
// real data. Lets the null-retry path be exercised end to end.
static int s_nullsRemaining = 0;
static int s_resultCalls    = 0;

extern "C" {
    static size_t mock_http_result_scripted(int id, char* content, size_t size) {
        ++s_resultCalls;
        const char* body = "{\"status\":\"ok\"}";
        if (s_nullsRemaining > 0) {
            --s_nullsRemaining;
            body = "null";
        }
        strncpy_s(content, size, body, _TRUNCATE);
        return strlen(body);
    }
}

// =============================================================================
// TESTS
// =============================================================================

bool test_buildUrl() {
    printf("[TEST] buildUrl...\n");

    // Set base URL
    strcpy_s(hl::g_config.baseUrl, "https://api.hyperliquid.xyz");

    char url[256];
    hl::http::buildUrl("/info", url, sizeof(url));

    if (strcmp(url, "https://api.hyperliquid.xyz/info") != 0) {
        printf("  FAILED: Expected 'https://api.hyperliquid.xyz/info', got '%s'\n", url);
        return false;
    }

    // Test without leading slash
    hl::http::buildUrl("exchange", url, sizeof(url));
    if (strcmp(url, "https://api.hyperliquid.xyz/exchange") != 0) {
        printf("  FAILED: Expected 'https://api.hyperliquid.xyz/exchange', got '%s'\n", url);
        return false;
    }

    printf("  PASSED\n");
    return true;
}

bool test_parsePerpDex() {
    printf("[TEST] parsePerpDex...\n");

    char perpDex[32], coin[32];

    // Test with underscore separator
    bool result = hl::http::parsePerpDex("xyz_XYZ100", perpDex, sizeof(perpDex), coin, sizeof(coin));
    if (!result || strcmp(perpDex, "xyz") != 0 || strcmp(coin, "XYZ100") != 0) {
        printf("  FAILED: xyz_XYZ100 parse failed\n");
        return false;
    }

    // Test with colon separator (legacy)
    result = hl::http::parsePerpDex("abc:ABC200", perpDex, sizeof(perpDex), coin, sizeof(coin));
    if (!result || strcmp(perpDex, "abc") != 0 || strcmp(coin, "ABC200") != 0) {
        printf("  FAILED: abc:ABC200 parse failed\n");
        return false;
    }

    // Test without perpDex
    result = hl::http::parsePerpDex("BTC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    if (result || perpDex[0] != '\0' || strcmp(coin, "BTC") != 0) {
        printf("  FAILED: BTC parse failed\n");
        return false;
    }

    printf("  PASSED\n");
    return true;
}

bool test_injectPerpDex() {
    printf("[TEST] injectPerpDex...\n");

    char output[256];

    // Test injection
    hl::http::injectPerpDex("{\"type\":\"allMids\"}", "xyz", output, sizeof(output));
    if (strstr(output, "\"dex\":\"xyz\"") == nullptr) {
        printf("  FAILED: dex field not injected, got '%s'\n", output);
        return false;
    }

    // Test without perpDex
    hl::http::injectPerpDex("{\"type\":\"meta\"}", "", output, sizeof(output));
    if (strcmp(output, "{\"type\":\"meta\"}") != 0) {
        printf("  FAILED: Empty perpDex should not modify payload\n");
        return false;
    }

    printf("  PASSED\n");
    return true;
}

bool test_infoPost() {
    printf("[TEST] infoPost (mock)...\n");

    // Initialize globals
    strcpy_s(hl::g_config.baseUrl, "https://api.hyperliquid-testnet.xyz");
    hl::g_config.diagLevel = 0;

    auto resp = hl::http::infoPost("{\"type\":\"meta\"}", true);

    if (!resp.success()) {
        printf("  FAILED: Request failed: %s\n", resp.error.c_str());
        return false;
    }

    if (resp.body.empty()) {
        printf("  FAILED: Empty response body\n");
        return false;
    }

    printf("  PASSED (mock response: %s)\n", resp.body.c_str());
    return true;
}

bool test_Response() {
    printf("[TEST] Response struct...\n");

    hl::http::Response r1;
    r1.statusCode = 200;
    if (!r1.success()) {
        printf("  FAILED: 200 should be success\n");
        return false;
    }

    hl::http::Response r2;
    r2.statusCode = 0;
    if (!r2.failed()) {
        printf("  FAILED: 0 should be failed\n");
        return false;
    }

    printf("  PASSED\n");
    return true;
}

bool test_nullBodyRecovers() {
    printf("[TEST] null body retried, then recovers...\n");

    strcpy_s(hl::g_config.baseUrl, "https://api.hyperliquid-testnet.xyz");
    hl::g_config.diagLevel = 0;

    auto* saved = http_result;
    http_result = mock_http_result_scripted;
    s_nullsRemaining = 2;      // two nulls, then real data
    s_resultCalls = 0;

    auto resp = hl::http::infoPost("{\"type\":\"meta\"}", true);

    http_result = saved;

    if (!resp.success()) {
        printf("  FAILED: should have recovered, got error '%s'\n", resp.error.c_str());
        return false;
    }
    if (resp.body != "{\"status\":\"ok\"}") {
        printf("  FAILED: expected the recovered body, got '%s'\n", resp.body.c_str());
        return false;
    }
    if (s_resultCalls != 3) {
        printf("  FAILED: expected 3 attempts (2 null + 1 good), got %d\n", s_resultCalls);
        return false;
    }

    printf("  PASSED (recovered on attempt %d)\n", s_resultCalls);
    return true;
}

bool test_nullBodyPersistsIsFailure() {
    printf("[TEST] persistent null body reported as failure...\n");

    strcpy_s(hl::g_config.baseUrl, "https://api.hyperliquid-testnet.xyz");
    hl::g_config.diagLevel = 0;

    auto* saved = http_result;
    http_result = mock_http_result_scripted;
    s_nullsRemaining = 999;    // never recovers
    s_resultCalls = 0;

    auto resp = hl::http::infoPost("{\"type\":\"clearinghouseState\"}", true);

    http_result = saved;

    // The whole point: `null` must NOT reach the caller as a valid response,
    // or it gets parsed into zeros and read as an empty account.
    if (resp.success()) {
        printf("  FAILED: null body reported as success, body '%s'\n", resp.body.c_str());
        return false;
    }
    if (!resp.failed()) {
        printf("  FAILED: expected failed() to be true\n");
        return false;
    }
    if (s_resultCalls != hl::http::NULL_RETRY_LIMIT + 1) {
        printf("  FAILED: expected %d attempts, got %d\n",
               hl::http::NULL_RETRY_LIMIT + 1, s_resultCalls);
        return false;
    }

    printf("  PASSED (failed after %d attempts)\n", s_resultCalls);
    return true;
}

// =============================================================================
// MAIN
// =============================================================================

int main() {
    printf("\n============================================\n");
    printf("  hl_http Module Compilation Test\n");
    printf("============================================\n\n");

    // Initialize globals for testing
    hl::initGlobals();

    int passed = 0;
    int failed = 0;

    if (test_buildUrl()) passed++; else failed++;
    if (test_parsePerpDex()) passed++; else failed++;
    if (test_injectPerpDex()) passed++; else failed++;
    if (test_Response()) passed++; else failed++;
    if (test_infoPost()) passed++; else failed++;
    if (test_nullBodyRecovers()) passed++; else failed++;
    if (test_nullBodyPersistsIsFailure()) passed++; else failed++;

    printf("\n============================================\n");
    printf("  Results: %d passed, %d failed\n", passed, failed);
    printf("============================================\n\n");

    hl::cleanupGlobals();

    return failed > 0 ? 1 : 0;
}
