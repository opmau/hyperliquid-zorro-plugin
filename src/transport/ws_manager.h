//=============================================================================
// ws_manager.h - High-level WebSocket orchestration
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: ws_connection.h, ws_price_cache.h
// THREAD SAFETY: All public methods are thread-safe except start/stop
//=============================================================================

#pragma once

#include "ws_connection.h"
#include "ws_price_cache.h"
#include <queue>
#include <map>
#include <atomic>

namespace hl {
namespace ws {

/// High-level WebSocket manager for Hyperliquid
///
/// Orchestrates connection, subscriptions, and message parsing.
/// Uses PriceCache for storing received data.
///
/// Usage:
///   PriceCache cache;
///   WebSocketManager mgr(cache);
///   mgr.setUserAddress("0x...");
///   mgr.start("api.hyperliquid.xyz");
///   mgr.subscribeL2Book("BTC");
///   mgr.markInitialSubscriptionsQueued();
///   // ... later
///   mgr.stop();
///
class WebSocketManager {
public:
    explicit WebSocketManager(PriceCache& cache);
    ~WebSocketManager();

    // Non-copyable
    WebSocketManager(const WebSocketManager&) = delete;
    WebSocketManager& operator=(const WebSocketManager&) = delete;

    //=========================================================================
    // LIFECYCLE
    //=========================================================================

    void start(const std::string& hostname, bool testnet = false);
    void stop();
    bool isRunning() const { return running_.load(); }
    bool isConnected() const { return connection_.isConnected(); }
    bool isHealthy() const;
    int getSecondsSinceLastMessage() const;

    //=========================================================================
    // CONFIGURATION
    //=========================================================================

    void setUserAddress(const std::string& address) { userAddress_ = address; }
    void setDiagLevel(int level);
    void setLogCallback(LogCallback cb);
    void setZorroWindow(HWND hwnd) { zorroWindow_ = hwnd; }
    void setOrderUpdateCallback(OrderUpdateCallback cb) { orderUpdateCallback_ = cb; }

    // Fill notification callback (from userFills subscription) [OPM-87]
    // Called on WS connection thread — implementation must be thread-safe.
    // Parameters: oid, cumulative filled size, weighted avg fill price.
    typedef void (*FillNotifyCallback)(const char* oid, double totalFilledSz,
                                        double avgFillPx);
    void setFillNotifyCallback(FillNotifyCallback cb) { fillNotifyCallback_ = cb; }

    //=========================================================================
    // SUBSCRIPTIONS (queue for sender thread)
    //=========================================================================

    void subscribeL2Book(const std::string& coin);
    bool hasL2BookData(const std::string& coin);
    void subscribeUserFills();
    void subscribeClearinghouseState();
    void subscribeOpenOrders();
    void subscribeAllAccountData();

    /// Signal that initial subscriptions are queued (unlocks sender thread)
    void markInitialSubscriptionsQueued() { initialSubsQueued_ = true; }

    //=========================================================================
    // ORDER POST (synchronous - waits for response)
    //=========================================================================

    OrderResponse sendOrderSync(const std::string& orderJson, DWORD timeoutMs = 5000);

    //=========================================================================
    // INDEX MAPPINGS (for allMids parsing: @142 -> "BTC")
    //=========================================================================

    void setIndexMapping(int index, const std::string& coin);
    void clearIndexMappings();
    std::string getCoinByIndex(int index) const;

    //=========================================================================
    // ACCESS
    //=========================================================================

    PriceCache& getPriceCache() { return cache_; }

private:
    // Core components
    PriceCache& cache_;
    Connection connection_;

    // Thread management (sender thread removed in OPM-128 — IXWebSocket handles I/O)
    HANDLE connectionThread_;
    HANDLE shutdownEvent_;
    std::atomic<bool> running_;

    // Configuration
    std::string hostname_;
    bool testnet_;
    std::string userAddress_;
    HWND zorroWindow_;
    int diagLevel_;
    LogCallback logCallback_;
    OrderUpdateCallback orderUpdateCallback_;
    FillNotifyCallback fillNotifyCallback_;

    // l2Book subscriptions
    CRITICAL_SECTION l2SubCs_;
    std::vector<std::string> l2Subscriptions_;
    std::vector<std::string> pendingL2Subs_;

    // Account subscriptions
    CRITICAL_SECTION accountSubCs_;
    std::atomic<bool> subscribedUserFills_;
    std::atomic<bool> subscribedClearinghouse_;
    std::atomic<bool> subscribedOpenOrders_;
    bool pendingUserFillsSub_;
    bool pendingClearinghouseSub_;
    bool pendingOpenOrdersSub_;
    std::atomic<bool> initialSubsQueued_;

    // Order post queue
    CRITICAL_SECTION postCs_;
    struct PendingPost { int id; std::string json; };
    std::queue<PendingPost> pendingPosts_;
    std::atomic<int> nextRequestId_;

    // Circuit breaker — stops reconnect storm after consecutive failures
    int consecutiveReconnects_;
    bool circuitOpen_;
    DWORD circuitOpenedAt_;

    // Response correlation
    CRITICAL_SECTION responseCs_;
    std::map<int, OrderResponse> completedResponses_;
    std::map<int, HANDLE> responseEvents_;

    // Index-to-coin mapping (for allMids parsing: @142 -> "BTC")
    mutable CRITICAL_SECTION indexMapCs_;
    std::map<int, std::string> indexToCoin_;

    // Thread functions
    static DWORD WINAPI ConnectionThreadProc(LPVOID param);
    void connectionLoop();

    // Message handling
    void handleMessage(const char* data, size_t len);
    void parseL2Book(const char* json);
    void parseClearinghouseState(const char* json);
    void parseOpenOrders(const char* json);
    void parseUserFills(const char* json);
    void parseOrderUpdates(const char* json);
    void parsePostResponse(const char* json);

    // Subscription helpers
    void subscribeInitialChannels();
    void sendPendingL2Subscriptions();
    void sendPendingAccountSubscriptions();
    void sendPendingPosts();
    void requeueSubscriptionsAfterReconnect();

    // Logging
    void log(int minLevel, const char* msg);
    void logf(int minLevel, const char* fmt, ...);
};

} // namespace ws
} // namespace hl
