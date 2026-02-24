//=============================================================================
// hl_http.h - Stateless HTTP client for Hyperliquid API
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: hl_config.h, hl_globals.h
// THREAD SAFETY: Functions are thread-safe (use thread-local buffers)
//=============================================================================

#pragma once

#include <string>

namespace hl {
namespace http {

// =============================================================================
// RESPONSE STRUCT
// =============================================================================

/// HTTP response with status and body
struct Response {
    int statusCode;         // HTTP status code (0 if request failed)
    std::string body;       // Response body (empty on failure)
    std::string error;      // Error message (empty on success)

    /// Returns true if request succeeded (status 200-299)
    bool success() const { return statusCode >= 200 && statusCode < 300; }

    /// Returns true if request failed entirely (network error, timeout, etc.)
    bool failed() const { return statusCode == 0; }
};

// =============================================================================
// CORE HTTP FUNCTIONS
// =============================================================================

/// Send HTTP POST request
/// @param url Full URL to send request to
/// @param jsonBody JSON body to send (can be nullptr for empty body)
/// @param useSmallBuffer Use 4KB buffer instead of 1MB (for simple queries)
/// @return Response with status code and body
Response post(const char* url, const char* jsonBody, bool useSmallBuffer = false);

/// Send HTTP GET request
/// @param url Full URL to send request to
/// @param useSmallBuffer Use 4KB buffer instead of 1MB
/// @return Response with status code and body
Response get(const char* url, bool useSmallBuffer = false);

// =============================================================================
// HYPERLIQUID API ENDPOINTS
// =============================================================================

/// POST to /info endpoint (market data, account queries)
/// @param jsonBody JSON payload (e.g., {"type":"meta"})
/// @param useSmallBuffer Use 4KB buffer for simple queries
/// @return Response from info endpoint
Response infoPost(const char* jsonBody, bool useSmallBuffer = false);

/// POST to /info endpoint with perpDex support
/// @param jsonBody JSON payload (will have "dex" field injected)
/// @param perpDex perpDex name (e.g., "xyz") or empty/null for default
/// @param useSmallBuffer Use 4KB buffer for simple queries
/// @return Response from info endpoint
Response infoPostPerpDex(const char* jsonBody, const char* perpDex, bool useSmallBuffer = false);

/// POST to /exchange endpoint (order placement, cancellation)
/// @param jsonBody Signed order JSON payload
/// @return Response from exchange endpoint
Response exchangePost(const char* jsonBody);

// =============================================================================
// URL HELPERS
// =============================================================================

/// Build full URL from base URL and path
/// @param path API path (e.g., "/info" or "info")
/// @param out Output buffer for full URL
/// @param outSize Size of output buffer
void buildUrl(const char* path, char* out, size_t outSize);

// =============================================================================
// PERPDEX HELPERS
// =============================================================================

/// Parse perpDex prefix from coin name
/// Supports both underscore (preferred) and colon separators
/// @param coinWithPrefix Full coin name (e.g., "xyz_XYZ100" or "xyz:XYZ100")
/// @param perpDex Output buffer for perpDex name (e.g., "xyz")
/// @param perpDexSize Size of perpDex buffer
/// @param coin Output buffer for coin name (e.g., "XYZ100")
/// @param coinSize Size of coin buffer
/// @return true if perpDex prefix was found
bool parsePerpDex(const char* coinWithPrefix, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize);

/// Inject perpDex into JSON payload
/// @param originalPayload Original JSON (e.g., {"type":"allMids"})
/// @param perpDex perpDex name to inject (empty = no change)
/// @param outputPayload Output buffer for modified JSON
/// @param outputSize Size of output buffer
void injectPerpDex(const char* originalPayload, const char* perpDex,
                   char* outputPayload, size_t outputSize);

} // namespace http
} // namespace hl
