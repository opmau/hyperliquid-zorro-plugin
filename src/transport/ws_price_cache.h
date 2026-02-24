//=============================================================================
// ws_price_cache.h - Thread-safe cache for prices, accounts, positions, orders
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: ws_types.h
// THREAD SAFETY: All public methods are thread-safe via CRITICAL_SECTION
//=============================================================================

#pragma once

#include "ws_types.h"
#include <map>
#include <vector>

namespace hl {
namespace ws {

/// Thread-safe cache for WebSocket data
///
/// Stores prices (from l2Book), account data (from clearinghouseState),
/// positions, open orders, and recent fills.
///
/// Usage:
///   PriceCache cache;
///   cache.setBidAsk("BTC", 50000.0, 50001.0);
///   double bid = cache.getBid("BTC");
///
class PriceCache {
public:
    PriceCache();
    ~PriceCache();

    // Non-copyable
    PriceCache(const PriceCache&) = delete;
    PriceCache& operator=(const PriceCache&) = delete;

    //=========================================================================
    // PRICE DATA (l2Book)
    //=========================================================================

    /// Set mid price only (from allMids - not recommended for trading)
    void setPrice(const std::string& coin, double price);

    /// Set bid/ask/mid (from l2Book) - preferred for trading
    void setBidAsk(const std::string& coin, double bid, double ask);

    /// Get mid price
    double getPrice(const std::string& coin) const;

    /// Get bid price
    double getBid(const std::string& coin) const;

    /// Get ask price
    double getAsk(const std::string& coin) const;

    /// Get full price data
    PriceData getPriceData(const std::string& coin) const;

    /// Get cache age in milliseconds (MAXDWORD if not present)
    DWORD getAge(const std::string& coin) const;

    /// Check if price exists and is fresh (age < maxAgeMs)
    bool isFresh(const std::string& coin, DWORD maxAgeMs = 5000) const;

    //=========================================================================
    // ACCOUNT DATA (webData3/clearinghouseState)
    //=========================================================================

    void setAccountData(double accValue, double marginUsed,
                       double withdraw, double ntlPos);
    void setSpotUSDC(double amount);
    AccountData getAccountData() const;
    DWORD getAccountDataAge() const;

    //=========================================================================
    // POSITION DATA (clearinghouseState)
    //=========================================================================

    void setPosition(const PositionData& pos);
    void clearPositions();
    PositionData getPosition(const std::string& coin) const;
    std::vector<PositionData> getAllPositions() const;
    DWORD getPositionAge(const std::string& coin) const;
    DWORD getPositionsAge() const;  // Age of last snapshot

    //=========================================================================
    // OPEN ORDERS (openOrders)
    //=========================================================================

    void setOpenOrder(const OpenOrderData& order);
    void removeOpenOrder(const std::string& oid);
    void clearOpenOrders();
    OpenOrderData getOpenOrder(const std::string& oid) const;
    std::vector<OpenOrderData> getAllOpenOrders() const;
    std::vector<OpenOrderData> getOpenOrdersForCoin(const std::string& coin) const;
    DWORD getOpenOrdersAge() const;  // Age of last snapshot

    //=========================================================================
    // FILLS (userFills)
    //=========================================================================

    void addFill(const FillData& fill);
    void clearFills();
    std::vector<FillData> getRecentFills(int count = 10) const;
    std::vector<FillData> getFillsForOrder(const std::string& oid) const;

    //=========================================================================
    // CLEAR ALL
    //=========================================================================

    void clear();

private:
    mutable CRITICAL_SECTION cs_;

    std::map<std::string, PriceData> prices_;
    AccountData accountData_;
    std::map<std::string, PositionData> positions_;
    std::map<std::string, OpenOrderData> openOrders_;
    std::vector<FillData> recentFills_;

    DWORD lastOpenOrdersUpdate_;
    DWORD lastPositionsUpdate_;

    static const size_t MAX_FILLS = 100;
};

} // namespace ws
} // namespace hl
