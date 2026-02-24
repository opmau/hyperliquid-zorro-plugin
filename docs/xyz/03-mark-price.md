<!-- Source: https://docs.trade.xyz/perp-mechanics/mark-price -->

The mark price—used for margining, liquidations, stop/limit triggers, and unrealized P&L—is the median of three components:

1. The oracle price.
2. The sum of the oracle price and a 150-second continuous-time exponentially weighted moving average of the difference between the perpetual's mid-price and the oracle price.
3. The median of the best bid, best ask, and last trade.

In each update, the Relayer publishes components (1) and (2) for every asset. The Hyperliquid protocol computes (3) and uses the median of the three components as the mark price.

Relayer updates are clamped to ±50 bps of the current value to mitigate significant price jumps. This applies to the mark price as well as the oracle price.

[PreviousOracle Pricechevron-left](/perp-mechanics/oracle-price)[NextExternal Pricechevron-right](/perp-mechanics/external-price)

Last updated 4 days ago

Was this helpful?