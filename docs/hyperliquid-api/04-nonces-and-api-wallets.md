<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/nonces-and-api-wallets -->

### [hashtag](#background) Background

A decentralized L1 must prevent replay attacks. When a user signs a USDC transfer transaction, the receiver cannot broadcast it multiple times to drain the sender's wallet. To solve this Ethereum stores a "nonce" for each address, which is a number that starts at 0. Each transaction must use exactly "nonce + 1" to be included.

### [hashtag](#api-wallets) API wallets

These are also known as `agent wallets` in the docs. A master account can approve API wallets to sign on behalf of the master account or any of the sub-accounts.

Note that API wallets are only used to sign. To query the account data associated with a master or sub-account, you must pass in the actual address of that account. A common pitfall is to use the agent wallet which leads to an empty result.

### [hashtag](#api-wallet-pruning) API wallet pruning

API wallets and their associated nonce state may be pruned in the following cases:

1. The wallet is deregistered. This happens to an existing unnamed API Wallet when an ApproveAgent action is sent to register a new unnamed API Wallet. This also happens to an existing named API Wallet when an ApproveAgent action is sent with a matching name.
2. The wallet expires.
3. The account that registered the agent no longer has funds.

**Important:** for those using API wallets programmatically, it is **strongly** suggested to not reuse their addresses. Once an agent is deregistered, its used nonce state may be pruned. Generate a new agent wallet on future use to avoid unexpected behavior. For example, previously signed actions can be replayed once the nonce set is pruned.

### [hashtag](#hyperliquid-nonces) Hyperliquid nonces

Ethereum's design does not work for an onchain order book. A market making strategy can send thousands of orders and cancels in a second. Requiring a precise ordering of inclusion on the blockchain will break any strategy.

On Hyperliquid, the 100 highest nonces are stored per address. Every new transaction must have nonce larger than the smallest nonce in this set and also never have been used before. Nonces are tracked per signer, which is the user address if signed with private key of the address, or the agent address if signed with an API wallet.

Nonces must be within `(T - 2 days, T + 1 day)`, where `T` is the unix millisecond timestamp on the block of the transaction.

The following steps may help port over an automated strategy from a centralized exchange:

1. Use a API wallet per trading process. Note that nonces are stored per signer (i.e. private key), so separate subaccounts signed by the same API wallet will share the nonce tracker of the API wallet. It's recommended to use separate API wallets for different subaccounts.
2. In each trading process, have a task that periodically batches order and cancel requests every 0.1 seconds. It is recommended to batch IOC and GTC orders separately from ALO orders because ALO order-only batches are prioritized by the validators.
3. The trading logic tasks send orders and cancels to the batching task.
4. For each batch of orders or cancels, fetch and increment an atomic counter that ensures a unique nonce for the address. The atomic counter can be fast-forwarded to current unix milliseconds if needed.

This structure is robust to out-of-order transactions within 2 seconds, which should be sufficient for an automated strategy geographically near an API server.

### [hashtag](#suggestions-for-subaccount-and-vault-users) Suggestions for subaccount and vault users

Note that nonces are stored per signer, which is the address of the private key used to sign the transaction. Therefore, it's recommended that each trading process or frontend session use a separate private key for signing. In particular, a single API wallet signing for a user, vault, or subaccount all share the same nonce set.

If users want to use multiple subaccounts in parallel, it would easier to generate two separate API wallets under the master account, and use one API wallet for each subaccount. This avoids collisions between the nonce set used by each subaccount.

[PreviousTick and lot sizechevron-left](/hyperliquid-docs/for-developers/api/tick-and-lot-size)[NextInfo endpointchevron-right](/hyperliquid-docs/for-developers/api/info-endpoint)

Last updated 9 months ago