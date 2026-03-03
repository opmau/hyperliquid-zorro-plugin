//=============================================================================
// mock_http.h - Mock HTTP transport for service layer tests [OPM-174]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test Infrastructure
// PURPOSE: Provide controllable mock implementations of hl::http namespace
//          functions for testing service layer code that makes HTTP calls.
//
// USAGE:
//   #define MOCK_HTTP_IMPLEMENTATION
//   #include "mocks/mock_http.h"
//
// Only define MOCK_HTTP_IMPLEMENTATION in ONE .cpp file to avoid linker errors.
// Other files can include without the define to get the extern declarations.
//
// Example:
//   hl::mock::httpReset();
//   hl::mock::httpEnqueue(200, R"({"levels":[[{"px":"50000"}],[{"px":"50001"}]]})");
//   http::Response resp = http::infoPost("{\"type\":\"l2Book\"}");
//   // resp.statusCode == 200, resp.body == the JSON above
//=============================================================================

#pragma once

#include <string>
#include <vector>
#include <cstdio>

namespace hl {

// Forward-declare http::Response (matches hl_http.h)
namespace http {
struct Response {
    int statusCode = 0;
    std::string body;
    std::string error;

    bool success() const { return statusCode >= 200 && statusCode < 300; }
    bool failed() const { return statusCode == 0; }
};
} // namespace http

//=============================================================================
// MOCK CONFIGURATION
//=============================================================================

namespace mock {

/// Record of a single HTTP call made through the mock
struct HttpCall {
    std::string endpoint;   // "/info", "/exchange", etc.
    std::string body;       // Request payload
    std::string method;     // "POST", "GET"
};

/// Queued response for the mock to return
struct QueuedResponse {
    int statusCode;
    std::string body;
    std::string error;
};

/// Global mock state
struct HttpMockState {
    std::vector<QueuedResponse> responseQueue;  // FIFO response queue
    std::vector<HttpCall> calls;                // Captured calls
    int queueIndex = 0;                         // Current position in queue

    // Default response when queue is empty
    int defaultStatusCode = 200;
    std::string defaultBody = "{}";
    std::string defaultError;
};

#ifdef MOCK_HTTP_IMPLEMENTATION
HttpMockState g_httpMockState;
#else
extern HttpMockState g_httpMockState;
#endif

//-----------------------------------------------------------------------------
// Mock control functions
//-----------------------------------------------------------------------------

/// Reset all mock state (call between tests)
inline void httpReset() {
    g_httpMockState.responseQueue.clear();
    g_httpMockState.calls.clear();
    g_httpMockState.queueIndex = 0;
    g_httpMockState.defaultStatusCode = 200;
    g_httpMockState.defaultBody = "{}";
    g_httpMockState.defaultError.clear();
}

/// Enqueue a response (FIFO — first enqueued = first returned)
inline void httpEnqueue(int statusCode, const std::string& body, const std::string& error = "") {
    QueuedResponse r;
    r.statusCode = statusCode;
    r.body = body;
    r.error = error;
    g_httpMockState.responseQueue.push_back(r);
}

/// Enqueue a failure response (statusCode=0, empty body)
inline void httpEnqueueFailure(const std::string& error = "mock failure") {
    httpEnqueue(0, "", error);
}

/// Set the default response (used when queue is exhausted)
inline void httpSetDefault(int statusCode, const std::string& body) {
    g_httpMockState.defaultStatusCode = statusCode;
    g_httpMockState.defaultBody = body;
}

/// Get number of HTTP calls made
inline int httpCallCount() {
    return (int)g_httpMockState.calls.size();
}

/// Get the Nth call (0-indexed)
inline const HttpCall& httpGetCall(int index) {
    return g_httpMockState.calls[index];
}

/// Get the last call made
inline const HttpCall& httpLastCall() {
    return g_httpMockState.calls.back();
}

/// Pop next response from queue (or return default)
inline http::Response httpNextResponse() {
    http::Response resp;

    if (g_httpMockState.queueIndex < (int)g_httpMockState.responseQueue.size()) {
        const auto& queued = g_httpMockState.responseQueue[g_httpMockState.queueIndex++];
        resp.statusCode = queued.statusCode;
        resp.body = queued.body;
        resp.error = queued.error;
    } else {
        // Queue exhausted — use default
        resp.statusCode = g_httpMockState.defaultStatusCode;
        resp.body = g_httpMockState.defaultBody;
        resp.error = g_httpMockState.defaultError;
    }

    return resp;
}

} // namespace mock

//=============================================================================
// MOCK HTTP FUNCTION IMPLEMENTATIONS
//=============================================================================

namespace http {

/// Mock infoPost — records the call and returns queued response
inline Response infoPost(const char* jsonBody, bool /*useSmallBuffer*/ = false) {
    mock::HttpCall call;
    call.endpoint = "/info";
    call.body = jsonBody ? jsonBody : "";
    call.method = "POST";
    mock::g_httpMockState.calls.push_back(call);

    return mock::httpNextResponse();
}

/// Mock infoPostPerpDex — records the call and returns queued response
inline Response infoPostPerpDex(const char* jsonBody, const char* perpDex,
                                bool /*useSmallBuffer*/ = false) {
    mock::HttpCall call;
    call.endpoint = "/info";
    call.body = jsonBody ? jsonBody : "";
    if (perpDex && perpDex[0]) {
        call.body += std::string(" [perpDex=") + perpDex + "]";
    }
    call.method = "POST";
    mock::g_httpMockState.calls.push_back(call);

    return mock::httpNextResponse();
}

/// Mock exchangePost — records the call and returns queued response
inline Response exchangePost(const char* jsonBody) {
    mock::HttpCall call;
    call.endpoint = "/exchange";
    call.body = jsonBody ? jsonBody : "";
    call.method = "POST";
    mock::g_httpMockState.calls.push_back(call);

    return mock::httpNextResponse();
}

/// Mock post — records the call and returns queued response
inline Response post(const char* url, const char* jsonBody, bool /*useSmallBuffer*/ = false) {
    mock::HttpCall call;
    call.endpoint = url ? url : "";
    call.body = jsonBody ? jsonBody : "";
    call.method = "POST";
    mock::g_httpMockState.calls.push_back(call);

    return mock::httpNextResponse();
}

/// Mock get — records the call and returns queued response
inline Response get(const char* url, bool /*useSmallBuffer*/ = false) {
    mock::HttpCall call;
    call.endpoint = url ? url : "";
    call.method = "GET";
    mock::g_httpMockState.calls.push_back(call);

    return mock::httpNextResponse();
}

/// Real parsePerpDex implementation (simple string parsing, no mock needed)
inline bool parsePerpDex(const char* coinWithPrefix, char* perpDex, size_t perpDexSize,
                         char* coin, size_t coinSize) {
    if (!coinWithPrefix || !perpDex || !coin) return false;

    // Check for underscore separator first (preferred)
    const char* sep = strchr(coinWithPrefix, '_');
    if (!sep) {
        // Fall back to colon separator (legacy)
        sep = strchr(coinWithPrefix, ':');
    }

    if (sep && sep > coinWithPrefix) {
        size_t prefixLen = sep - coinWithPrefix;
        if (prefixLen < perpDexSize) {
            strncpy_s(perpDex, perpDexSize, coinWithPrefix, prefixLen);
            perpDex[prefixLen] = '\0';
        }
        strncpy_s(coin, coinSize, sep + 1, _TRUNCATE);
        return true;
    } else {
        perpDex[0] = '\0';
        strncpy_s(coin, coinSize, coinWithPrefix, _TRUNCATE);
        return false;
    }
}

/// Stub buildUrl (not needed in tests but prevents linker errors)
inline void buildUrl(const char* /*path*/, char* out, size_t outSize) {
    strncpy_s(out, outSize, "https://mock.test/", _TRUNCATE);
}

/// Stub injectPerpDex
inline void injectPerpDex(const char* originalPayload, const char* perpDex,
                          char* outputPayload, size_t outputSize) {
    if (!perpDex || perpDex[0] == '\0') {
        strncpy_s(outputPayload, outputSize, originalPayload, _TRUNCATE);
        return;
    }
    const char* closeBrace = strrchr(originalPayload, '}');
    if (!closeBrace) {
        strncpy_s(outputPayload, outputSize, originalPayload, _TRUNCATE);
        return;
    }
    size_t beforeBrace = closeBrace - originalPayload;
    char temp[2048];
    strncpy_s(temp, sizeof(temp), originalPayload, beforeBrace);
    temp[beforeBrace] = '\0';
    snprintf(outputPayload, outputSize, "%s,\"dex\":\"%s\"}", temp, perpDex);
}

} // namespace http
} // namespace hl
