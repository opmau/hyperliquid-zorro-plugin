<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/activation-gas-fee -->

New HyperCore accounts require 1 quote token (e.g., 1 USDC, 1 USDT, or 1 USDH) of fees for the first transaction which has the new account as destination address. This applies regardless of the asset being transfered to the new account.

Unactivated accounts cannot send CoreWriter actions. Contract deployers who do not want this one-time behavior could manually send an activation transaction to the EVM contract address on HyperCore.

[PreviousRate limits and user limitschevron-left](/hyperliquid-docs/for-developers/api/rate-limits-and-user-limits)[NextOptimizing latencychevron-right](/hyperliquid-docs/for-developers/api/optimizing-latency)

Last updated 4 months ago