//=============================================================================
// ws_connection.h - Low-level WebSocket connection management
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: ws_types.h, IXWebSocket
// THREAD SAFETY: connect/disconnect are NOT thread-safe. send() can be called
//                from any thread while connected. poll() must be called from
//                a single thread (typically the connection thread).
//
// BACKEND: IXWebSocket (replaced WinHTTP in OPM-127)
//   IXWebSocket fires callbacks on its internal thread. This class queues
//   incoming messages and lets poll() drain them on the caller's thread,
//   preserving the same threading contract that ws_manager.cpp expects.
//=============================================================================

#pragma once

#include "ws_types.h"
#include <IXWebSocket.h>
#include <functional>
#include <string>
#include <queue>
#include <atomic>

namespace hl {
namespace ws {

/// Low-level WebSocket connection using IXWebSocket.
/// Incoming messages are queued internally. Call poll() to drain the queue
/// and dispatch messages to the messageHandler on the caller's thread.
class Connection {
public:
    // Callback for complete messages
    using MessageHandler = std::function<void(const char* data, size_t len)>;

    Connection();
    ~Connection();

    // Non-copyable
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    //=========================================================================
    // LIFECYCLE
    //=========================================================================

    /// Connect to WebSocket endpoint
    /// @param hostname e.g., "api.hyperliquid.xyz"
    /// @param secure Use wss:// (always true for Hyperliquid)
    /// @param path WebSocket path (default "/ws")
    /// @param timeoutMs Connection timeout
    /// @return true on success
    bool connect(const char* hostname, bool secure = true,
                const char* path = "/ws", int timeoutMs = 30000);

    /// Disconnect and cleanup
    void disconnect();

    /// Check if currently connected
    bool isConnected() const { return connected_.load(); }

    /// Get connection state
    ConnectionState state() const { return state_.load(); }

    //=========================================================================
    // I/O
    //=========================================================================

    /// Send a text message
    /// @return true on success
    bool send(const char* message);

    /// Send a text message with explicit length
    bool send(const char* message, size_t len);

    /// Drain queued messages from the internal queue and dispatch them.
    /// @param timeoutMs Max wait if queue is empty (0 = non-blocking)
    /// @return Number of messages dispatched, -1 on disconnect
    int poll(int timeoutMs = 100);

    /// Set handler for incoming messages
    void setMessageHandler(MessageHandler handler) { messageHandler_ = handler; }

    //=========================================================================
    // DIAGNOSTICS
    //=========================================================================

    void setLogCallback(LogCallback cb, int level);
    time_t lastMessageTime() const { return lastMessageTime_.load(); }
    DWORD getLastError() const { return lastError_; }

    /// Reason for the most recent disconnect
    DisconnectReason disconnectReason() const { return disconnectReason_.load(); }
    /// Error code when disconnectReason == NetworkError
    DWORD disconnectError() const { return disconnectError_; }

    /// Enable IXWebSocket auto-reconnect with configurable backoff.
    /// Must be called BEFORE connect(). When enabled:
    ///   - IXWebSocket retries automatically on disconnect/error
    ///   - poll() returns 0 (not -1) during temporary disconnects
    ///   - wasReconnected() returns true after each reconnect [OPM-128]
    void enableAutoReconnect(int minWaitMs = 1000, int maxWaitMs = 30000);

    /// Stop IXWebSocket and disable auto-reconnect.
    /// Safe to call from connection thread (unlike disconnect() which suppresses
    /// logCallback_ for GUI thread deadlock prevention [OPM-133]).
    /// Call enableAutoReconnect() + connect() to resume.
    void stopAutoReconnect();

    /// Check and clear the reconnected flag.
    /// Returns true (once) after IXWebSocket auto-reconnects.
    /// Caller should re-subscribe channels when this returns true. [OPM-128]
    bool wasReconnected();

private:
    // IXWebSocket instance
    ix::WebSocket ws_;

    // Internal message queue (filled by IXWebSocket callback thread,
    // drained by poll() on the caller's thread)
    struct QueuedMessage {
        std::string data;
    };
    CRITICAL_SECTION queueCs_;
    std::queue<QueuedMessage> messageQueue_;
    HANDLE queueEvent_;        // Manual-reset, signaled when queue non-empty

    // Disconnect detection: set by callback thread, read by poll()
    std::atomic<bool> disconnectPending_;

    // Auto-reconnect state [OPM-128]
    bool autoReconnect_;            // true if enableAutoReconnect() was called
    std::atomic<bool> reconnected_; // set true on Open after a previous connection
    bool hasConnectedOnce_;         // tracks whether first Open has fired

    // State
    std::atomic<bool> connected_;
    std::atomic<ConnectionState> state_;
    std::atomic<time_t> lastMessageTime_;
    DWORD lastError_;
    std::atomic<DisconnectReason> disconnectReason_;
    DWORD disconnectError_;

    // Callbacks
    MessageHandler messageHandler_;
    LogCallback logCallback_;
    int logLevel_;

    // IXWebSocket message callback (fires on IX internal thread)
    void onIxMessage(const ix::WebSocketMessagePtr& msg);

    void log(int minLevel, const char* msg);
    void logf(int minLevel, const char* fmt, ...);
};

} // namespace ws
} // namespace hl
