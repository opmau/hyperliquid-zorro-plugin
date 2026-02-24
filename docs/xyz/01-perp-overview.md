<!-- Source: https://docs.trade.xyz/perp-mechanics/overview -->

A perpetual futures contract (or "perp") is a derivative product that allows traders to speculate on an asset's price. Unlike traditional futures, which settle on a fixed date, positions can be held indefinitely. Perps rely on a funding rate mechanism to keep the contract price aligned with the underlying spot price.

XYZ perpetuals are linear contracts, margined and settled in USDC. The oracle is denominated in USD, and no USDC/USD conversion is applied. Economic exposure is therefore *quanto* with USD P&L cash-settled in USDC.

Listed on the XYZ HIP-3 deployment, XYZ perpetual's matching, order types, funding, liquidations, and auto-deleveraging are managed by HyperCore. Three components are bespoke: the *oracle price,* the *mark price,* and the *external price*.

The *Relayer* is responsible for computing oracle, mark, and external prices for each asset and broadcasting updates to HyperCore. TradeXYZ manages a distributed set of Relayer instances which transmit updates approximately every 3 seconds XYZ's oracle updater address: `0x1234567890545d1Df9EE64B35Fdd16966e08aCEC`.

[PreviousXYZ Architecturechevron-left](/)[NextOracle Pricechevron-right](/perp-mechanics/oracle-price)

Last updated 4 days ago

Was this helpful?