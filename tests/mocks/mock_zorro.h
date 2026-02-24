//=============================================================================
// mock_zorro.h - Mock implementations of Zorro SDK function pointers
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test Infrastructure
// PURPOSE: Provide mock implementations of Zorro functions for standalone testing
//
// USAGE:
//   #define MOCK_ZORRO_IMPLEMENTATION
//   #include "mocks/mock_zorro.h"
//
// Only define MOCK_ZORRO_IMPLEMENTATION in ONE .cpp file to avoid linker errors.
// Other files can include without the define to get the extern declarations.
//=============================================================================

#pragma once

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <functional>

namespace hl { namespace mock {

//-----------------------------------------------------------------------------
// Mock HTTP response configuration
// Set these before calling functions that use http_request
//-----------------------------------------------------------------------------
struct HttpMockConfig {
    std::string responseBody;   // What http_result returns
    int statusCode;             // What http_status returns (0=pending, 1+=ready)
    bool shouldFail;            // If true, http_request returns 0
    int nextRequestId;          // Incrementing request ID counter

    void reset() {
        responseBody = "{\"status\":\"ok\"}";
        statusCode = 1;
        shouldFail = false;
        nextRequestId = 1;
    }
};

// Global mock config (defined in implementation section)
#ifdef MOCK_ZORRO_IMPLEMENTATION
HttpMockConfig g_httpMock;
#else
extern HttpMockConfig g_httpMock;
#endif

}} // namespace hl::mock

//-----------------------------------------------------------------------------
// Zorro function pointer declarations
// These match the signatures expected by the plugin code
//-----------------------------------------------------------------------------

extern "C" {

#ifdef MOCK_ZORRO_IMPLEMENTATION

//=============================================================================
// HTTP MOCKS
//=============================================================================

static int mock_http_request(const char* Url, const char* Data, const char* Header, const char* Method) {
    if (hl::mock::g_httpMock.shouldFail) {
        return 0;  // Failure
    }
    return hl::mock::g_httpMock.nextRequestId++;
}

static int mock_http_status(int id) {
    return hl::mock::g_httpMock.statusCode;
}

static size_t mock_http_result(int id, char* content, size_t size) {
    const std::string& resp = hl::mock::g_httpMock.responseBody;
    size_t len = resp.size();
    if (len >= size) len = size - 1;
    memcpy(content, resp.c_str(), len);
    content[len] = '\0';
    return len;
}

static int mock_http_free(int id) {
    return 1;
}

//=============================================================================
// TIMING MOCKS
//=============================================================================

static int mock_nap(int ms) {
    // In tests, we don't actually sleep
    return 1;
}

static int mock_wait(int ms) {
    // In tests, we don't actually wait
    return 1;
}

//=============================================================================
// LOGGING MOCKS
//=============================================================================

// Captured log output for test verification
static std::string g_lastLogMessage;

static int mock_print(int to, const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    g_lastLogMessage = buffer;

    // Also print to stdout for test visibility
    printf("[LOG] %s", buffer);
    return 1;
}

static int mock_msg(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("[MSG] %s\n", buffer);
    return 1;
}

//=============================================================================
// FUNCTION POINTER DEFINITIONS
// These are the actual function pointers that plugin code uses
//=============================================================================

int (*http_request)(const char*, const char*, const char*, const char*) = mock_http_request;
int (*http_status)(int) = mock_http_status;
size_t (*http_result)(int, char*, size_t) = mock_http_result;
int (*http_free)(int) = mock_http_free;
int (*nap)(int) = mock_nap;
int (*wait)(int) = mock_wait;

// Note: print/msg have variable args, so we declare them differently
// For most tests, the plugin's BrokerError/logInfo functions are mocked instead

#else // !MOCK_ZORRO_IMPLEMENTATION

//=============================================================================
// EXTERN DECLARATIONS (for header-only includes)
//=============================================================================

extern int (*http_request)(const char*, const char*, const char*, const char*);
extern int (*http_status)(int);
extern size_t (*http_result)(int, char*, size_t);
extern int (*http_free)(int);
extern int (*nap)(int);
extern int (*wait)(int);

#endif // MOCK_ZORRO_IMPLEMENTATION

} // extern "C"

//-----------------------------------------------------------------------------
// Helper functions for tests
//-----------------------------------------------------------------------------

namespace hl { namespace mock {

inline void resetMocks() {
#ifdef MOCK_ZORRO_IMPLEMENTATION
    g_httpMock.reset();
    g_lastLogMessage.clear();
#endif
}

inline void setHttpResponse(const std::string& body, int status = 1) {
#ifdef MOCK_ZORRO_IMPLEMENTATION
    g_httpMock.responseBody = body;
    g_httpMock.statusCode = status;
#endif
}

inline void setHttpFailure(bool shouldFail) {
#ifdef MOCK_ZORRO_IMPLEMENTATION
    g_httpMock.shouldFail = shouldFail;
#endif
}

}} // namespace hl::mock
