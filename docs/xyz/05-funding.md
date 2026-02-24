<!-- Source: https://docs.trade.xyz/perp-mechanics/funding -->

### [hashtag](#overview) Overview

Funding rates are a mechanism designed to prevent large price disparities between the perpetual contract and the oracle price.

The funding rate is a periodic fee that is paid by one side of the contract (either long or short) to the other side. Funding is purely peer-to-peer, and no fees are collected on the payments.

The rate is calculated based on the difference between the contract price and the oracle price. If the contract price is higher than the oracle price, the premium and hence the funding rate will be positive, and the long position will pay the short position. Conversely, if the contract price is lower than the spot price, the funding rate will be negative, and the short position will pay the long position.

The funding rate is paid every hour. The funding rate is added or subtracted from the balance of contract holders at the funding interval.

---

### [hashtag](#technical-details) Technical details

**Interval**: Hourly

**Formula:**

`Funding Rate XYZ (F) = 0.5 [P + clamp (r - P, -0.0005, 0.0005)]`.

*where*

* `P` = Average Premium Index = `impact_price_difference / oracle_price`
* `r` = Interest rate
* `impact_price_difference = max(impact_bid_px - oracle_px, 0) - max(oracle_px - impact_ask_px, 0)`
* `impact_bid_px` and `impact_ask_px` = average execution prices to trade the configured impact notional on the bid and ask sides, respectively.

*Note: See the contract specifications for the impact notional used and other contract-specific parameters.*

**Payment:**

`funding_payment = position_size * oracle_price * funding_rate`.

*Note: Oracle price (not mark) is used to convert position size to notional.*

**Cap:** ±4% / hour.

---

### [hashtag](#technical-details-1) Scaling Factor Rationale

XYZ markets apply a 0.5× multiplier to Hyperliquid's baseline funding rate formula. This reduces baseline funding from a default of ~11% annualized to ~5.5%, better reflecting carry costs of traditional asset classes (typically closer to SOFR + 1-2%). This also dampens funding during weekend price discovery.

[PreviousExternal Pricechevron-left](/perp-mechanics/external-price)[NextFeeschevron-right](/perp-mechanics/fees)

Last updated 3 days ago

Was this helpful?