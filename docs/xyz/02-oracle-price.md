<!-- Source: https://docs.trade.xyz/perp-mechanics/oracle-price -->

The oracle serves two critical functions:

1. as the reference price for funding.
2. as a direct input to mark price calculation.

The key unlock for equity perpetuals is extending the oracle to operate 24/7, beyond external trading sessions of the underlying asset. To achieve this, the system prioritizes external pricing and employs a robust internal mechanism when external data is unavailable.

### [hashtag](#external-pricing) External Pricing

The Relayer consumes price data for the underlying assets from a robust set of venues, markets, and institutional data providers. When external markets are open, the externally-derived fair price is transmitted as the oracle price in relayer updates.

Instrument-specific details for external price derivation can be found in their respective section.

### [hashtag](#internal-pricing) Internal Pricing

When external inputs are unavailable, the oracle advances via a continuous-time exponentially weighted moving average that incrementally adjusts the previous oracle price by a fraction of the impact price difference.

With oracle price SSS the impact price difference (IPDIPDIPD) is defined as:

IPD=max(PimpactBid−S,0)−max(S−PimpactAsk,0)IPD = max(P\_{impactBid} - S, 0) - max(S-P\_{impactAsk}, 0)IPD=max(PimpactBid​−S,0)−max(S−PimpactAsk​,0)

where the impact bid price (PimpactBidP\_{impactBid}PimpactBid​) and impact ask price PimpactAskP\_{impactAsk}PimpactAsk​ are the average execution prices to trade a configured impact notional amount on the bid and ask sides of the orderbook, respectively. If insufficient depth exists on a side, that side's contribution is set to zero.

The oracle is robust to irregular updates and market halts. It uses a time constant τ=1\tau = 1τ=1 hours and updates as:

St=βt St−+(1−βt) xt,βt=exp⁡ ⁣(−Δt∗τ),xt=St−+IPDt
S\_t = \beta\_t\, S\_{t^-} + (1-\beta\_t)\, x\_t,
\qquad
\beta\_t = \exp\!\left(-\frac{\Delta t^\ast}{\tau}\right),
\qquad
x\_t = S\_{t^-} + \mathrm{IPD}\_tSt​=βt​St−​+(1−βt​)xt​,βt​=exp(−τΔt∗​),xt​=St−​+IPDt​

where Δt∗=min⁡ ⁣(Δt, c τ)\Delta t^\ast = \min\!\bigl(\Delta t,\, c\,\tau\bigr)Δt∗=min(Δt,cτ), Δt=t−tprev\Delta t = t - t\_{\mathrm{prev}}Δt=t−tprev​, and c=0.1c = 0.1c=0.1 (so 1−βt≤1−e−0.1≈9.5%1-\beta\_t \le 1 - e^{-0.1} \approx 9.5\%1−βt​≤1−e−0.1≈9.5%).

> **Note:** This provides the more general form of the continuous-time EMA. Component (2) of the mark price samples the basis (xt=St−Pmid,tx\_t = S\_t - P\_{mid,t}xt​=St​−Pmid,t​) with a time constant τ=150\tau = 150τ=150 seconds.

When external data becomes unavailable, the internal mechanism initializes from the last available external price. When external inputs resume, the oracle reverts to the externally derived price on the next tick.

[PreviousOverviewchevron-left](/perp-mechanics/overview)[NextMark Pricechevron-right](/perp-mechanics/mark-price)

Last updated 4 days ago

Was this helpful?