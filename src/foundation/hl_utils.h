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
// "BTC-USD" -> "BTC" (strips dash suffix, uppercase)
// "HYPE/USDC" -> "HYPE/USDC" (preserves slash for spot)
// "@107" -> "@107" (preserves @-format for spot)
void normalizeCoin(const char* symbol, char* out, size_t outSize);

// Canonicalize a time-in-force string to the exact casing Hyperliquid signs
// with. [OPM-794] The msgpack signing path normalizes ("ioc" -> "Ioc") but the
// JSON payload does not, so a non-canonical string produces a JSON/signature
// mismatch that HL reports as "User or API Wallet does not exist".
// Returns "Ioc" / "Gtc" / "Alo", or nullptr when the input is not a valid TIF.
const char* canonicalTif(const char* tif);

// Is this a real exchange order ID (a positive decimal integer)? [OPM-797]
// The tradeMap also holds synthetic IDs — "PENDING_<cloid>", "RESUMED_<n>",
// "IMPORTED_<n>", "DRY_RUN" — which _atoi64 silently turns into 0, so a cancel
// built from one submits a well-formed cancel for oid 0 and reports success.
bool isExchangeOrderId(const char* oid);

// Lenient coin comparison across the plugin's three spellings of an asset:
// the exchange's ("BTC", "xyz:TSLA"), Zorro's display name ("BTC-USDC_xyz")
// and the API form buildCoinForApi() derives from it ("xyz:BTC-USDC").
// The perpDex prefix must agree; the collateral suffix is ignored.
//   coinMatches("xyz:BTC-USDC", "xyz:BTC") -> true
//   coinMatches("BTC", "xyz:BTC")          -> false (different venue)
bool coinMatches(const char* exchangeCoin, const char* wanted);

// Parse perpDex venue from display name (underscore-suffix format) [OPM-169]
// "GOLD-USDC_xyz" -> perpDex="xyz", coin="GOLD-USDC", returns true
// "BTC-USDC" -> perpDex="", coin="BTC-USDC", returns false
// "BTC" -> perpDex="", coin="BTC", returns false
// Splits at LAST underscore to extract perpDex venue suffix
bool parsePerpDex(const char* fullName, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize);

// Build display name from components [OPM-169: underscore separator for Zorro history files]
// ("xyz", "GOLD", "USDC") -> "GOLD-USDC_xyz"   (perpDex)
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

// Rounding direction for exchange price snapping. [OPM-796]
// Passive (maker) orders must never be rounded ACROSS the spread, or the
// exchange rejects them as post-only-would-match. Buy limits round Down,
// sell limits round Up; both directions are strictly less aggressive than the
// requested price, so they are also safe for IOC/market orders.
enum class PriceRound { Nearest, Down, Up };

// Round price to the nearest valid Hyperliquid price. [OPM-796]
//
// Valid prices are multiples of the 5-significant-figure grid step
// 10^(floor(log10(px)) - 4) that also fit within (MAX_DECIMALS - szDecimals)
// decimal places — PLUS every integer, which HL accepts regardless of
// significant figures ("123456 is a valid price even though 12345.6 is not",
// Hyperliquid tick-and-lot-size docs). Above 100000 the sig-fig grid
// is coarser than 1, so the integer grid is both valid and finer and we snap
// to it — that is what makes $1-level maker quotes on BTC expressible.
double roundPriceForExchange(double price, int szDecimals, int maxDecimals = 6,
                             PriceRound mode = PriceRound::Nearest);

// Format price rounded per exchange rules, returns string for order JSON
std::string formatPriceForExchange(double price, int szDecimals, int maxDecimals = 6,
                                   PriceRound mode = PriceRound::Nearest);

// Format size with proper decimals for API (avoids scientific notation)
std::string formatSize(double size, int szDecimals);

// Format price with proper decimals for API
std::string formatPrice(double price, int pxDecimals);

} // namespace utils
} // namespace hl
