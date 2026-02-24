//=============================================================================
// hl_types.h - Core data structures for Hyperliquid plugin
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: None | THREAD SAFETY: Caller must sync
//=============================================================================

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace hl {

// === Enumerations ===

enum class OrderStatus { Pending, Open, Filled, PartialFill, Cancelled, Error };
enum class OrderSide { Buy, Sell };
enum class OrderType { Ioc, Gtc, Alo };  // Immediate-or-cancel, Good-til-cancelled, Add-liquidity-only
enum class TriggerType { NoTrigger, SL, TP };  // [OPM-77] SL=stop-loss, TP=take-profit

// Unified fill status detection with 0.999 threshold [OPM-91]
// Accounts for floating-point rounding in fill sizes (e.g., 0.9999 BTC for 1.0 BTC order)
inline OrderStatus determineFilledStatus(double filledSize, double requestedSize) {
    if (filledSize <= 0) return OrderStatus::Open;
    if (requestedSize <= 0) return OrderStatus::Filled;
    return (filledSize >= requestedSize * 0.999)
        ? OrderStatus::Filled : OrderStatus::PartialFill;
}

// === Price Data ===

struct PriceData {
    double bid = 0.0;
    double ask = 0.0;
    double mid = 0.0;
    uint32_t timestamp = 0;  // GetTickCount() when updated
    bool isStale(uint32_t maxAgeMs = 5000) const;
};

// === Asset Information ===

struct AssetInfo {
    char name[64] = {0};        // Display name (e.g., "BTC-USD")
    char coin[32] = {0};        // Exchange coin name (e.g., "BTC")
    int index = -1;             // Asset index for API calls
    int szDecimals = 0;         // Size decimal places
    int pxDecimals = 2;         // Price decimal places
    double minSize = 0.0;
    double tickSize = 0.0;
    int maxLeverage = 1;
    // Collateral
    char collateral[16] = {0};  // Collateral token name (e.g., "USDC", "USDT0", "USDH")
    // PerpDex support
    bool isPerpDex = false;
    char perpDex[32] = {0};     // PerpDex name if applicable
    int localIndex = 0;         // Index within perpDex universe
    int perpDexOffset = 0;      // Offset for this perpDex
    // Spot support
    bool isSpot = false;
    char spotCoin[32] = {0};    // API coin name for spot (e.g., "@107", "PURR/USDC")
};

// === Order Tracking ===

struct OrderState {
    char orderId[64] = {0};     // Exchange order ID (oid)
    char cloid[64] = {0};       // Client order ID
    char coin[32] = {0};
    OrderSide side = OrderSide::Buy;
    double requestedSize = 0.0;
    double filledSize = 0.0;
    double avgPrice = 0.0;
    OrderStatus status = OrderStatus::Pending;
    int zorroTradeId = 0;
    double lastUpdate = 0.0;    // DATE of last update
    char lastError[256] = {0};
};

// === Position Data ===

struct Position {
    std::string coin;
    double size = 0.0;          // Positive = long, negative = short
    double entryPrice = 0.0;
    double unrealizedPnl = 0.0;
    double leverage = 0.0;
    double liquidationPx = 0.0;
    double marginUsed = 0.0;
    uint32_t timestamp = 0;
};

// === Account State ===

struct AccountState {
    double equity = 0.0;
    double availableBalance = 0.0;
    double marginUsed = 0.0;
    double totalNotional = 0.0;
    std::vector<Position> positions;
    uint32_t timestamp = 0;
};

// === Fill Data ===

struct FillData {
    std::string oid;
    std::string coin;
    std::string tid;            // Trade ID
    OrderSide side = OrderSide::Buy;
    double price = 0.0;
    double size = 0.0;
    double fee = 0.0;
    int64_t timestamp = 0;      // ms since epoch
};

// === Order Status Query (three-state reconciliation) [OPM-89] ===

enum class QueryOutcome { Found, NotFound, Failed };

struct CloidQueryResult {
    char oid[64] = {0};         // Exchange order ID (if found)
    char status[32] = {0};     // "filled", "open", "canceled", "unknownOid", etc.
    double filledSize = 0.0;
    double avgPrice = 0.0;
    QueryOutcome outcome = QueryOutcome::Failed;
};

// === Order Request/Response ===

struct OrderRequest {
    std::string coin;
    int assetIndex = 0;
    OrderSide side = OrderSide::Buy;
    double size = 0.0;
    double limitPrice = 0.0;    // 0 for market orders; for triggers: slippage-protected price
    OrderType orderType = OrderType::Ioc;
    bool reduceOnly = false;
    std::string cloid;

    // Trigger order fields [OPM-77]
    TriggerType triggerType = TriggerType::NoTrigger;
    double triggerPx = 0.0;         // Price at which trigger activates
    bool triggerIsMarket = true;    // true = market execution when triggered

    bool isTriggerOrder() const { return triggerType != TriggerType::NoTrigger; }
};

struct OrderResult {
    bool success = false;
    std::string oid;            // Exchange order ID
    std::string cloid;
    std::string error;
    double filledSize = 0.0;
    double avgPrice = 0.0;
    std::string status;         // "open", "filled", etc.
};

} // namespace hl
