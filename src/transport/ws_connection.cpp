//=============================================================================
// ws_connection.cpp - Low-level WebSocket connection (IXWebSocket backend)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// BACKEND: IXWebSocket (replaced WinHTTP in OPM-127)
//
// Threading model:
//   - IXWebSocket runs its own background thread for receive
//   - onIxMessage() fires on that thread, pushes to queue, signals event
//   - poll() runs on ws_manager's connection thread, drains queue
//   - send() can be called from any thread (IXWebSocket is thread-safe)
//=============================================================================

#include "ws_connection.h"
#include <IXNetSystem.h>
#include <cstdio>
#include <cstdarg>

namespace hl {
namespace ws {

//=============================================================================
// CONSTRUCTION / DESTRUCTION
//=============================================================================

Connection::Connection()
    : queueEvent_(NULL), disconnectPending_(false),
      autoReconnect_(false), reconnected_(false), hasConnectedOnce_(false),
      connected_(false), state_(ConnectionState::Disconnected),
      lastMessageTime_(0), lastError_(0),
      disconnectReason_(DisconnectReason::None), disconnectError_(0),
      messageHandler_(nullptr), logCallback_(nullptr), logLevel_(0) {
    InitializeCriticalSection(&queueCs_);
    queueEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);  // Manual-reset
    ws_.disableAutomaticReconnection();
}

Connection::~Connection() {
    disconnect();
    if (queueEvent_) {
        CloseHandle(queueEvent_);
        queueEvent_ = NULL;
    }
    DeleteCriticalSection(&queueCs_);
}

//=============================================================================
// LOGGING
//=============================================================================

void Connection::setLogCallback(LogCallback cb, int level) {
    logCallback_ = cb;
    logLevel_ = level;
}

void Connection::log(int minLevel, const char* msg) {
    if (logCallback_ && logLevel_ >= minLevel) {
        logCallback_(msg);
    }
}

void Connection::logf(int minLevel, const char* fmt, ...) {
    if (logCallback_ && logLevel_ >= minLevel) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        logCallback_(buf);
    }
}

//=============================================================================
// AUTO-RECONNECT CONFIGURATION [OPM-128]
//=============================================================================

void Connection::enableAutoReconnect(int minWaitMs, int maxWaitMs) {
    autoReconnect_ = true;
    ws_.enableAutomaticReconnection();
    ix::WebSocketPerMessageDeflateOptions noDeflate(false);
    ws_.setMinWaitBetweenReconnectionRetries(minWaitMs);
    ws_.setMaxWaitBetweenReconnectionRetries(maxWaitMs);
    logf(2, "WS: Auto-reconnect enabled (min=%dms max=%dms)", minWaitMs, maxWaitMs);
}

void Connection::stopAutoReconnect() {
    autoReconnect_ = false;
    ws_.disableAutomaticReconnection();
    ws_.stop();
    connected_ = false;
    state_ = ConnectionState::Disconnected;
    log(1, "WS: Auto-reconnect stopped");
}

bool Connection::wasReconnected() {
    return reconnected_.exchange(false);
}

//=============================================================================
// IX MESSAGE CALLBACK (fires on IXWebSocket's internal thread)
//=============================================================================

void Connection::onIxMessage(const ix::WebSocketMessagePtr& msg) {
    switch (msg->type) {
        case ix::WebSocketMessageType::Open:
            logf(1, "WS: Connected (IXWebSocket)");
            connected_ = true;
            state_ = ConnectionState::Connected;
            lastMessageTime_ = time(NULL);
            // Set reconnected flag if this isn't the first Open [OPM-128]
            if (hasConnectedOnce_) {
                reconnected_ = true;
                log(1, "WS: Auto-reconnected");
            }
            hasConnectedOnce_ = true;
            // Signal queueEvent_ so connect() or poll() can stop waiting
            SetEvent(queueEvent_);
            break;

        case ix::WebSocketMessageType::Close:
            logf(1, "WS: Close frame (code=%d reason='%s')",
                 msg->closeInfo.code, msg->closeInfo.reason.c_str());
            // Only set ServerClose if we didn't initiate the disconnect.
            // When disconnect() calls ws_.stop(), IXWebSocket sends a close frame
            // and receives the server's response — but the reason should stay ClientClose.
            if (disconnectReason_.load() != DisconnectReason::ClientClose) {
                disconnectReason_ = DisconnectReason::ServerClose;
            }
            disconnectError_ = static_cast<DWORD>(msg->closeInfo.code);
            connected_ = false;
            state_ = ConnectionState::Disconnected;
            // With auto-reconnect, IXWebSocket will retry — don't signal permanent disconnect [OPM-128]
            if (!autoReconnect_) {
                disconnectPending_ = true;
            }
            SetEvent(queueEvent_);  // Wake up poll()
            break;

        case ix::WebSocketMessageType::Error:
            logf(1, "WS: Error: %s (retries=%d, wait=%dms, HTTP=%d)",
                 msg->errorInfo.reason.c_str(),
                 msg->errorInfo.retries,
                 msg->errorInfo.wait_time,
                 msg->errorInfo.http_status);
            disconnectReason_ = DisconnectReason::NetworkError;
            disconnectError_ = static_cast<DWORD>(msg->errorInfo.http_status);
            lastError_ = disconnectError_;
            connected_ = false;
            state_ = ConnectionState::Disconnected;
            // With auto-reconnect, IXWebSocket will retry — don't signal permanent disconnect [OPM-128]
            if (!autoReconnect_) {
                disconnectPending_ = true;
            }
            SetEvent(queueEvent_);  // Wake up poll() or connect()
            break;

        case ix::WebSocketMessageType::Message:
            // Queue the message for poll() to dispatch on caller's thread
            if (!msg->str.empty()) {
                EnterCriticalSection(&queueCs_);
                messageQueue_.push({msg->str});
                LeaveCriticalSection(&queueCs_);
                SetEvent(queueEvent_);
            }
            break;

        case ix::WebSocketMessageType::Ping:
            logf(2, "WS: Ping received");
            break;

        case ix::WebSocketMessageType::Pong:
            logf(2, "WS: Pong received");
            break;

        case ix::WebSocketMessageType::Fragment:
            // IXWebSocket handles fragment reassembly internally;
            // complete messages arrive as Message type
            break;
    }
}

//=============================================================================
// CONNECTION
//=============================================================================

bool Connection::connect(const char* hostname, bool secure,
                        const char* path, int timeoutMs) {
    if (connected_) {
        return true;  // Already connected
    }

    state_ = ConnectionState::Connecting;
    lastError_ = 0;
    disconnectReason_ = DisconnectReason::None;
    disconnectError_ = 0;
    disconnectPending_ = false;

    // Drain any stale messages from previous connection
    EnterCriticalSection(&queueCs_);
    while (!messageQueue_.empty()) messageQueue_.pop();
    LeaveCriticalSection(&queueCs_);
    ResetEvent(queueEvent_);

    // Build URL
    std::string url;
    url.reserve(256);
    url += secure ? "wss://" : "ws://";
    url += hostname;
    url += path;

    logf(1, "WS: Connecting to %s...", url.c_str());

    ws_.setUrl(url);
    // Auto-reconnect is configured by enableAutoReconnect() before connect [OPM-128]
    ws_.disablePerMessageDeflate();

    // Set callback (captures 'this')
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        onIxMessage(msg);
    });

    // Start IXWebSocket (launches background thread)
    ws_.start();

    // Wait for Open or Error callback
    DWORD waitResult = WaitForSingleObject(queueEvent_, static_cast<DWORD>(timeoutMs));
    ResetEvent(queueEvent_);

    if (connected_) {
        logf(1, "WS: Connected to %s", hostname);
        return true;
    }

    // Connection failed (timeout or error)
    if (waitResult == WAIT_TIMEOUT) {
        logf(1, "WS: Connection timeout (%dms)", timeoutMs);
    }
    ws_.stop();
    state_ = ConnectionState::Disconnected;
    return false;
}

void Connection::disconnect() {
    if (state_ == ConnectionState::Disconnected && !connected_) {
        return;
    }

    log(1, "WS: Disconnecting...");
    connected_ = false;
    state_ = ConnectionState::Disconnected;
    disconnectReason_ = DisconnectReason::ClientClose;
    disconnectPending_ = true;  // Signal permanent shutdown to poll() [OPM-128]
    SetEvent(queueEvent_);      // Wake poll() immediately

    // Suppress logging BEFORE stop() to prevent deadlock [OPM-133].
    // IXWebSocket's Close callback fires on its internal thread and calls
    // logf() → BrokerMessage → SendMessage to GUI. If the main (GUI) thread
    // is blocked here in ws_.stop() → _thread.join(), SendMessage deadlocks.
    // Clearing logCallback_ (raw pointer, atomic on x86) makes logf() a no-op
    // on the IX thread, avoiding the cross-thread SendMessage entirely.
    logCallback_ = nullptr;

    ws_.stop();

    // Drain queue
    EnterCriticalSection(&queueCs_);
    while (!messageQueue_.empty()) messageQueue_.pop();
    LeaveCriticalSection(&queueCs_);

    log(1, "WS: Disconnected");
}

//=============================================================================
// SEND
//=============================================================================

bool Connection::send(const char* message) {
    return send(message, strlen(message));
}

bool Connection::send(const char* message, size_t len) {
    if (!connected_) {
        return false;
    }

    auto result = ws_.send(std::string(message, len));

    if (!result.success) {
        logf(1, "WS: Send failed");
        disconnectReason_ = DisconnectReason::NetworkError;
        connected_ = false;
        state_ = ConnectionState::Disconnected;
        return false;
    }

    return true;
}

//=============================================================================
// POLL (RECEIVE)
//=============================================================================

int Connection::poll(int timeoutMs) {
    // Return -1 only for permanent disconnects (client-initiated disconnect())
    // With auto-reconnect active, temporary disconnects return 0 [OPM-128]
    if (!connected_ && !disconnectPending_ && !autoReconnect_) {
        return -1;
    }

    // Wait for data or disconnect signal
    EnterCriticalSection(&queueCs_);
    bool empty = messageQueue_.empty();
    LeaveCriticalSection(&queueCs_);

    if (empty && !disconnectPending_) {
        ResetEvent(queueEvent_);
        DWORD waitMs = (timeoutMs > 0) ? static_cast<DWORD>(timeoutMs) : 1;
        WaitForSingleObject(queueEvent_, waitMs);
    }

    // Check for disconnect
    if (disconnectPending_.exchange(false)) {
        return -1;
    }

    // Drain all available messages
    int messagesProcessed = 0;
    while (true) {
        EnterCriticalSection(&queueCs_);
        if (messageQueue_.empty()) {
            LeaveCriticalSection(&queueCs_);
            break;
        }
        QueuedMessage msg = std::move(messageQueue_.front());
        messageQueue_.pop();
        LeaveCriticalSection(&queueCs_);

        lastMessageTime_ = time(NULL);
        if (messageHandler_) {
            messageHandler_(msg.data.c_str(), msg.data.size());
        }
        messagesProcessed++;
    }

    return messagesProcessed;
}

} // namespace ws
} // namespace hl
