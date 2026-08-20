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

    // Split at LAST underscore to extract perpDex venue suffix [OPM-169]
    // "GOLD-USDC_xyz" -> perpDex="xyz", coin="GOLD-USDC"
    // "A.B-USDC_xyz"  -> perpDex="xyz", coin="A.B-USDC" (last underscore wins)
    const char* lastUnderscore = strrchr(fullName, '_');
    if (lastUnderscore && lastUnderscore > fullName && lastUnderscore[1] != '\0') {
        // Has perpDex suffix after the underscore
        size_t coinLen = lastUnderscore - fullName;
        if (coinLen < coinSize) {
            strncpy_s(coin, coinSize, fullName, coinLen);
            coin[coinLen] = '\0';
        } else {
            strncpy_s(coin, coinSize, fullName, _TRUNCATE);
        }
        strncpy_s(perpDex, perpDexSize, lastUnderscore + 1, _TRUNCATE);
        return true;
    } else {
        // No underscore — perp, spot, or legacy bare coin
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

    // Append perpDex venue suffix if provided: "BTC-USDC" + "xyz" -> "BTC-USDC_xyz" [OPM-169]
    if (perpDex && *perpDex) {
        result += "_";
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

const char* canonicalTif(const char* tif) {
    if (!tif || !*tif) return nullptr;
    if (_stricmp(tif, "Ioc") == 0) return "Ioc";
    if (_stricmp(tif, "Gtc") == 0) return "Gtc";
    if (_stricmp(tif, "Alo") == 0) return "Alo";
    return nullptr;
}

bool isTifValidForModify(const char* tif) {
    // Narrow canonicalTif's set — never widen it, so an unparseable TIF stays
    // invalid here too.
    const char* canonical = canonicalTif(tif);
    if (!canonical) return false;
    return strcmp(canonical, "Ioc") != 0;
}

bool isExchangeOrderId(const char* oid) {
    if (!oid || !*oid) return false;
    for (const char* p = oid; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
    }
    return _atoi64(oid) > 0;
}

double exchangeOrderIdToDouble(const char* oid) {
    if (!isExchangeOrderId(oid)) return 0.0;

    // A double represents every integer up to 2^53 exactly and none above it,
    // so a larger ID would come back altered. _atoi64 also saturates at
    // _I64_MAX on an over-long digit string, which this same bound catches.
    const int64_t MAX_EXACT_IN_DOUBLE = 9007199254740992LL;  // 2^53
    int64_t id = _atoi64(oid);
    if (id > MAX_EXACT_IN_DOUBLE) return 0.0;
    return (double)id;
}

// Split "xyz:BTC-USDC" into dex="xyz", base="BTC". Spot pairs ("PURR/USDC")
// and index names ("@107") contain no dash and pass through untouched.
static void splitCoinParts(const char* coin, std::string& dex, std::string& base) {
    dex.clear();
    base.clear();
    if (!coin || !*coin) return;

    const char* colon = strchr(coin, ':');
    if (colon) {
        dex.assign(coin, colon - coin);
        base.assign(colon + 1);
    } else {
        base.assign(coin);
    }

    size_t dash = base.find('-');
    if (dash != std::string::npos) base.erase(dash);
}

bool coinMatches(const char* exchangeCoin, const char* wanted) {
    if (!exchangeCoin || !*exchangeCoin || !wanted || !*wanted) return false;

    std::string exDex, exBase, wDex, wBase;
    splitCoinParts(exchangeCoin, exDex, exBase);
    splitCoinParts(wanted, wDex, wBase);

    if (_stricmp(exDex.c_str(), wDex.c_str()) != 0) return false;
    return _stricmp(exBase.c_str(), wBase.c_str()) == 0;
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

double roundPriceForExchange(double price, int szDecimals, int maxDecimals,
                             PriceRound mode) {
    if (price == 0.0) return 0.0;

    // Guard: reject non-finite values (NaN, infinity)
    if (!isfinite(price)) return 0.0;

    // Guard: reject negative prices (exchange prices are always positive)
    if (price < 0.0) return 0.0;

    // Guard: reject prices beyond safe double precision for 5 sig-fig rounding
    if (price >= 1e15) return 0.0;

    int maxDecPlaces = maxDecimals - szDecimals;
    if (maxDecPlaces < 0) maxDecPlaces = 0;

    // [OPM-796] Grid step = the coarser of the 5-sig-fig grid and the decimal
    // limit, capped at 1 because HL always accepts integer prices. The cap is
    // what makes 123456 pass through unchanged instead of snapping to 123460,
    // and what lets BTC above $100k be quoted on the $1 grid rather than $10.
    const int sigFigs = 5;
    double magnitude = floor(log10(price));
    double step = pow(10.0, magnitude - (sigFigs - 1));
    double decimalStep = pow(10.0, -maxDecPlaces);
    if (decimalStep > step) step = decimalStep;
    if (step > 1.0) step = 1.0;

    // Snap to the grid in the requested direction. Prices already on-grid must
    // survive Down/Up untouched — 1234.6/0.1 evaluates to 12345.999999999998 in
    // binary floating point, and a naive floor() would drop a full tick.
    double q = price / step;
    double qNearest = round(q);
    double scale = (fabs(qNearest) > 1.0) ? fabs(qNearest) : 1.0;
    if (fabs(q - qNearest) <= 1e-9 * scale) {
        q = qNearest;                                  // already on-grid
    } else if (mode == PriceRound::Down) {
        q = floor(q);
    } else if (mode == PriceRound::Up) {
        q = ceil(q);
    } else {
        q = qNearest;
    }

    // Multiplying back reintroduces binary noise (e.g. 12346 * 0.1 =
    // 1234.6000000000001); clamp to the decimal budget to keep the wire string
    // canonical. Integer results are unaffected.
    double result = roundToDecimals(q * step, maxDecPlaces);

    // A positive price must never round to zero — that would put "p":"0" on the
    // wire. Reachable when the asset allows no decimal places (szDecimals >=
    // maxDecimals, forcing the integer grid) and the price is below the first
    // grid point: flooring 0.5 to the integer grid yields 0. No live perp hits
    // this today (max szDecimals across HL's 232 perps is 5, so maxDecPlaces >=
    // 1), but assets are listed continuously and the failure is silent, so we
    // clamp to the smallest representable positive price instead.
    if (result <= 0.0) return step;

    return result;
}

std::string formatPriceForExchange(double price, int szDecimals, int maxDecimals,
                                   PriceRound mode) {
    double rounded = roundPriceForExchange(price, szDecimals, maxDecimals, mode);

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
    std::string s = oss.str();

    // [OPM-677] Strip trailing zeros to match Hyperliquid's canonical form
    // (Python SDK's float_to_wire uses Decimal.normalize()). Without this,
    // sizes like 0.09730 hash differently on HL's side than locally, causing
    // signature verification to recover a phantom address and the order to
    // be rejected with "User or API Wallet does not exist".
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

std::string formatPrice(double price, int pxDecimals) {
    if (pxDecimals < 0) pxDecimals = 0;
    if (pxDecimals > 15) pxDecimals = 15;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(pxDecimals) << price;
    return oss.str();
}

} // namespace utils
} // namespace hl
