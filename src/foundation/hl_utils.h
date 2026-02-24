//=============================================================================
// hl_utils.h - Pure utility functions (no state, no side effects)
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: None | THREAD SAFETY: All functions safe
//=============================================================================

#pragma once

#include <string>
#include <cstdint>

namespace hl {
namespace utils {

// =============================================================================
// STRING HELPERS
// =============================================================================

// Skip leading whitespace (space, tab, newline, carriage return)
const char* skipWhitespace(const char* p);

// Normalize Zorro symbol to Hyperliquid coin name
// "BTC/USD" -> "BTC", "btc-usd" -> "BTC" (uppercase, strips suffix)
void normalizeCoin(const char* symbol, char* out, size_t outSize);

// Parse perpDex venue from display name (dot-suffix format)
// "GOLD-USDC.xyz" -> perpDex="xyz", coin="GOLD-USDC", returns true
// "BTC-USDC" -> perpDex="", coin="BTC-USDC", returns false
// "BTC" -> perpDex="", coin="BTC", returns false
// Splits at LAST dot to handle hypothetical coin names with dots
bool parsePerpDex(const char* fullName, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize);

// Build display name from components (matches Hyperliquid UI convention)
// ("xyz", "GOLD", "USDC") -> "GOLD-USDC.xyz"   (perpDex)
// ("", "BTC", "USDC")     -> "BTC-USDC"         (perp)
// ("", "BTC", "")          -> "BTC"              (legacy)
std::string buildCoinName(const char* perpDex, const char* coin,
                          const char* collateral = "");

// Strip perpDex API prefix from coin name (e.g., "xyz:TSLA" -> "TSLA")
// If no colon found, copies input unchanged.
void stripApiCoinPrefix(const char* apiCoin, char* out, size_t outSize);

// Convert string to uppercase in place
void toUpperCase(char* str);

// =============================================================================
// TIME HELPERS
// =============================================================================

// Current time in milliseconds (GetTickCount wrapper, for relative timing)
uint32_t nowMs();

// Current Unix timestamp in milliseconds (for API calls)
int64_t currentTimestampMs();

// Convert Unix timestamp (seconds) to OLE DATE (Zorro's date format)
// OLE DATE: days since Dec 30, 1899 (25569 = Jan 1, 1970)
double unixToOleDate(int64_t unixSeconds);

// Convert Unix timestamp (milliseconds) to OLE DATE
double unixMsToOleDate(int64_t unixMs);

// Convert OLE DATE to Unix timestamp (seconds)
int64_t oleToUnix(double oleDate);

// Convert OLE DATE to Unix timestamp (milliseconds)
int64_t oleToUnixMs(double oleDate);

// =============================================================================
// NUMERIC HELPERS
// =============================================================================

// Round to specified decimal places
double roundToDecimals(double value, int decimals);

// Round price to tick size (e.g., 50001.23 with tick=0.1 -> 50001.2)
double roundToTickSize(double price, double tickSize);

// Round price per Hyperliquid rules: max 5 significant figures, then
// max (MAX_DECIMALS - szDecimals) decimal places. Integer prices always valid.
// Matches Python SDK: round(float(f"{px:.5g}"), 6 - szDecimals)
double roundPriceForExchange(double price, int szDecimals, int maxDecimals = 6);

// Format price rounded per exchange rules, returns string for order JSON
std::string formatPriceForExchange(double price, int szDecimals, int maxDecimals = 6);

// Format size with proper decimals for API (avoids scientific notation)
std::string formatSize(double size, int szDecimals);

// Format price with proper decimals for API
std::string formatPrice(double price, int pxDecimals);

} // namespace utils
} // namespace hl
