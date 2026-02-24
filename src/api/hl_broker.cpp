//=============================================================================
// hl_broker.cpp - Zorro Broker API lifecycle and infrastructure
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API (top layer - only file that includes Zorro SDK)
// DEPENDENCIES: All service layer modules
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// This module provides:
// - zorro() function pointer initialization
// - BrokerOpen/BrokerLogin lifecycle
// - BrokerTime connection status
// - BrokerCommand dispatch (to hl_broker_commands.cpp)
// - WS callback bridges (onFillNotify, onOrderUpdate)
// - Shared helpers (parsePerpDex, buildCoinForApi)
//
// Data queries: hl_broker_market.cpp (BrokerAsset, BrokerAccount, BrokerHistory2)
// Order execution: hl_broker_trade.cpp (BrokerBuy2, BrokerSell2, BrokerTrade)
// Extended commands: hl_broker_commands.cpp (handleBrokerCommand)
//=============================================================================

#include "hl_broker_internal.h"

// Define Zorro runtime function pointers.
// These are populated by the zorro() export function from GLOBALS->Functions[].
// Transport layer (hl_http.cpp) references them as extern "C".
extern "C" {
    int (*http_request)(const char*, const char*, const char*, const char*) = nullptr;
    int (*http_status)(int) = nullptr;
    size_t (*http_result)(int, char*, size_t) = nullptr;
    int (*http_free)(int) = nullptr;
    int (*nap)(int) = nullptr;
}

// Global pointer to Zorro's GLOBALS struct
static GLOBALS* g_zorroGlobals = nullptr;

// Zorro callbacks (set by BrokerOpen)
static int (__cdecl *BrokerMessage)(const char* text) = nullptr;
static int (__cdecl *BrokerProgress)(intptr_t progress) = nullptr;

// Shared state (extern-declared in hl_broker_internal.h)
bool g_everReceivedAccountData = false;
DWORD g_lastHttpFallbackTime = 0;
const DWORD HTTP_FALLBACK_COOLDOWN_MS = 30000;  // 30 seconds

//=============================================================================
// zorro() - Zorro function pointer initialization (called BEFORE BrokerOpen)
//=============================================================================
// Zorro calls this to pass its GLOBALS struct containing Functions[] array.
// We extract the function pointers our HTTP transport layer needs.
//
// Index derivation: include/func_list.h, index = line_number - 3
// Cross-verified against inline comments in func_list.h (e.g., "//142")
//
// CRITICAL: Without this, http_request/http_status/nap stay nullptr,
// every HTTP call fails silently (SEH catch), meta cache is empty.
//=============================================================================
DLLFUNC int zorro(GLOBALS* Globals) {
    g_zorroGlobals = Globals;

    // wait() at func_list.h line 16 → index 13. Used as nap() for non-blocking sleep.
    nap          = (int(*)(int))                                             Globals->Functions[13];
    // http_status at func_list.h line 145 → index 142 (verified: comment "//142")
    http_status  = (int(*)(int))                                             Globals->Functions[142];
    // http_result at func_list.h line 146 → index 143 (verified: comment "//143")
    http_result  = (size_t(*)(int,char*,size_t))                             Globals->Functions[143];
    // http_free at func_list.h line 147 → index 144 (verified: comment "//144")
    http_free    = (int(*)(int))                                             Globals->Functions[144];
    // http_request at func_list.h line 657 → index 654 (verified: comment "// 654")
    http_request = (int(*)(const char*,const char*,const char*,const char*)) Globals->Functions[654];

    return SCRIPT_VERSION;
}

// Zorro callback wrapper for logging
static int zorroLogCallback(const char* msg) {
    if (BrokerMessage) return BrokerMessage(msg);
    return 0;
}

// WS userFills → TradeMap bridge [OPM-87]
// Called on WS connection thread; trading functions use critical sections.
// Receives per-OID cumulative fill totals computed from PriceCache.
static void onFillNotify(const char* oid, double totalFilledSz, double avgFillPx) {
    int tradeId = hl::trading::findTradeIdByOid(oid);
    if (tradeId <= 0) return;

    hl::OrderState state;
    if (!hl::trading::getOrder(tradeId, state)) return;

    if (totalFilledSz < state.filledSize) return;

    hl::OrderStatus newStatus = hl::determineFilledStatus(totalFilledSz, state.requestedSize);

    hl::trading::updateOrder(tradeId, totalFilledSz, avgFillPx, newStatus);
}

// WS orderUpdates → TradeMap bridge [OPM-86]
// Called on WS connection thread; trading functions use critical sections.
static void onOrderUpdate(const char* oid, const char* cloid,
                           const char* status, double filledSz, double avgPx) {
    int tradeId = 0;
    if (cloid) tradeId = hl::trading::findTradeIdByCloid(cloid);
    if (tradeId == 0 && oid) tradeId = hl::trading::findTradeIdByOid(oid);
    if (tradeId == 0) return;

    hl::OrderState state;
    hl::trading::getOrder(tradeId, state);

    hl::OrderStatus orderStatus = hl::OrderStatus::Open;
    if (status) {
        if (strcmp(status, "filled") == 0)
            orderStatus = hl::determineFilledStatus(filledSz, state.requestedSize);
        else if (strstr(status, "canceled") || strstr(status, "Canceled"))
            orderStatus = hl::OrderStatus::Cancelled;
        else if (strcmp(status, "rejected") == 0 || strstr(status, "Rejected"))
            orderStatus = hl::OrderStatus::Error;
        else if (filledSz > 0 && state.requestedSize > 0)
            orderStatus = hl::determineFilledStatus(filledSz, state.requestedSize);
    }

    hl::trading::updateOrder(tradeId, filledSz, avgPx, orderStatus);
}

//=============================================================================
// HELPER FUNCTIONS (shared with hl_broker_market.cpp, hl_broker_trade.cpp)
//=============================================================================

// Parse Zorro symbol into perpDex venue and API coin name
// Handles all three asset types:
//   "GOLD-USDC.xyz" → perpDex="xyz", coin="GOLD" (perpDex: dot-suffix)
//   "BTC-USDC"      → perpDex="",    coin="BTC"  (perp: strip collateral)
//   "BTC/USDC"      → perpDex="",    coin="BTC/USDC" (spot: keep as-is)
//   "@107"           → perpDex="",    coin="@107" (spot @-format)
//   "BTC"            → perpDex="",    coin="BTC"  (legacy bare coin)
void parsePerpDex(const char* symbol, char* perpDex, size_t perpDexSize,
                  char* coin, size_t coinSize) {
    perpDex[0] = '\0';
    coin[0] = '\0';
    if (!symbol || !*symbol) return;

    // Spot symbols: '/' separator (BTC/USDC) or '@' prefix (@107)
    // Keep as-is for API (no normalization needed)
    if (strchr(symbol, '/') || symbol[0] == '@') {
        strncpy_s(coin, coinSize, symbol, _TRUNCATE);
        return;
    }

    // PerpDex: dot-suffix format (GOLD-USDC.xyz)
    // Split at last dot → perpDex=venue, remainder=COIN-COLLATERAL
    if (strchr(symbol, '.')) {
        char displayCoin[64];
        hl::utils::parsePerpDex(symbol, perpDex, perpDexSize,
                                displayCoin, sizeof(displayCoin));
        // Normalize: strip collateral suffix (GOLD-USDC → GOLD)
        hl::utils::normalizeCoin(displayCoin, coin, coinSize);
        return;
    }

    // Perp or legacy: normalize (BTC-USDC → BTC, BTC → BTC)
    strncpy_s(coin, coinSize, symbol, _TRUNCATE);
    hl::utils::normalizeCoin(coin, coin, coinSize);
}

// Build coin name for API calls (perpDex:coin or just coin)
std::string buildCoinForApi(const char* perpDex, const char* coin) {
    if (perpDex && perpDex[0]) {
        return std::string(perpDex) + ":" + coin;
    }
    return std::string(coin);
}

//=============================================================================
// BrokerOpen - Plugin initialization
//=============================================================================

DLLFUNC int BrokerOpen(char* name, FARPROC fpMessage, FARPROC fpProgress) {
    strcpy_s(name, 32, PLUGIN_NAME);
    (FARPROC&)BrokerMessage = fpMessage;
    (FARPROC&)BrokerProgress = fpProgress;

    hl::initGlobals();

#ifdef DEV_BUILD
    hl::g_config.diagLevel = 2;
    hl::g_logger.level = 2;
#endif

    hl::g_logger.callback = zorroLogCallback;

    if (!hl::crypto::init()) {
        if (BrokerMessage) {
            BrokerMessage("ERROR: Failed to initialize crypto");
        }
        return 0;
    }

    return PLUGIN_TYPE;
}

//=============================================================================
// BrokerLogin - Login/logout handling
//=============================================================================

DLLFUNC int BrokerLogin(char* user, char* pwd, char* type, char* accounts) {
    if (hl::g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "BrokerLogin: %s %s user='%s' isLogin=%d",
                  PLUGIN_NAME, PLUGIN_VERSION,
                  user ? user : "(null)", user ? 1 : 0);
        hl::g_logger.log(1, msg);
    }

    if (user && *user) {
        // ===== LOGIN =====

        // Preserve Zorro window handle and diag level (SET_HWND/SET_DIAGNOSTICS called before login)
        HWND savedWindow = hl::g_config.zorroWindow;
        int savedDiagLevel = hl::g_config.diagLevel;

        memset(&hl::g_config, 0, sizeof(hl::g_config));
        hl::g_config.zorroWindow = savedWindow;
        hl::g_config.diagLevel = savedDiagLevel;
        hl::g_config.isTestnet = true;
        hl::g_config.enableWebSocket = true;
        hl::g_config.useWsOrders = true;
        hl::g_config.enableHttpSeed = true;
        hl::g_config.httpSeedCooldownMs = hl::config::HTTP_SEED_COOLDOWN_MS;
        strcpy_s(hl::g_config.orderType, "Gtc");

        if (type && *type) {
            if (_stricmp(type, "Real") == 0) {
                hl::g_config.isTestnet = false;
                strcpy_s(hl::g_config.baseUrl, hl::config::MAINNET_REST);
                hl::g_logger.log(1, "BrokerLogin: LIVE TRADING on MAINNET");
            } else {
                strcpy_s(hl::g_config.baseUrl, hl::config::TESTNET_REST);
                hl::g_logger.log(1, "BrokerLogin: Demo mode on TESTNET");
            }
        } else {
            strcpy_s(hl::g_config.baseUrl, hl::config::TESTNET_REST);
        }

        if (user && *user) strncpy_s(hl::g_config.walletAddress, user, _TRUNCATE);
        if (pwd && *pwd) strncpy_s(hl::g_config.privateKey, pwd, _TRUNCATE);

        // [OPM-19] Derive signer address and compare with walletAddress
        char derivedAddr[64] = {0};
        bool addressesDiffer = false;
        if (hl::g_config.privateKey[0] &&
            hl::crypto::deriveAddress(hl::g_config.privateKey, derivedAddr, sizeof(derivedAddr))) {
            addressesDiffer = (_stricmp(derivedAddr, hl::g_config.walletAddress) != 0);
            if (hl::g_config.diagLevel >= 2) {
                hl::g_logger.logf(2, "BrokerLogin: User=%s Signer=%s %s",
                    hl::g_config.walletAddress, derivedAddr,
                    addressesDiffer ? "(agent key)" : "(same)");
            }
        }

        hl::trading::init();

        // Cache asset metadata — retry once on failure [OPM-105]
        int cached = hl::market::refreshMeta();
        if (cached <= 0) {
            hl::g_logger.log(1, "Meta fetch failed, retrying in 2s...");
            Sleep(2000);
            cached = hl::market::refreshMeta();
        }
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "Meta cache: %d assets", cached);
        }
        if (cached <= 0) {
            hl::g_logger.log(1, "BrokerLogin: Failed to fetch asset metadata — aborting");
            if (BrokerMessage) {
                BrokerMessage("ERROR: Failed to fetch asset metadata from Hyperliquid. "
                    "Check network connection and try again.");
            }
            return 0;
        }

        // [OPM-19] Detect agent wallets
        auto role = hl::account::checkUserRole();
        if (role == hl::account::UserRole::Agent) {
            if (!addressesDiffer) {
                if (BrokerMessage) {
                    BrokerMessage("ERROR: User field is an API wallet address. "
                        "Balance and positions will be empty. "
                        "Enter your MASTER account address in the User field "
                        "(keep the API wallet private key in Password).");
                }
                hl::g_logger.log(1, "ERROR: walletAddress is an agent wallet AND matches "
                    "the private key. Balance queries will return 0. User must enter "
                    "master account address in the User field.");
            } else {
                if (BrokerMessage) {
                    BrokerMessage("WARNING: User field is an API wallet address. "
                        "Balance will show 0. Use the master account address instead.");
                }
                hl::g_logger.log(1, "WARNING: walletAddress is an agent wallet — "
                    "clearinghouseState will return empty. Use master account address.");
            }
        } else if (role == hl::account::UserRole::Missing) {
            if (BrokerMessage) {
                BrokerMessage("WARNING: Address not found on Hyperliquid. "
                    "Check that the address is correct.");
            }
        } else if (addressesDiffer && role == hl::account::UserRole::User) {
            hl::g_logger.logf(1, "BrokerLogin: Using agent key (signer=%s) for "
                "master account %s", derivedAddr, hl::g_config.walletAddress);
        }

        // Initialize WebSocket
        if (hl::g_config.enableWebSocket) {
            if (!hl::g_priceCache) {
                hl::g_priceCache = new hl::ws::PriceCache();
            }

            auto* priceCache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);

            if (!hl::g_wsManager) {
                auto* wsMgr = new hl::ws::WebSocketManager(*priceCache);
                wsMgr->setDiagLevel(hl::g_config.diagLevel);
                wsMgr->setLogCallback(zorroLogCallback);
                wsMgr->setOrderUpdateCallback(onOrderUpdate);
                wsMgr->setFillNotifyCallback(onFillNotify);
                wsMgr->setUserAddress(hl::g_config.walletAddress);
                if (hl::g_config.zorroWindow) {
                    wsMgr->setZorroWindow(hl::g_config.zorroWindow);
                }
                hl::g_wsManager = wsMgr;
            }

            auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);

            std::string wsUrl = hl::g_config.baseUrl;
            size_t pos = wsUrl.find("https://");
            if (pos != std::string::npos) {
                wsUrl.replace(pos, 8, "wss://");
            }
            wsUrl += "/ws";

            wsMgr->start(wsUrl, hl::g_config.isTestnet);

            wsMgr->subscribeUserFills();
            wsMgr->subscribeClearinghouseState();
            wsMgr->subscribeOpenOrders();
            wsMgr->markInitialSubscriptionsQueued();
        }

        if (accounts) accounts[0] = '\0';

        char verMsg[64];
        sprintf_s(verMsg, "%s %s", PLUGIN_NAME, PLUGIN_VERSION);
        hl::g_logger.log(1, verMsg);

        return 1;

    } else {
        // ===== LOGOUT =====
        g_everReceivedAccountData = false;
        g_lastHttpFallbackTime = 0;

        hl::market::cleanup();
        hl::trading::cleanup();

        if (hl::g_wsManager) {
            auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
            wsMgr->stop();
            delete wsMgr;
            hl::g_wsManager = nullptr;
        }
        if (hl::g_priceCache) {
            auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
            delete cache;
            hl::g_priceCache = nullptr;
        }

        HWND savedWindow = hl::g_config.zorroWindow;
        memset(&hl::g_config, 0, sizeof(hl::g_config));
        hl::g_config.zorroWindow = savedWindow;

        return 0;
    }
}

//=============================================================================
// BrokerTime - Connection status and server time
//=============================================================================

DLLFUNC int BrokerTime(DATE *pTimeGMT) {
    if (!hl::g_config.walletAddress[0]) return 0;

    if (pTimeGMT) {
        time_t now = time(NULL);
        *pTimeGMT = 25569.0 + (double)now / 86400.0;
    }

    return 2;  // Crypto market is always open
}

//=============================================================================
// BrokerCommand - Extended commands (implemented in hl_broker_commands.cpp)
//=============================================================================

DLLFUNC double BrokerCommand(int mode, intptr_t parameter) {
    double result = handleBrokerCommand(mode, parameter);
    if (hl::g_config.diagLevel >= 1 && mode != GET_VOLUME) {
        hl::g_logger.logf(1, "BrokerCommand: mode=%d param=%lld -> %.4f",
                          mode, (long long)parameter, result);
    }
    return result;
}
