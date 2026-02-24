<!-- Source: https://docs.trade.xyz/risk-and-margining/liquidation-mechanics -->

As a HIP-3 Market, we inherit our liquidation and auto-deleveraging mechanisms from Hypercore. You should review Hyperliquid's documentation in detail to better understand how [liquidationsarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/trading/liquidations) and [auto-deleveragingarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/trading/auto-deleveraging) works on all Hyperliquid markets. There is currently no backstop liquidator for XYZ markets.

**I**MPORTANT NOTE: XYZ assets are not backstopped by the Liquidator Vault. In the event of an account's equity dropping below the maintenance margin, [auto-deleveraging](/risk-and-margining/auto-deleveraging) is expected. We also use a different mark price than Hyperliquid.

---

#### [hashtag](#overview) How Liquidations Work on Hyperliquid

A liquidation event occurs when a trader's positions move against them to the point where the account equity falls below the maintenance margin.

When account equity drops below the maintenance margin, the positions are first attempted to be entirely closed by sending market orders to the book. These orders are the full size of the position, although they may only be partially closed depending on market liquidity. After a position is partially or fully closed, the trader retains any remaining collateral if maintenance margin requirements have been met.

If the account equity drops below 2/3 of the maintenance margin without successful liquidation through market orders on the book, a backstop liquidation happens through the Hyperliquid liquidator vault (only applicable for Hyperliquid-native perps). During backstop liquidation, the maintenance margin is not returned to the user. You can read more about the Liquidator Vault in Hyperliquid's documentation.

In Hyperliquid's system, when a cross position is backstop liquidated, the trader's cross positions and cross margin are all transferred to the liquidator. Cross margin and other positions are not impacted by isolated positions that are backstop liquidated.

Liquidations use the mark price. Hyperliquid-native perps specifically use Hyperliquid's mark price, while XYZ assets use the XYZ Relayer Mark price.

---

#### [hashtag](#computing-liquidation-price) Computing Liquidation Price

When entering a trade, an estimated liquidation price is shown. This estimation may be inaccurate compared to the position's estimated liquidation price due to changing liquidity on the book. Even after a position is opened, a liquidation price may be shown which is not the actual liquidation price due to funding payments or changes in unrealized PnL in other positions (for cross margin positions).

The actual liquidation price is independent on the leverage set for cross margin positions. However, the liquidation price does depend on leverage set for isolated margin positions, because the amount of isolated margin allocated depends on the initial margin set.

The precise formula for the liquidation price of a position is

`liq_price = price - side * margin_available / position_size / (1 - l * side)`

where

`l = 1 / MAINTENANCE_LEVERAGE` .

For assets with margin tiers, maintenance leverage depends on the unique margin tier corresponding to the position value at the liquidation price.

[PreviousMargin Modeschevron-left](/risk-and-margining/margin-modes)[NextAuto-Deleveragingchevron-right](/risk-and-margining/auto-deleveraging)

Last updated 3 days ago

Was this helpful?