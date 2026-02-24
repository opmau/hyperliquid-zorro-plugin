cmake_minimum_required(VERSION 3.16)
project(HyperliquidPlugin VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime for vcpkg static triplets
if(VCPKG_TARGET_TRIPLET MATCHES "-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Dev vs Production build
option(DEV_BUILD "Build development DLL (Hyperliquid_Dev.dll)" ON)

# Windows-specific settings
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -DWIN32_LEAN_AND_MEAN)
    add_definitions(-DWIN32 -DNDEBUG -D_WINDOWS -D_USRDLL)
    if(DEV_BUILD)
        add_definitions(-DDEV_BUILD)
    endif()
endif()

# NOTE: Do NOT use global include_directories() for Zorro SDK (include/)
# Zorro's custom windows.h conflicts with system Windows headers
# Only add Zorro SDK includes to targets that actually need it (API layer)

#=============================================================================
# FOUNDATION LAYER - No dependencies on upper layers
#=============================================================================
add_library(hl_foundation STATIC
    src/foundation/hl_globals.cpp
    src/foundation/hl_utils.cpp
    src/foundation/hl_crypto.cpp
    src/foundation/hl_eip712.cpp
    src/foundation/hl_msgpack.cpp
)
target_include_directories(hl_foundation PUBLIC
    ${CMAKE_SOURCE_DIR}/src/foundation
    ${CMAKE_SOURCE_DIR}/Source/HyperliquidPlugin/crypto
)

#=============================================================================
# IXWebSocket (via vcpkg) — required for WebSocket transport [OPM-127]
#=============================================================================
find_package(ixwebsocket CONFIG REQUIRED)

#=============================================================================
# TRANSPORT LAYER - Depends on Foundation + IXWebSocket
#=============================================================================
add_library(hl_transport STATIC
    src/transport/hl_http.cpp
    src/transport/ws_price_cache.cpp
    src/transport/ws_connection.cpp
    src/transport/ws_parsers.cpp
    src/transport/ws_manager.cpp
    src/vendor/yyjson/yyjson.c
)
target_include_directories(hl_transport PUBLIC
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    ${CMAKE_SOURCE_DIR}/src/vendor/yyjson
)
target_link_libraries(hl_transport
    PUBLIC hl_foundation
    PUBLIC ixwebsocket::ixwebsocket   # PUBLIC: ws_connection.h exposes IXWebSocket types
    PRIVATE bcrypt                     # Required by mbedTLS (IXWebSocket TLS backend)
)

#=============================================================================
# SERVICES LAYER - Depends on Transport
#=============================================================================
add_library(hl_services STATIC
    src/services/hl_meta.cpp
    src/services/hl_meta_spot.cpp
    src/services/hl_market_service.cpp
    src/services/hl_trading_service.cpp
    src/services/hl_trading_cancel.cpp
    src/services/hl_account_service.cpp
)
target_include_directories(hl_services PUBLIC
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
)
target_link_libraries(hl_services
    PUBLIC hl_transport
)

#=============================================================================
# CRYPTO (keccak256 implementation from original plugin)
# Note: EIP-712 is now in hl_foundation as hl_eip712.cpp
#=============================================================================
add_library(hl_crypto_impl STATIC
    Source/HyperliquidPlugin/crypto/keccak256.c
)
target_include_directories(hl_crypto_impl PUBLIC
    ${CMAKE_SOURCE_DIR}/Source/HyperliquidPlugin/crypto
)

#=============================================================================
# API LAYER - Zorro Broker interface
#=============================================================================
add_library(hl_api STATIC
    src/api/hl_broker.cpp
    src/api/hl_broker_market.cpp
    src/api/hl_broker_trade.cpp
    src/api/hl_broker_commands.cpp
)
target_include_directories(hl_api PUBLIC
    ${CMAKE_SOURCE_DIR}/src/api
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    # NOTE: Do NOT add ${CMAKE_SOURCE_DIR}/include here!
    # Zorro SDK has custom windows.h/stdio.h that conflict with system headers.
    # hl_broker.cpp uses relative paths: ../../include/trading.h
)
target_link_libraries(hl_api
    PUBLIC hl_services
)

#=============================================================================
# MAIN DLL - Full Hyperliquid Plugin
#=============================================================================
add_library(Hyperliquid SHARED
    src/api/hl_broker.cpp
    src/api/hl_broker_market.cpp
    src/api/hl_broker_trade.cpp
    src/api/hl_broker_commands.cpp
)
target_include_directories(Hyperliquid PRIVATE
    ${CMAKE_SOURCE_DIR}/src/api
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    # NOTE: Do NOT add ${CMAKE_SOURCE_DIR}/include here!
    # Zorro SDK has custom windows.h/stdio.h that conflict with system headers.
    # hl_broker.cpp uses relative paths: ../../include/trading.h
)
target_link_libraries(Hyperliquid PRIVATE
    hl_services
    hl_crypto_impl
)
if(DEV_BUILD)
    set(HL_OUTPUT_NAME "Hyperliquid_Dev")
else()
    set(HL_OUTPUT_NAME "Hyperliquid")
endif()
set_target_properties(Hyperliquid PROPERTIES
    OUTPUT_NAME "${HL_OUTPUT_NAME}"
    SUFFIX ".dll"
)

#=============================================================================
# TESTS
#=============================================================================
# Unit test for price cache
add_executable(test_ws_price_cache
    tests/test_ws_price_cache.cpp
)
target_link_libraries(test_ws_price_cache PRIVATE hl_transport)

# Compile test (just builds, doesn't run)
add_executable(compile_test_transport
    tests/compile_test_transport.cpp
)
target_link_libraries(compile_test_transport PRIVATE hl_transport)

# Live test: poll() timeout contract (connects to testnet)
# Now uses hl_transport (IXWebSocket backend) [OPM-127]
add_executable(test_poll_timeout
    tests/test_poll_timeout.cpp
)
target_include_directories(test_poll_timeout PRIVATE
    src/transport
    src/foundation
    tests
)
target_link_libraries(test_poll_timeout PRIVATE hl_transport)

# Connection class regression test (IXWebSocket backend) [OPM-127]
add_executable(test_ws_connection_ix
    tests/test_ws_connection_ix.cpp
)
target_include_directories(test_ws_connection_ix PRIVATE
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(test_ws_connection_ix PRIVATE hl_transport)

# Auto-reconnect test (simplified ws_manager) [OPM-128]
add_executable(test_ws_auto_reconnect
    tests/test_ws_auto_reconnect.cpp
)
target_include_directories(test_ws_auto_reconnect PRIVATE
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(test_ws_auto_reconnect PRIVATE hl_transport)

# IXWebSocket direct API test [OPM-126]
add_executable(test_ixwebsocket_connect
    tests/test_ixwebsocket_connect.cpp
)
target_include_directories(test_ixwebsocket_connect PRIVATE
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(test_ixwebsocket_connect PRIVATE
    ixwebsocket::ixwebsocket
    bcrypt
)

# l2Book multi-asset integration test [OPM-129]
add_executable(test_ws_l2book_integration
    tests/test_ws_l2book_integration.cpp
)
target_include_directories(test_ws_l2book_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src/transport
    ${CMAKE_SOURCE_DIR}/src/foundation
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(test_ws_l2book_integration PRIVATE hl_transport)
