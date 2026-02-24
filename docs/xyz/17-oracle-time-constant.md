<!-- Source: https://docs.trade.xyz/changelog/oracle-time-constant-update -->

### [hashtag](#november-21-2025-oracle-time-constant-update) *November 21, 2025 - Oracle Time Constant Update*

We reduced the internal oracle time constant from 8 hours to 1 hour for equity perpetuals.

Effect in Practice

* Faster reflection of price discovery - during closed sessions, the oracle now tracks orderflow with a shorter time constant, reducing the gap between “traded price” and oracle.
* Lower funding drag on genuine moves - traders who are directionally right on gap moves keep more of the move (87% vs 10%); shorts who sit through those gaps receive much less funding compensation.
* Same safety properties - the 1 / max\_leverage band on mark vs last external spot still limits single-tick jumps and weekend bands in mark.

[PreviousAuto-Deleveragingchevron-left](/risk-and-margining/auto-deleveraging)[NextFunding Rate Formula Updatechevron-right](/changelog/funding-rate-formula-update)

Last updated 4 days ago

Was this helpful?