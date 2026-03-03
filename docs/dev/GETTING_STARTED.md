# Getting Started

Developer setup guide for the Hyperliquid broker plugin for Zorro.

---

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| Visual Studio 2022 Build Tools | 17.x | MSVC C++ compiler (Desktop development workload) |
| CMake | Bundled with VS Build Tools | Path: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` |
| vcpkg | Latest | Package manager for C++ dependencies |
| Git | Any | Source control |
| Zorro S | 2.x+ | Required only for integration testing (not for building) |

### vcpkg Setup

```powershell
# Clone vcpkg (if not already installed)
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Install IXWebSocket with TLS support (32-bit static)
vcpkg install ixwebsocket[ssl,mbedtls]:x86-windows-static
```

The plugin depends on IXWebSocket for WebSocket transport, with mbedTLS as the TLS backend. The `x86-windows-static` triplet is required because Zorro is a 32-bit application.

---

## Repository Layout

```
HyperliquidPlugin/
├── CMakeLists.txt               # Build configuration (C++17, Win32 DLL)
├── vcpkg.json                   # Dependency manifest
├── src/
│   ├── foundation/              # Layer 1: types, config, crypto, utilities
│   ├── transport/               # Layer 2: HTTP, WebSocket, price cache
│   ├── services/                # Layer 3: market data, trading, account
│   ├── api/                     # Layer 4: Zorro broker interface (DLL exports)
│   └── vendor/yyjson/           # Bundled JSON parser
├── include/                     # Zorro SDK headers (trading.h, variables.h)
├── lib/                         # Static libraries (secp256k1)
├── Source/                      # C crypto implementations (keccak256)
├── tests/                       # Unit tests and regression suite
│   ├── test_framework.h         # Lightweight assertion macros
│   ├── mocks/mock_zorro.h       # Zorro SDK function mocks
│   ├── unit/                    # Individual test source files
│   ├── run_unit_tests.bat       # Main test runner
│   └── compile_*.bat            # Per-test compile+run scripts
├── scripts/                     # Developer tools (pre-commit hooks)
├── docs/                        # Documentation
└── build_vcpkg/                 # CMake build output directory
```

---

## Build

### First-Time CMake Configure

Open a Developer Command Prompt for VS 2022 (or any shell with MSVC on PATH):

```batch
cd build_vcpkg
cmake .. -G "Visual Studio 17 2022" -A Win32 ^
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=x86-windows-static
```

Key flags:
- **`-A Win32`** -- Mandatory. Zorro is 32-bit; an x64 DLL will be silently ignored.
- **`-DVCPKG_TARGET_TRIPLET=x86-windows-static`** -- Links IXWebSocket and mbedTLS statically.

### Incremental Build

```powershell
powershell.exe -Command "& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build 'c:\Users\admki\OneDrive\Zorro-plugin\build_vcpkg' --config Release"
```

Output: `build_vcpkg/Release/Hyperliquid_Dev.dll` (~2.6 MB)

### Dev vs Production Build

| CMake option | DLL name | Plugin reports to Zorro as |
|-------------|----------|---------------------------|
| `-DDEV_BUILD=ON` (default) | `Hyperliquid_Dev.dll` | "Hyperliquid-DEV" |
| `-DDEV_BUILD=OFF` | `Hyperliquid.dll` | "Hyperliquid" |

---

## Run Tests

The test suite is 17 tests that run in under 10 seconds:

```batch
cd tests
run_unit_tests.bat
```

Expected output when all pass:
```
=====================================================
  HYPERLIQUID PLUGIN UNIT TEST SUITE
  Running fast tests before commit...
=====================================================

[1/17] Testing PIP/PIPCost/LotAmount formulas...
       PASSED
[2/17] Testing multi-asset position parsing...
       PASSED
...
[17/17] Testing market service logic...
        PASSED
=====================================================
  ALL TESTS PASSED (17 tests)
=====================================================
```

Tests compile standalone using `cl.exe` -- they don't require CMake or Zorro. Each test includes `mock_zorro.h` to provide stub implementations of Zorro's runtime function pointers.

### Install Pre-Commit Hook

```batch
cd scripts
install_hooks.bat
```

This blocks `git commit` if any test fails.

---

## Deploy to Zorro

Copy the built DLL to Zorro's plugin directory:

```powershell
Copy-Item 'build_vcpkg\Release\Hyperliquid_Dev.dll' 'C:\Zorro\Plugin\Hyperliquid_Dev.dll'
```

Adjust the Zorro path to match your installation. The plugin will appear in Zorro's broker dropdown as "Hyperliquid-DEV" (or "Hyperliquid" for production builds).

---

## Testnet Setup

1. Get testnet credentials from [Hyperliquid Testnet](https://app.hyperliquid-testnet.xyz)
2. In Zorro, select the Hyperliquid plugin
3. Enter your wallet address in the **User** field
4. Enter your private key (with `0x` prefix) in the **Password** field
5. Set Account to **Demo** (this selects testnet)

The plugin auto-detects testnet vs mainnet:
- **Real** account type = mainnet (`api.hyperliquid.xyz`)
- Any other account type = testnet (`api.hyperliquid-testnet.xyz`)

### Agent Wallets

If using an API/agent wallet (not the master wallet), enter the **master** wallet address in the User field. The plugin auto-detects the wallet type at login and will warn if the agent address is used instead (agent wallets return empty account data when queried directly).

---

## Build System Details

The CMake configuration produces these build targets:

| Target | Type | Contents |
|--------|------|----------|
| `hl_foundation` | Static lib | Types, config, globals, utils, crypto, EIP-712, msgpack |
| `hl_transport` | Static lib | HTTP client, WebSocket (connection, manager, cache, parsers) |
| `hl_services` | Static lib | Market, trading, account services + metadata |
| `hl_crypto_impl` | Static lib | keccak256 C implementation |
| `Hyperliquid` | **Shared DLL** | API layer + all static libs linked in |

The DLL exports the standard Zorro broker plugin functions: `zorro`, `BrokerOpen`, `BrokerLogin`, `BrokerTime`, `BrokerAsset`, `BrokerAccount`, `BrokerHistory2`, `BrokerBuy2`, `BrokerSell2`, `BrokerTrade`, `BrokerCommand`.

### Important Build Notes

- The Zorro SDK `include/` directory is **not** on the global include path. It contains custom `windows.h` and `stdio.h` that conflict with system headers. Only `hl_broker_internal.h` includes Zorro headers with careful macro cleanup.
- MSVC static runtime is used (`/MT`) to match the vcpkg static triplet.
- C++17 standard is required (for `std::optional`, structured bindings, etc.).
