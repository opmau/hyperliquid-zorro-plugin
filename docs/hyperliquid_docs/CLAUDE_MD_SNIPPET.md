# Project Context

## Hyperliquid API Reference

Full API documentation is in `docs/hyperliquid-api/`. Read `docs/hyperliquid-api/INDEX.md` for structure.

Key files:
- `05-info-endpoint.md` - All read-only queries (positions, orders, account state, market data)
- `06-exchange-endpoint.md` - All write operations (place/cancel orders, transfers)
- `07-websocket.md` - WebSocket subscriptions (orderbook, trades, user events)
- `09-signing.md` - Request signing (EIP-712 typed data)
- `10-rate-limits.md` - Rate limits per endpoint
- `04-nonces-and-api-wallets.md` - API wallet setup and nonce management

API endpoints:
- Mainnet REST: https://api.hyperliquid.xyz
- Testnet REST: https://api.hyperliquid-testnet.xyz
- Mainnet WS: wss://api.hyperliquid.xyz/ws
- Testnet WS: wss://api.hyperliquid-testnet.xyz/ws

We work directly against the REST/WebSocket API (not the Python SDK or CCXT).
Always reference the local docs before making assumptions about request/response formats.

## trade[XYZ] Reference

XYZ is a HIP-3 DEX on Hyperliquid for equities, indices, commodities, and FX perpetuals.
Documentation is in `docs/xyz/`. Read `docs/xyz/INDEX.md` for structure.

Key files:
- `00-architecture.md` - XYZ architecture overview and HIP-3 metadata
- `01-perp-overview.md` - Perp mechanics overview
- `02-oracle-price.md` through `04-external-price.md` - Price feeds and oracle design
- `05-funding.md` - Funding rate mechanics
- `06-fees.md` - Fee structure
- `07-commodities.md` through `10-fx.md` - Asset-specific specs (commodities, equity indices, stocks, FX)
- `11-specification-index.md` - Full specification reference for all listed assets
- `12-holiday-closures.md` - Market closure schedules
- `13-roll-schedules.md` - Futures roll schedules
- `14-margin-modes.md` - Cross/isolated margin
- `15-liquidation-mechanics.md` - Liquidation logic
- `16-auto-deleveraging.md` - ADL mechanics

XYZ assets use the `xyz:` prefix in the Hyperliquid API (e.g. `xyz:GOLD`, `xyz:SPX`).
XYZ markets are accessed via the same Hyperliquid API endpoints documented above.
