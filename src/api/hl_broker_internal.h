//=============================================================================
// hl_broker_internal.h - Shared preamble for API layer modules
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API (internal shared header — NOT a public API)
// PURPOSE: Eliminates duplicated Zorro include preamble across API .cpp files
//=============================================================================

#pragma once

// Windows headers FIRST (system versions, not Zorro's)
#include <windows.h>
#include <wtypes.h>  // DATE typedef (excluded by WIN32_LEAN_AND_MEAN, needed by trading.h)
#include <time.h>

// C++ STL headers BEFORE Zorro headers (prevents macro conflicts)
#include <string>
#include <vector>
#include <cstring>
#include <cmath>

// Undefine system macros that conflict with Zorro
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// Zorro headers with proper packing
#pragma pack(push,4)
#include "../../include/trading.h"
#include "../../include/variables.h"
#pragma pack(pop)

// IMMEDIATELY undef ALL conflicting macros from trading.h / variables.h
#undef and
#undef or
#undef not
#undef function
#undef PI
#undef swap
#undef setf
#undef resf
#undef isf
#undef ref
#undef MAX_ASSETS   // Zorro defines as 8000, conflicts with config::MAX_ASSETS
#undef Balance      // Zorro defines as g->vBalance, conflicts with hl::account::Balance

// Service layer includes
#include "../foundation/hl_globals.h"
#include "../foundation/hl_config.h"
#include "../foundation/hl_crypto.h"
#include "../foundation/hl_utils.h"
#include "../services/hl_market_service.h"
#include "../services/hl_trading_service.h"
#include "../services/hl_trading_response.h"
#include "../services/hl_trading_openorders.h"
#include "../services/hl_account_service.h"
#include "../services/hl_account_spot.h"
#include "../services/hl_meta.h"
#include "../transport/ws_manager.h"
#include "../transport/ws_price_cache.h"

// Plugin identification
#define PLUGIN_TYPE 2
// Keep in step with project(... VERSION ...) in CMakeLists.txt and CHANGELOG.md.
#ifdef DEV_BUILD
  #define PLUGIN_NAME "Hyperliquid-DEV"
  #define PLUGIN_VERSION "2.2.0-DEV"
#else
  #define PLUGIN_NAME "Hyperliquid"
  #define PLUGIN_VERSION "2.2.0"
#endif

// Format the running DLL's link time (defined in hl_broker.cpp). Identifies
// the loaded build where a version string cannot — see the definition for why
// a __DATE__ macro is not a substitute.
void formatBuildStamp(char* buf, size_t bufSize);

#define DLLFUNC extern "C" __declspec(dllexport)

// Custom command codes
#define HL_VALIDATE_PRICES  50010
#define HL_ENABLE_WEBSOCKET 50011
#define HL_SET_ORDER_TYPE   50012
#define HL_GET_OPEN_ORDERS  50020
#define HL_SET_ACCOUNT_MODE 50021
#define HL_GET_LAST_ORDER_ERROR 50023  // Class of last order reject; optional char[256] out [OPM-795]
#define HL_EXPORT_ASSETS    50001
#define HL_EXPORT_META      50002
#define HL_EXPORT_ACCOUNT   50003
#define HL_SET_EXPORT_NFA   50004  // NFA/compliance column for 50003's template [OPM-801]
#define HL_GET_FUNDING_RATE     50031  // Query current hourly funding rate for a coin [OPM-172]
#define HL_FORCE_WS_DISCONNECT 50030  // Debug: force WS disconnect to test reconnect [OPM-170]
#define HL_SCHEDULE_CANCEL     50032  // Dead man's switch: param=seconds from now, 0=clear [OPM-83]
#define HL_PLACE_TWAP          50040  // Place TWAP order: param=TwapRequest* [OPM-81]
#define HL_CANCEL_TWAP         50041  // Cancel TWAP order: param=twapId [OPM-81]
#define HL_MODIFY_ORDER        50042  // Atomic order modify: param=ModifyRequest* [OPM-80]
#define HL_PLACE_BRACKET       50043  // Bracket order: param=BracketRequest* [OPM-79]
#define HL_MODIFY_BY_TRADEID   50044  // Reprice by trade ID: param=double[3]
                                      // {tradeId, newPrice, newSize} [OPM-793]

// Zorro runtime function pointer (defined in hl_broker.cpp, used by BrokerAccount)
extern "C" { extern int (*nap)(int); }

// Shared state (defined in hl_broker.cpp)
extern bool g_everReceivedAccountData;
extern DWORD g_lastHttpFallbackTime;
extern const DWORD HTTP_FALLBACK_COOLDOWN_MS;

// Cross-module helpers (defined in hl_broker.cpp)
void parsePerpDex(const char* symbol, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize);
std::string buildCoinForApi(const char* perpDex, const char* coin);
void zorroQuit(const char* reason);  // Halt strategy like pressing [Stop]

// BrokerCommand handler (defined in hl_broker_commands.cpp)
double handleBrokerCommand(int mode, intptr_t parameter);

// Hyperliquid-specific command handler for the 500xx range
// (defined in hl_broker_commands_hl.cpp; called from handleBrokerCommand's
// default branch). Returns 0 for modes it does not recognize.
double handleHyperliquidCommand(int mode, intptr_t parameter);

// Position-accounting helpers shared by the execution and query trade modules
// (defined in hl_broker_trade.cpp, used by hl_broker_trade_query.cpp).
// See the definitions for the OPM-680/OPM-733 reasoning they encode.
bool hasOtherSameSideTracker(int excludeTradeId, const char* coin,
                             hl::OrderSide side);
void recordZorroClose(int tradeId, hl::OrderState& state, double closeSz);
