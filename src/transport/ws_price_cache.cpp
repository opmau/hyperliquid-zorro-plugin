//=============================================================================
// ws_price_cache.cpp - Thread-safe cache implementation
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// THREAD SAFETY: All methods use CRITICAL_SECTION for synchronization
//=============================================================================

#include "ws_price_cache.h"

namespace hl {
namespace ws {

//=============================================================================
// CONSTRUCTION / DESTRUCTION
//=============================================================================

PriceCache::PriceCache()
    : lastOpenOrdersUpdate_(0), lastPositionsUpdate_(0) {
    InitializeCriticalSection(&cs_);
}

PriceCache::~PriceCache() {
    DeleteCriticalSection(&cs_);
}

//=============================================================================
// PRICE DATA
//=============================================================================

void PriceCache::setPrice(const std::string& coin, double price) {
    EnterCriticalSection(&cs_);
    prices_[coin].mid = price;
    prices_[coin].timestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

void PriceCache::setBidAsk(const std::string& coin, double bid, double ask) {
    EnterCriticalSection(&cs_);
    prices_[coin].bid = bid;
    prices_[coin].ask = ask;
    prices_[coin].mid = (bid + ask) / 2.0;
    prices_[coin].timestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

double PriceCache::getPrice(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    double price = 0.0;
    auto it = prices_.find(coin);
    if (it != prices_.end()) {
        price = it->second.mid;
    }
    LeaveCriticalSection(&cs_);
    return price;
}

double PriceCache::getBid(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    double bid = 0.0;
    auto it = prices_.find(coin);
    if (it != prices_.end()) {
        bid = it->second.bid;
    }
    LeaveCriticalSection(&cs_);
    return bid;
}

double PriceCache::getAsk(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    double ask = 0.0;
    auto it = prices_.find(coin);
    if (it != prices_.end()) {
        ask = it->second.ask;
    }
    LeaveCriticalSection(&cs_);
    return ask;
}

PriceData PriceCache::getPriceData(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    PriceData data;
    auto it = prices_.find(coin);
    if (it != prices_.end()) {
        data = it->second;
    }
    LeaveCriticalSection(&cs_);
    return data;
}

DWORD PriceCache::getAge(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    DWORD age = MAXDWORD;
    auto it = prices_.find(coin);
    if (it != prices_.end() && it->second.timestamp > 0) {
        DWORD now = GetTickCount();
        age = (now >= it->second.timestamp) ? (now - it->second.timestamp) : 0;
    }
    LeaveCriticalSection(&cs_);
    return age;
}

bool PriceCache::isFresh(const std::string& coin, DWORD maxAgeMs) const {
    return getAge(coin) < maxAgeMs;
}

//=============================================================================
// ACCOUNT DATA
//=============================================================================

void PriceCache::setAccountData(double accValue, double marginUsed,
                               double withdraw, double ntlPos) {
    EnterCriticalSection(&cs_);
    accountData_.accountValue = accValue;
    accountData_.totalMarginUsed = marginUsed;
    accountData_.withdrawable = withdraw;
    accountData_.totalNtlPos = ntlPos;
    accountData_.timestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

void PriceCache::setSpotUSDC(double amount) {
    EnterCriticalSection(&cs_);
    accountData_.spotUSDC = amount;
    accountData_.spotTimestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

AccountData PriceCache::getAccountData() const {
    EnterCriticalSection(&cs_);
    AccountData data = accountData_;
    LeaveCriticalSection(&cs_);
    return data;
}

DWORD PriceCache::getAccountDataAge() const {
    EnterCriticalSection(&cs_);
    DWORD age = MAXDWORD;
    if (accountData_.timestamp > 0) {
        DWORD now = GetTickCount();
        age = (now >= accountData_.timestamp) ? (now - accountData_.timestamp) : 0;
    }
    LeaveCriticalSection(&cs_);
    return age;
}

//=============================================================================
// POSITION DATA
//=============================================================================

void PriceCache::setPosition(const PositionData& pos) {
    EnterCriticalSection(&cs_);
    positions_[pos.coin] = pos;
    positions_[pos.coin].timestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

void PriceCache::clearPositions() {
    EnterCriticalSection(&cs_);
    positions_.clear();
    lastPositionsUpdate_ = GetTickCount();
    LeaveCriticalSection(&cs_);
}

PositionData PriceCache::getPosition(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    PositionData pos;
    auto it = positions_.find(coin);
    if (it != positions_.end()) {
        pos = it->second;
    }
    LeaveCriticalSection(&cs_);
    return pos;
}

std::vector<PositionData> PriceCache::getAllPositions() const {
    EnterCriticalSection(&cs_);
    std::vector<PositionData> result;
    for (const auto& p : positions_) {
        if (p.second.size != 0) {  // Only non-zero positions
            result.push_back(p.second);
        }
    }
    LeaveCriticalSection(&cs_);
    return result;
}

DWORD PriceCache::getPositionAge(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    DWORD age = MAXDWORD;
    auto it = positions_.find(coin);
    if (it != positions_.end() && it->second.timestamp > 0) {
        DWORD now = GetTickCount();
        age = (now >= it->second.timestamp) ? (now - it->second.timestamp) : 0;
    }
    LeaveCriticalSection(&cs_);
    return age;
}

DWORD PriceCache::getPositionsAge() const {
    EnterCriticalSection(&cs_);
    DWORD now = GetTickCount();
    DWORD age = (lastPositionsUpdate_ > 0 && now >= lastPositionsUpdate_)
                ? (now - lastPositionsUpdate_) : MAXDWORD;
    LeaveCriticalSection(&cs_);
    return age;
}

//=============================================================================
// OPEN ORDERS
//=============================================================================

void PriceCache::setOpenOrder(const OpenOrderData& order) {
    EnterCriticalSection(&cs_);
    openOrders_[order.oid] = order;
    openOrders_[order.oid].timestamp = GetTickCount();
    LeaveCriticalSection(&cs_);
}

void PriceCache::removeOpenOrder(const std::string& oid) {
    EnterCriticalSection(&cs_);
    openOrders_.erase(oid);
    LeaveCriticalSection(&cs_);
}

void PriceCache::clearOpenOrders() {
    EnterCriticalSection(&cs_);
    openOrders_.clear();
    lastOpenOrdersUpdate_ = GetTickCount();
    LeaveCriticalSection(&cs_);
}

OpenOrderData PriceCache::getOpenOrder(const std::string& oid) const {
    EnterCriticalSection(&cs_);
    OpenOrderData order;
    auto it = openOrders_.find(oid);
    if (it != openOrders_.end()) {
        order = it->second;
    }
    LeaveCriticalSection(&cs_);
    return order;
}

std::vector<OpenOrderData> PriceCache::getAllOpenOrders() const {
    EnterCriticalSection(&cs_);
    std::vector<OpenOrderData> result;
    for (const auto& o : openOrders_) {
        result.push_back(o.second);
    }
    LeaveCriticalSection(&cs_);
    return result;
}

std::vector<OpenOrderData> PriceCache::getOpenOrdersForCoin(const std::string& coin) const {
    EnterCriticalSection(&cs_);
    std::vector<OpenOrderData> result;
    for (const auto& o : openOrders_) {
        if (o.second.coin == coin) {
            result.push_back(o.second);
        }
    }
    LeaveCriticalSection(&cs_);
    return result;
}

DWORD PriceCache::getOpenOrdersAge() const {
    EnterCriticalSection(&cs_);
    DWORD now = GetTickCount();
    DWORD age = (lastOpenOrdersUpdate_ > 0 && now >= lastOpenOrdersUpdate_)
                ? (now - lastOpenOrdersUpdate_) : MAXDWORD;
    LeaveCriticalSection(&cs_);
    return age;
}

//=============================================================================
// FILLS
//=============================================================================

void PriceCache::addFill(const FillData& fill) {
    EnterCriticalSection(&cs_);
    recentFills_.push_back(fill);
    if (recentFills_.size() > MAX_FILLS) {
        recentFills_.erase(recentFills_.begin());
    }
    LeaveCriticalSection(&cs_);
}

void PriceCache::clearFills() {
    EnterCriticalSection(&cs_);
    recentFills_.clear();
    LeaveCriticalSection(&cs_);
}

std::vector<FillData> PriceCache::getRecentFills(int count) const {
    EnterCriticalSection(&cs_);
    std::vector<FillData> result;
    int start = static_cast<int>(recentFills_.size()) - count;
    if (start < 0) start = 0;
    for (int i = start; i < static_cast<int>(recentFills_.size()); i++) {
        result.push_back(recentFills_[i]);
    }
    LeaveCriticalSection(&cs_);
    return result;
}

std::vector<FillData> PriceCache::getFillsForOrder(const std::string& oid) const {
    EnterCriticalSection(&cs_);
    std::vector<FillData> result;
    for (const auto& f : recentFills_) {
        if (f.oid == oid) {
            result.push_back(f);
        }
    }
    LeaveCriticalSection(&cs_);
    return result;
}

//=============================================================================
// CLEAR ALL
//=============================================================================

void PriceCache::clear() {
    EnterCriticalSection(&cs_);
    prices_.clear();
    accountData_ = AccountData();
    positions_.clear();
    openOrders_.clear();
    recentFills_.clear();
    lastOpenOrdersUpdate_ = 0;
    lastPositionsUpdate_ = 0;
    LeaveCriticalSection(&cs_);
}

} // namespace ws
} // namespace hl
