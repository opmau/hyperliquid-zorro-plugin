//=============================================================================
// hl_utils.cpp - Pure utility functions implementation
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: hl_utils.h
//=============================================================================

#include "hl_utils.h"
#include <windows.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace hl {
namespace utils {

// =============================================================================
// STRING HELPERS
// =============================================================================

const char* skipWhitespace(const char* p) {
    while (p && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) ++p;
    return p;
}

void normalizeCoin(const char* symbol, char* out, size_t outSize) {
    if (!symbol || !*symbol || !out || outSize == 0) {
        if (out && outSize > 0) *out = 0;
        return;
    }

    strncpy_s(out, outSize, symbol, _TRUNCATE);

    // Strip perp suffix after '-' (e.g., BTC-USDC -> BTC)
    // Preserve '/' for spot pairs (e.g., BTC/USDC stays BTC/USDC)
    // Preserve '@' for spot @-format (e.g., @107 stays @107)
    char* dash = strchr(out, '-');
    if (dash) *dash = 0;

    // Uppercase (safe for @107 — @ is unaffected)
    toUpperCase(out);
}

bool parsePerpDex(const char* fullName, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize) {
    if (!fullName || !perpDex || !coin) return false;

    // Split at LAST dot to extract perpDex venue suffix
    // "GOLD-USDC.xyz" -> perpDex="xyz", coin="GOLD-USDC"
    // "A.B-USDC.xyz"  -> perpDex="xyz", coin="A.B-USDC" (last dot wins)
    const char* lastDot = strrchr(fullName, '.');
    if (lastDot && lastDot > fullName && lastDot[1] != '\0') {
        // Has perpDex suffix after the dot
        size_t coinLen = lastDot - fullName;
        if (coinLen < coinSize) {
            strncpy_s(coin, coinSize, fullName, coinLen);
            coin[coinLen] = '\0';
        } else {
            strncpy_s(coin, coinSize, fullName, _TRUNCATE);
        }
        strncpy_s(perpDex, perpDexSize, lastDot + 1, _TRUNCATE);
        return true;
    } else {
        // No dot — perp, spot, or legacy bare coin
        perpDex[0] = '\0';
        strncpy_s(coin, coinSize, fullName, _TRUNCATE);
        return false;
    }
}

std::string buildCoinName(const char* perpDex, const char* coin,
                          const char* collateral) {
    if (!coin || !*coin) return "";
    std::string result(coin);

    // Append collateral suffix if provided: "BTC" + "USDC" -> "BTC-USDC"
    if (collateral && *collateral) {
        result += "-";
        result += collateral;
    }

    // Append perpDex venue suffix if provided: "BTC-USDC" + "xyz" -> "BTC-USDC.xyz"
    if (perpDex && *perpDex) {
        result += ".";
        result += perpDex;
    }

    return result;
}

void stripApiCoinPrefix(const char* apiCoin, char* out, size_t outSize) {
    if (!apiCoin || !out || outSize == 0) {
        if (out && outSize > 0) out[0] = 0;
        return;
    }
    // PerpDex API returns coin names with venue prefix: "xyz:TSLA"
    // Strip everything up to and including the colon.
    const char* colon = strchr(apiCoin, ':');
    const char* src = colon ? colon + 1 : apiCoin;
    strncpy_s(out, outSize, src, _TRUNCATE);
}

void toUpperCase(char* str) {
    if (!str) return;
    for (char* c = str; *c; ++c) {
        if (*c >= 'a' && *c <= 'z') {
            *c = static_cast<char>(*c - 'a' + 'A');
        }
    }
}

// =============================================================================
// TIME HELPERS
// =============================================================================

uint32_t nowMs() {
    return GetTickCount();
}

int64_t currentTimestampMs() {
    return static_cast<int64_t>(time(nullptr)) * 1000;
}

double unixToOleDate(int64_t unixSeconds) {
    // OLE DATE: days since Dec 30, 1899
    // Unix epoch (Jan 1, 1970) = OLE day 25569
    return 25569.0 + static_cast<double>(unixSeconds) / 86400.0;
}

double unixMsToOleDate(int64_t unixMs) {
    return 25569.0 + static_cast<double>(unixMs) / 1000.0 / 86400.0;
}

int64_t oleToUnix(double oleDate) {
    return static_cast<int64_t>((oleDate - 25569.0) * 86400.0);
}

int64_t oleToUnixMs(double oleDate) {
    return static_cast<int64_t>((oleDate - 25569.0) * 86400.0 * 1000.0);
}

// =============================================================================
// NUMERIC HELPERS
// =============================================================================

double roundToDecimals(double value, int decimals) {
    if (decimals < 0) decimals = 0;
    if (decimals > 15) decimals = 15;
    double multiplier = pow(10.0, decimals);
    return round(value * multiplier) / multiplier;
}

double roundToTickSize(double price, double tickSize) {
    if (tickSize <= 0) return price;
    return round(price / tickSize) * tickSize;
}

double roundPriceForExchange(double price, int szDecimals, int maxDecimals) {
    if (price == 0.0) return 0.0;

    // Guard: reject non-finite values (NaN, infinity)
    if (!isfinite(price)) return 0.0;

    // Guard: reject negative prices (exchange prices are always positive)
    if (price < 0.0) return 0.0;

    // Guard: reject prices beyond safe double precision for 5 sig-fig rounding
    if (price >= 1e15) return 0.0;

    // Step 1: Round to 5 significant figures (matches Python f"{px:.5g}")
    const int sigFigs = 5;
    double magnitude = floor(log10(fabs(price)));
    double factor = pow(10.0, sigFigs - 1 - magnitude);
    double rounded = round(price * factor) / factor;

    // Step 2: Round to max allowed decimal places (MAX_DECIMALS - szDecimals)
    int maxDecPlaces = maxDecimals - szDecimals;
    if (maxDecPlaces < 0) maxDecPlaces = 0;
    rounded = roundToDecimals(rounded, maxDecPlaces);

    return rounded;
}

std::string formatPriceForExchange(double price, int szDecimals, int maxDecimals) {
    double rounded = roundPriceForExchange(price, szDecimals, maxDecimals);

    // Format with the max allowed decimal places, then strip trailing zeros
    int maxDecPlaces = maxDecimals - szDecimals;
    if (maxDecPlaces < 0) maxDecPlaces = 0;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(maxDecPlaces) << rounded;
    std::string s = oss.str();

    // Strip trailing zeros after decimal point
    size_t dot = s.find('.');
    if (dot != std::string::npos) {
        size_t end = s.find_last_not_of('0');
        if (end != std::string::npos)
            s = s.substr(0, end + 1);
        if (!s.empty() && s.back() == '.')
            s.pop_back();
    }
    return s;
}

std::string formatSize(double size, int szDecimals) {
    if (szDecimals < 0) szDecimals = 0;
    if (szDecimals > 15) szDecimals = 15;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(szDecimals) << size;
    return oss.str();
}

std::string formatPrice(double price, int pxDecimals) {
    if (pxDecimals < 0) pxDecimals = 0;
    if (pxDecimals > 15) pxDecimals = 15;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(pxDecimals) << price;
    return oss.str();
}

} // namespace utils
} // namespace hl
