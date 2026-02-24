<!-- Source: https://docs.trade.xyz/perp-mechanics/external-price -->

The *External Price* is a separate price, included alongside oracle and mark in Relayer updates, which is equal to the last externally-derived fair price. When external markets are open, the external price is equal to the oracle price. When external markets are closed, the external price remains fixed at the external close price while the oracle advances via its internal pricing mechanism.

A fundamental unlock of XYZ markets is the facilitation of 24/7 price discovery. When external markets close, XYZ traders continue to buy, sell, or otherwise speculate on the asset's fair price. As a result, XYZ is the primary venue for price discovery over weekends and holidays.

To provide a fair and safe venue for weekend speculation, price discovery is limited within a specific range around the external price. By restricting the extent to which price moves can occur, market makers and speculators can more confidently participate in weekend price discovery. As long as the trader maintains a liquidation threshold outside of the discovery bound, they will be rewarded for correctly speculating on the assets price upon market open.

### [hashtag](#how-discovery-bounds-work) How Discovery Bounds Work

Discovery Bounds are leverage-based limits that restrict how far the mark price may deviate from the external oracle price. Enabled by default on all markets, these bounds provide clearly defined price levels within which market makers and other market participants can confidently participate, without risk of liquidation. Their purpose is to prevent price movements that exceed what may reasonably constitute true price discovery.

#### [hashtag](#bound-range) Bound Range

The mark price is restricted to move within `1 / max leverage` of the last externally derived oracle price. Discovery Bounds are enforced 24/7, but are particularly relevant during internal pricing sessions (e.g., weekends or holidays), in which the external reference price may remain unchanged for extended periods. Market participants can use this information to manage their risk and margin accordingly.

Market

Max Leverage

Bound Range

XYZ100

25x

±4%

Commodities

20x

±5%

Single Name Equities

10x

±10%

FX (EUR, JPY)

50x

±2%

#### [hashtag](#example) Example

If the Silver oracle price is $75 at Friday’s 17:00 ET close, given 20× max leverage, the applicable Discovery Bound is ±5%. As a result, both the mark price and oracle price are restricted to the range $71.25 – $78.75 until external pricing resumes on Sunday at 18:00 ET.

If Silver is trading 8% above Friday’s close and a trader believes the move does not reflect fair value, they may enter a short position with confidence that the position will be rewarded if that view is validated once external pricing resumes. Importantly, if a trader’s liquidation price lies outside the active price bounds, their position cannot be liquidated while those bounds are in effect.

[PreviousMark Pricechevron-left](/perp-mechanics/mark-price)[NextFundingchevron-right](/perp-mechanics/funding)

Last updated 3 days ago

Was this helpful?