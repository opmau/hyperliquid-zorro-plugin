# Hyperliquid Plugin for Zorro

A broker plugin that connects [Zorro](https://zorro-project.com/) algorithmic trading platform to the [Hyperliquid](https://hyperliquid.xyz/) decentralized exchange.

## Features

- **Perpetual futures trading** on Hyperliquid L1
- **Real-time market data** via WebSocket (l2Book, allMids)
- **HTTP fallback** when WebSocket is unhealthy
- **Native EIP-712 signing** (no Python dependency)
- **Position and order management** through Zorro's broker interface

## Architecture

```
API Layer         hl_broker.cpp, hl_broker_commands.cpp, hl_broker_trade.cpp
                  (Zorro broker interface - thin adapter)

Service Layer     hl_market_service, hl_trading_service, hl_account_service, hl_meta
                  (Business logic, coordinates transport)

Transport Layer   hl_http, ws_connection, ws_manager, ws_price_cache
                  (Network I/O, WebSocket, caching)

Foundation Layer  hl_types, hl_config, hl_globals, hl_utils, hl_crypto, hl_eip712
                  (No dependencies on upper layers)
```

## Prerequisites

- **Windows** (Zorro is Windows-only)
- **[Zorro](https://zorro-project.com/)** S or higher (free version works)
- **[Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/)** with C++ workload
- **[vcpkg](https://vcpkg.io/)** package manager
- **[IXWebSocket](https://github.com/machinezone/IXWebSocket)** (installed via vcpkg)

## Setup

### 1. Clone and create Zorro SDK junction

```batch
git clone https://github.com/opmau/hyperliquid-zorro-plugin.git
cd hyperliquid-zorro-plugin

REM Create junction to your Zorro include directory
mklink /J include "C:\Zorro\include"
```

### 2. Install dependencies

```batch
vcpkg install ixwebsocket:x86-windows-static
```

### 3. Build

```batch
mkdir build_vcpkg
cd build_vcpkg
cmake .. -G "Visual Studio 17 2022" -A Win32 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x86-windows-static
cmake --build . --config Release
```

**Important:** Zorro is a 32-bit application. Always build with `-A Win32`.

Output: `build_vcpkg/Release/Hyperliquid_Dev.dll`

### 4. Deploy

Copy the DLL to your Zorro Plugin directory:

```batch
copy build_vcpkg\Release\Hyperliquid_Dev.dll "C:\Zorro\Plugin\"
```

### 5. Configure in Zorro

1. Select **Hyperliquid** (or **Hyperliquid-DEV**) as broker
2. Enter your **wallet address** in the User field
3. Enter your **private key** in the Password field
4. Use testnet first: set `TESTNET=1` in your strategy

## Testing

```batch
cd tests
run_unit_tests.bat
```

## Project Structure

```
src/
  foundation/    Types, config, utilities, crypto primitives
  transport/     HTTP client, WebSocket connection/manager/cache
  services/      Market data, trading, account, metadata services
  api/           Zorro broker plugin interface
tests/           Unit and compile tests
docs/            Reference documentation
build/           Compile helper scripts
```

## Status

Under active development. Currently supports:
- Perpetual futures (not spot)
- Market and limit orders
- Position queries and account info
- Real-time price streaming

## License

MIT
