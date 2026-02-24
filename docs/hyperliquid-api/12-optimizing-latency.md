<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/optimizing-latency -->

The following optimizations may help latency-sensitive traders:

1. Run a non-validating node against a reliable peer, such as Hyper Foundation non-validator.
2. Run node with `--disable-output-file-buffering` to get outputs as soon as blocks are executed
3. Run node with sufficient machines specs, at least 32 logical cores and 500 MB/s disk throughput. Increasing cores can reduce latency because blocks will be faster to execute.
4. Construct book and other exchange state locally using outputs from node, which has faster and more granular data than the API. See [https://github.com/hyperliquid-dex/order\_book\_serverarrow-up-right](https://github.com/hyperliquid-dex/order_book_server) for an example on how to build an order book on the same machine that is running a node.
5. `--batch-by-block`  on the node will wait until the end of the block to write the data. The example order book server above uses this to simplify logic, but a further optimization could include turning the flag off and inferring block boundaries otherwise.
6. Consider canceling pending orders by invalidating the nonce instead of spamming the cancelation action. This will save on user rate limits and have a guaranteed success rate if the nonce invalidation transaction lands first. A cheap transaction to use for nonce invalidation is `noop` with no additional fields.

[PreviousActivation gas feechevron-left](/hyperliquid-docs/for-developers/api/activation-gas-fee)[NextBridge2chevron-right](/hyperliquid-docs/for-developers/api/bridge2)

Last updated 5 months ago