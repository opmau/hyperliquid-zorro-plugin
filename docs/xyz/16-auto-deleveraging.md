<!-- Source: https://docs.trade.xyz/risk-and-margining/auto-deleveraging -->

#### [hashtag](#auto-deleveraging) Auto-Deleveraging:

Auto-deleveraging (ADL) is the final safeguard in the liquidation waterfall and exists to ensure system solvency under all conditions. It occurs only when both market and backstop liquidations (for Hyperliquid-native assets) fail to close a position that has become under-collateralized.

**Remember: XYZ Assets are not protected by the Hyperliquid vault and therefore, if market liquidations fail, ADL will ensure system solvency.**

In a perpetual futures market, every long is matched by a short, and all positions are collateralized with margin from a shared collateral pool. When a trader’s account equity or an isolated position value becomes negative, the system must rebalance by closing positions on the profitable side. Otherwise, the system would accrue bad debt and violate the zero-sum constraint that total long notional equals total short notional.

When ADL is triggered, users on the profitable side are ranked and selected for forced deleveraging based on a composite index that accounts for both profitability and leverage:

`sorting_index = (mark_price / entry_price) * (notional_position / account_value)`

Users with the highest ranking index (most profitable and most leveraged) are deleveraged first. Their positions are closed at the previous mark price against the insolvent side of the market. This transfer of position value ensures that aggregate system equity remains constant and that the platform has no bad debt.

While users affected by ADL are typically in profit overall, individual ADL events can occur at unfavorable prices. The system minimizes the frequency of ADL events through conservative maintenance margin settings and by routing most liquidations through market orders or, if necessary (only applicable for Hyperliquid native-perps), the Hypercore liquidator vault. ADL is therefore a rare, last-resort settlement mechanism.

ADL exists because liquidity and solvency must be preserved in a derivatives system, even when market depth and backstop capacity are exhausted.

In practical terms, ADL is similar to a system-wide rebalancing event that closes the loop when no external liquidity remains. It is mechanical, transparent, and non-discretionary. Every ADL event can be independently verified on-chain via Hypercore’s settlement records

[PreviousLiquidation Mechanicschevron-left](/risk-and-margining/liquidation-mechanics)[NextOracle Time Constant Updatechevron-right](/changelog/oracle-time-constant-update)

Last updated 4 days ago

Was this helpful?