<!-- Source: https://docs.trade.xyz/asset-directory/commodities -->

### [hashtag](#external-coverage) External Coverage

The Relayer derives external prices for commodities 23/5, from Sunday 6:00 PM ET to Friday 5:00 PM ET, with daily gaps from 5:00 PM to 6:00 PM ET.

### [hashtag](#external-price-derivation) External Price Derivation

#### [hashtag](#precious-metals) Precious Metals

Liquid spot markets are available for Gold, Silver, Platinum, and Palladium. These markets are used to source an external price for the respective asset.

#### [hashtag](#industrial-metals-and-energy-commodities) Industrial Metals & Energy Commodities

Industrial metals and energy commodities do not have "spot" markets as other assets do. The "underlying" price of these assets often varies on a variety of factors, including geography.

Thus, to derive external prices for these assets, the relayer uses an industry standard approach - referencing a fixed set of designated futures contracts. The underlying contract is rolled from the 5th to the 10th business day of the month, at predefined timestamps which correspond to internal pricing sessions, such that:

* on the 5th business day, the external price is 100% front contract
* on the 6th business day, the external price is 80% front contract, 20% next contract
* on the 7th business day, the external price is 60% front contract, 40% next contract
* on the 8th business day, the external price is 40% front contract, 60% next contract
* on the 9th business day, the external price is 20% front contract, 80% next contract
* on the 10th business day, the external price is 100% next contract

Please see [Roll Schedules](/consolidated-resources/roll-schedules) to see the specific schedule for markets that reference futures-based price feeds (e.g., CL, Copper, Natural Gas).

### [hashtag](#designated-contract-schedule) Designated Contract Schedule

The following schedule defines the active contract at the start of each month for each commodity

Asset

Underlying

Jan

Feb

Mar

Apr

May

Jun

Jul

Aug

Sep

Oct

Nov

Dec

COPPER

1lb of Copper

H

H

K

K

N

N

U

U

Z

Z

Z

H

CL

1 barrel of WTI Light Sweet Crude Oil

G

H

J

K

M

N

Q

U

V

X

Z

F

NATGAS

1 MMBtu of Henry Hub Natural Gas

G

H

J

K

M

N

Q

U

V

X

Z

F

### [hashtag](#data-source) **Data Source**

See the [Specification Index](/consolidated-resources/specification-index) page for pricing referencing sources.

[PreviousFeeschevron-left](/perp-mechanics/fees)[NextEquity Indiceschevron-right](/asset-directory/equity-indices)

Last updated 2 days ago

Was this helpful?