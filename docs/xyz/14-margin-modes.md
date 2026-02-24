<!-- Source: https://docs.trade.xyz/risk-and-margining/margin-modes -->

XYZ inherits its margining mechanisms from Hypercore.

#### [hashtag](#margin-mode) Margin Mode

*Normal Isolated Margin:* allows an asset's collateral to be constrained to that asset. Liquidations in that asset do not affect other isolated positions or cross positions. Similarly, cross liquidations or other isolated liquidations do not affect the original isolated position. Currently, *Normal Isolated Margin* is enabled for XYZ100, Gold, and Silver.

*Strict Isolated Margin*: functions the same as isolated margin, with the additional constraint that margin cannot be removed. Margin is proportionally removed as the position is closed. Currently, all markets outside of XYZ100, Gold, and Silver are set to *Strict Isolated Margin.*

#### [hashtag](#initial-margin-and-leverage) Initial Margin and Leverage

Leverage can be set by a user to any integer between 1 and the max leverage. Max leverage depends on the asset.

The margin required to open a position is `position_size * mark_price / leverage`. *Normal Isolated Margin* positions support adding and removing margin after opening the position. Isolated positions will apply unrealized pnl as additional margin for the open position. The leverage of an existing position can be increased without closing the position. Leverage is only checked upon opening a position. Afterwards, the user is responsible for monitoring the leverage usage to avoid liquidation. Possible actions to take on positions with negative unrealized pnl include partially or fully closing the position or adding margin (if isolated).

**Maintenance Margin:** the minimum collateral you must keep to avoid liquidation. If your account equity (collateral + PnL) falls below the maintenance level, you'll be liquidated. The maintenance level is currently determined by multiplying the maintenance margin by the total open notional position. The maintenance margin is set to half of the initial margin at max leverage for that asset, which varies from 3-40x. In other words, the maintenance margin ranges from 1.25% (at 40x max leverage) to 16.7 (at 3x max leverage).

[PreviousRoll Scheduleschevron-left](/consolidated-resources/roll-schedules)[NextLiquidation Mechanicschevron-right](/risk-and-margining/liquidation-mechanics)

Last updated 4 days ago

Was this helpful?