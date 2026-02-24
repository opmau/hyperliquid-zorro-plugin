<!-- Source: https://docs.trade.xyz/perp-mechanics/fees -->

### [hashtag](#trading-and-fees) **Trading and Fees**

**Standard Fees**: Standard fees for all HIP-3 assets are currently [2x the usual fees on validator-operated perp marketsarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/hyperliquid-improvement-proposals-hips/hip-3-builder-deployed-perpetuals), with 50% of those fees going to Hyperliquid and 50% to trade[XYZ].

**Growth Mode**: Growth mode may be enabled on certain assets. With growth mode, all-in fees are reduced by ≥90% from the standard fee rate, and both rebates and volume-contribution credits are also reduced by ≥90%.

The following are not eligible for growth mode:

* Crypto perps against any collateral
* Perps on crypto indexes, ETFs, or other baskets of crypto assets
* Perps on mathematical combinations including crypto assets (e.g., BTC + X)
* Perps on vehicles or wrappers that hold primarily crypto assets (e.g. MSTR)
* Perps tracking gold, because PAXG-USDC already tracks gold price (e.g. Gold)

**Fee Tiers**: Fee tiers are based on rolling 14-day volume, assessed daily (UTC). One tier applies across all perps, HIP-3 perps, and spot. Maker rebates are paid continuously per trade.

Hyperliquid staking discounts apply. You can read more about fees in [Hyperliquid's documentationarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/trading/fees).

---

### [hashtag](#hip-3-fee-tiers-growth-mode) HIP-3 Fee Tiers — Growth Mode

Base Rate

Diamond

Platinum

Gold

Silver

Bronze

Wood

Tier

14d weighted volume ($)

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

0

—

0.0090%

0.0030%

0.00540%

0.00180%

0.00630%

0.00210%

0.00720%

0.00240%

0.00766%

0.00256%

0.00810%

0.00270%

0.00856%

0.00286%

1

>$5M

0.0080%

0.0024%

0.00480%

0.00144%

0.00560%

0.00168%

0.00640%

0.00192%

0.00680%

0.00204%

0.00720%

0.00216%

0.00760%

0.00228%

2

>$25M

0.0070%

0.0016%

0.00420%

0.00096%

0.00490%

0.00112%

0.00560%

0.00128%

0.00596%

0.00136%

0.00630%

0.00144%

0.00666%

0.00152%

3

>$100M

0.0060%

0.0008%

0.00360%

0.00048%

0.00420%

0.00056%

0.00480%

0.00064%

0.00510%

0.00068%

0.00540%

0.00072%

0.00570%

0.00076%

4

>$500M

0.0056%

0.0000%

0.00336%

0.00000%

0.00392%

0.00000%

0.00448%

0.00000%

0.00476%

0.00000%

0.00504%

0.00000%

0.00532%

0.00000%

5

>$2B

0.0052%

0.0000%

0.00312%

0.00000%

0.00364%

0.00000%

0.00416%

0.00000%

0.00442%

0.00000%

0.00000%

0.00000%

0.00494%

0.00000%

6

>$7B

0.0048%

0.0000%

0.00288%

0.00000%

0.00336%

0.00000%

0.00384%

0.00000%

0.00408%

0.00000%

0.00000%

0.00000%

0.00456%

0.00000%

### [hashtag](#hip-3-fee-tiers-standard) HIP-3 Fee Tiers — Standard

Base Rate

Diamond

Platinum

Gold

Silver

Bronze

Wood

Tier

14d weighted volume ($)

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

Taker

Maker

0

—

0.090%

0.030%

0.0540%

0.0180%

0.0630%

0.0210%

0.0720%

0.0240%

0.0766%

0.0256%

0.0810%

0.0270%

0.0856%

0.0286%

1

>$5M

0.080%

0.024%

0.0480%

0.0144%

0.0560%

0.0168%

0.0640%

0.0192%

0.0680%

0.0204%

0.0720%

0.0216%

0.0760%

0.0228%

2

>$25M

0.070%

0.016%

0.0420%

0.0096%

0.0490%

0.0112%

0.0560%

0.0128%

0.0596%

0.0136%

0.0630%

0.0144%

0.0666%

0.0152%

3

>$100M

0.060%

0.008%

0.0360%

0.0048%

0.0420%

0.0056%

0.0480%

0.0064%

0.0510%

0.0068%

0.0054%

0.0072%

0.0570%

0.0076%

4

>$500M

0.056%

0.000%

0.0336%

0.0000%

0.0392%

0.0000%

0.0448%

0.0000%

0.0476%

0.0000%

0.0504%

0.0000%

0.0532%

0.0000%

5

>$2B

0.052%

0.000%

0.0312%

0.0000%

0.0364%

0.0000%

0.0416%

0.0000%

0.0442%

0.0000%

0.0468%

0.0000%

0.0494%

0.0000%

6

>$7B

0.048%

0.000%

0.0288%

0.0000%

0.0336%

0.0000%

0.0384%

0.0000%

0.0408%

0.0000%

0.0432%

0.0000%

0.0456%

0.0000%

[PreviousFundingchevron-left](/perp-mechanics/funding)[NextCommoditieschevron-right](/asset-directory/commodities)

Last updated 3 days ago

Was this helpful?