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
#include "../services/hl_account_service.h"
#include "../services/hl_meta.h"
#include "../transport/ws_manager.h"
#include "../transport/ws_price_cache.h"

// Plugin identification
#define PLUGIN_TYPE 2
#ifdef DEV_BUILD
  #define PLUGIN_NAME "Hyperliquid-DEV"
  #define PLUGIN_VERSION "2.0.0-DEV"
#else
  #define PLUGIN_NAME "Hyperliquid"
  #define PLUGIN_VERSION "2.0.0-Modular"
#endif

#define DLLFUNC extern "C" __declspec(dllexport)

// Custom command codes
#define HL_VALIDATE_PRICES  50010
#define HL_ENABLE_WEBSOCKET 50011
#define HL_SET_ORDER_TYPE   50012
#define HL_GET_OPEN_ORDERS  50020
#define HL_SET_ACCOUNT_MODE 50021
#define HL_EXPORT_ASSETS    50001
#define HL_EXPORT_META      50002
#define HL_EXPORT_ACCOUNT   50003

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

// BrokerCommand handler (defined in hl_broker_commands.cpp)
double handleBrokerCommand(int mode, intptr_t parameter);
