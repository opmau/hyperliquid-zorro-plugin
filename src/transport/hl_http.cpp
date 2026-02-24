//=============================================================================
// hl_http.cpp - Stateless HTTP client implementation
//=============================================================================
// LAYER: Transport
// DEPENDENCIES: hl_globals.h, Zorro SDK (functions.h)
//=============================================================================

#include "hl_http.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_config.h"

#include <cstring>
#include <cstdio>

// Windows headers
#include <windows.h>

// =============================================================================
// ZORRO SDK HTTP FUNCTIONS (declared as extern function pointers)
// These are set by Zorro when the plugin loads via functions.h
// =============================================================================

extern "C" {
    extern int (*http_request)(const char* Url, const char* Data, const char* Header, const char* Method);
    extern int (*http_status)(int id);
    extern size_t (*http_result)(int id, char* content, size_t size);
    extern int (*http_free)(int id);
    extern int (*nap)(int milliseconds);  // Non-blocking sleep
}

namespace hl {
namespace http {

// =============================================================================
// INTERNAL HELPERS
// =============================================================================

// Static buffers for HTTP responses (thread-local would be better, but matches legacy)
static char s_largeBuffer[1024 * 1024];  // 1MB for large responses
static char s_smallBuffer[4096];          // 4KB for simple queries

// HTTP headers for all requests
static const char* HTTP_HEADERS =
    "Content-Type: application/json\r\n"
    "Accept: application/json\r\n"
    "Connection: close\r\n"
    "User-Agent: Zorro-Hyperliquid/1.1";

// =============================================================================
// URL HELPERS
// =============================================================================

void buildUrl(const char* path, char* out, size_t outSize) {
    const char* baseUrl = g_config.baseUrl;
    if (!baseUrl[0]) {
        baseUrl = config::TESTNET_REST;  // Safe default
    }

    // Skip leading slash to avoid double slash
    if (path && path[0] == '/') {
        path++;
    }

    snprintf(out, outSize, "%s/%s", baseUrl, path ? path : "");
}

// =============================================================================
// PERPDEX HELPERS
// =============================================================================

bool parsePerpDex(const char* coinWithPrefix, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize) {
    if (!coinWithPrefix || !perpDex || !coin) return false;

    // Check for underscore separator first (preferred)
    const char* sep = strchr(coinWithPrefix, '_');
    if (!sep) {
        // Fall back to colon separator (legacy)
        sep = strchr(coinWithPrefix, ':');
    }

    if (sep && sep > coinWithPrefix) {
        // Has perpDex prefix
        size_t prefixLen = sep - coinWithPrefix;
        if (prefixLen < perpDexSize) {
            strncpy_s(perpDex, perpDexSize, coinWithPrefix, prefixLen);
            perpDex[prefixLen] = '\0';
        }
        strncpy_s(coin, coinSize, sep + 1, _TRUNCATE);
        return true;
    } else {
        // No perpDex prefix
        perpDex[0] = '\0';
        strncpy_s(coin, coinSize, coinWithPrefix, _TRUNCATE);
        return false;
    }
}

void injectPerpDex(const char* originalPayload, const char* perpDex,
                   char* outputPayload, size_t outputSize) {
    if (!perpDex || perpDex[0] == '\0') {
        // No perpDex, use original payload
        strncpy_s(outputPayload, outputSize, originalPayload, _TRUNCATE);
        return;
    }

    // Find the closing brace
    const char* closeBrace = strrchr(originalPayload, '}');
    if (!closeBrace) {
        strncpy_s(outputPayload, outputSize, originalPayload, _TRUNCATE);
        return;
    }

    // Calculate position
    size_t beforeBrace = closeBrace - originalPayload;
    size_t perpLen = strlen(perpDex);
    size_t neededSize = beforeBrace + perpLen + 20;  // Space for ,"dex":""}

    if (neededSize > outputSize || beforeBrace >= 2048) {
        // Payload too large, just copy original
        strncpy_s(outputPayload, outputSize, originalPayload, _TRUNCATE);
        return;
    }

    char temp[2048];
    strncpy_s(temp, sizeof(temp), originalPayload, beforeBrace);
    temp[beforeBrace] = '\0';

    // Add dex field (perpDex API uses "dex", not "perpDex")
    snprintf(outputPayload, outputSize, "%s,\"dex\":\"%s\"}", temp, perpDex);
}

// =============================================================================
// CORE HTTP IMPLEMENTATION
// =============================================================================

// Result codes for the low-level HTTP operation
enum HttpResultCode {
    HTTP_OK = 0,
    HTTP_REQUEST_FAILED = 1,
    HTTP_TIMEOUT = 2,
    HTTP_ABORTED = 3,
    HTTP_EMPTY_RESPONSE = 4
};

// Low-level HTTP send with SEH exception handling
// This function is separate because __try/__except cannot be used in functions
// that have C++ objects requiring destructors (like std::string)
static HttpResultCode sendHttpRaw(const char* fullUrl, const char* body,
                                   const char* method, char* buffer,
                                   size_t bufferSize, size_t* outResultSize) {
    *outResultSize = 0;

    // Send request with exception handling (Zorro functions can throw SEH exceptions)
    int requestId = 0;
    __try {
        requestId = http_request(fullUrl, body ? body : nullptr, HTTP_HEADERS, method);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        requestId = 0;
    }

    if (!requestId) {
        return HTTP_REQUEST_FAILED;
    }

    // Wait for response (~30 second timeout with 10ms intervals)
    int waitCount = 3000;
    int size = 0;
    while (waitCount > 0) {
        __try {
            size = http_status(requestId);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            size = 0;
        }

        if (size != 0) break;

        // Non-blocking sleep (allows Zorro message processing)
        int napResult = 0;
        __try {
            napResult = nap(10);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            napResult = 0;
        }

        if (!napResult) {
            // nap returned false - abort requested
            __try { http_free(requestId); } __except(EXCEPTION_EXECUTE_HANDLER) {}
            return HTTP_ABORTED;
        }

        waitCount--;
    }

    if (size == 0) {
        __try { http_free(requestId); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        return HTTP_TIMEOUT;
    }

    // Get response body
    size_t resultSize = 0;
    __try {
        resultSize = http_result(requestId, buffer, bufferSize);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        resultSize = 0;
    }

    // Free request resources
    __try { http_free(requestId); } __except(EXCEPTION_EXECUTE_HANDLER) {}

    if (resultSize == 0) {
        return HTTP_EMPTY_RESPONSE;
    }

    // Ensure null termination
    buffer[bufferSize - 1] = '\0';
    *outResultSize = resultSize;

    return HTTP_OK;
}

// High-level HTTP send that returns a Response struct
static Response sendHttpInternal(const char* fullUrl, const char* body,
                                  const char* method, bool useSmallBuffer) {
    Response resp;
    resp.statusCode = 0;

    // Select buffer
    char* buffer = useSmallBuffer ? s_smallBuffer : s_largeBuffer;
    size_t bufferSize = useSmallBuffer ? sizeof(s_smallBuffer) : sizeof(s_largeBuffer);

    // Log request at diag level 2
    if (g_config.diagLevel >= 2) {
        g_logger.logf(2, "HTTP Send: %s", fullUrl);
    }

    // Call the low-level function (which has SEH handling)
    size_t resultSize = 0;
    HttpResultCode result = sendHttpRaw(fullUrl, body, method, buffer, bufferSize, &resultSize);

    // Handle result
    switch (result) {
        case HTTP_OK:
            resp.statusCode = 200;  // Zorro doesn't expose actual status codes
            resp.body = buffer;
            break;

        case HTTP_REQUEST_FAILED:
            resp.error = "http_request failed";
            if (g_config.diagLevel >= 1) {
                g_logger.logf(1, "HTTP failed: %s", fullUrl);
            }
            return resp;

        case HTTP_TIMEOUT:
            resp.error = "Request timeout";
            if (g_config.diagLevel >= 1) {
                g_logger.logf(1, "HTTP timeout: %s", fullUrl);
            }
            return resp;

        case HTTP_ABORTED:
            resp.error = "Request aborted";
            return resp;

        case HTTP_EMPTY_RESPONSE:
            resp.error = "Empty response";
            if (g_config.diagLevel >= 1) {
                g_logger.logf(1, "HTTP empty response: %s", fullUrl);
            }
            return resp;
    }

    // Log response at diag level 2
    if (g_config.diagLevel >= 2) {
        // Truncate for logging
        char snippet[2048];
        strncpy_s(snippet, buffer, sizeof(snippet) - 5);
        snippet[sizeof(snippet) - 5] = '\0';
        size_t len = strlen(snippet);
        if (buffer[len] != '\0') {
            strcat_s(snippet, sizeof(snippet), "...");
        }
        g_logger.logf(2, "HTTP Resp: %s", snippet);
        g_logger.logf(2, "HTTP Response length: %zu bytes", strlen(buffer));
    }

    return resp;
}

// =============================================================================
// PUBLIC API
// =============================================================================

Response post(const char* url, const char* jsonBody, bool useSmallBuffer) {
    return sendHttpInternal(url, jsonBody, "POST", useSmallBuffer);
}

Response get(const char* url, bool useSmallBuffer) {
    return sendHttpInternal(url, nullptr, "GET", useSmallBuffer);
}

Response infoPost(const char* jsonBody, bool useSmallBuffer) {
    char url[512];
    buildUrl("/info", url, sizeof(url));
    return sendHttpInternal(url, jsonBody, "POST", useSmallBuffer);
}

Response infoPostPerpDex(const char* jsonBody, const char* perpDex, bool useSmallBuffer) {
    char modifiedPayload[2048];
    injectPerpDex(jsonBody, perpDex, modifiedPayload, sizeof(modifiedPayload));

    // Log perpDex API payload at Diag >= 3
    if (g_config.diagLevel >= 3) {
        g_logger.logf(3, "perpDex API payload: %s", modifiedPayload);
    }

    return infoPost(modifiedPayload, useSmallBuffer);
}

Response exchangePost(const char* jsonBody) {
    char url[512];
    buildUrl("/exchange", url, sizeof(url));
    return sendHttpInternal(url, jsonBody, "POST", false);  // Always use large buffer
}

} // namespace http
} // namespace hl
