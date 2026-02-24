//=============================================================================
// test_framework.h - Lightweight test assertions and utilities
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test Infrastructure
// PURPOSE: Provide consistent assertion macros across all unit tests
// THREAD SAFETY: Not thread-safe (single-threaded test execution)
//=============================================================================

#pragma once

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>

namespace hl { namespace test {

//-----------------------------------------------------------------------------
// Test counters
//-----------------------------------------------------------------------------
static int g_testsPassed = 0;
static int g_testsFailed = 0;
static const char* g_currentTest = nullptr;

//-----------------------------------------------------------------------------
// Test registration
//-----------------------------------------------------------------------------
#define TEST_CASE(name) \
    void test_##name(); \
    struct TestRegistrar_##name { \
        TestRegistrar_##name() { printf("[TEST] %s\n", #name); } \
    }; \
    void test_##name()

#define RUN_TEST(name) \
    do { \
        hl::test::g_currentTest = #name; \
        printf("[TEST] %s... ", #name); \
        try { \
            test_##name(); \
            printf("OK\n"); \
            hl::test::g_testsPassed++; \
        } catch (const std::exception& e) { \
            printf("EXCEPTION: %s\n", e.what()); \
            hl::test::g_testsFailed++; \
        } \
    } while(0)

//-----------------------------------------------------------------------------
// Core assertion macros
//-----------------------------------------------------------------------------

// Basic equality
#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            printf("\n  FAILED: %s\n    %s:%d\n", #cond, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(actual, expected) \
    do { \
        if ((actual) != (expected)) { \
            printf("\n  FAILED: %s == %s\n    Expected: %s\n    Got: (see above)\n    %s:%d\n", \
                #actual, #expected, #expected, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_NE(actual, expected) \
    do { \
        if ((actual) == (expected)) { \
            printf("\n  FAILED: %s != %s (values are equal)\n    %s:%d\n", \
                #actual, #expected, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

// String equality
#define ASSERT_STREQ(actual, expected) \
    do { \
        if (strcmp((actual), (expected)) != 0) { \
            printf("\n  FAILED: strcmp(%s, %s)\n    Expected: \"%s\"\n    Got: \"%s\"\n    %s:%d\n", \
                #actual, #expected, (expected), (actual), __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_STRNE(actual, expected) \
    do { \
        if (strcmp((actual), (expected)) == 0) { \
            printf("\n  FAILED: %s should not equal \"%s\"\n    %s:%d\n", \
                #actual, (expected), __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

// Floating-point equality (with tolerance)
inline bool approxEqual(double a, double b, double epsilon = 1e-9) {
    return fabs(a - b) < epsilon;
}

#define ASSERT_FLOAT_EQ(actual, expected) \
    do { \
        double _a = (actual); \
        double _e = (expected); \
        if (!hl::test::approxEqual(_a, _e)) { \
            printf("\n  FAILED: %s ~= %s\n    Expected: %.10f\n    Got: %.10f\n    Diff: %.10e\n    %s:%d\n", \
                #actual, #expected, _e, _a, fabs(_a - _e), __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_FLOAT_EQ_TOL(actual, expected, tolerance) \
    do { \
        double _a = (actual); \
        double _e = (expected); \
        if (!hl::test::approxEqual(_a, _e, tolerance)) { \
            printf("\n  FAILED: %s ~= %s (tol=%.10e)\n    Expected: %.10f\n    Got: %.10f\n    Diff: %.10e\n    %s:%d\n", \
                #actual, #expected, (double)(tolerance), _e, _a, fabs(_a - _e), __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_FLOAT_NE(actual, unexpected) \
    do { \
        double _a = (actual); \
        double _u = (unexpected); \
        if (hl::test::approxEqual(_a, _u)) { \
            printf("\n  FAILED: %s should not equal %.10f\n    %s:%d\n", \
                #actual, _u, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

// Comparison operators
#define ASSERT_GT(actual, limit) \
    do { \
        if (!((actual) > (limit))) { \
            printf("\n  FAILED: %s > %s\n    %s:%d\n", \
                #actual, #limit, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_GE(actual, limit) \
    do { \
        if (!((actual) >= (limit))) { \
            printf("\n  FAILED: %s >= %s\n    %s:%d\n", \
                #actual, #limit, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_LT(actual, limit) \
    do { \
        if (!((actual) < (limit))) { \
            printf("\n  FAILED: %s < %s\n    %s:%d\n", \
                #actual, #limit, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_LE(actual, limit) \
    do { \
        if (!((actual) <= (limit))) { \
            printf("\n  FAILED: %s <= %s\n    %s:%d\n", \
                #actual, #limit, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

// Null pointer checks
#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            printf("\n  FAILED: %s is NULL\n    %s:%d\n", \
                #ptr, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            printf("\n  FAILED: %s should be NULL\n    %s:%d\n", \
                #ptr, __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

// Custom message
#define ASSERT_MSG(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("\n  FAILED: %s\n    Message: %s\n    %s:%d\n", \
                #cond, (msg), __FILE__, __LINE__); \
            hl::test::g_testsFailed++; \
            return; \
        } \
    } while(0)

//-----------------------------------------------------------------------------
// Test summary
//-----------------------------------------------------------------------------
inline int printTestSummary() {
    printf("\n========================================\n");
    if (g_testsFailed == 0) {
        printf("ALL TESTS PASSED (%d tests)\n", g_testsPassed);
    } else {
        printf("TESTS FAILED: %d passed, %d failed\n", g_testsPassed, g_testsFailed);
    }
    printf("========================================\n");
    return g_testsFailed > 0 ? 1 : 0;
}

}} // namespace hl::test
