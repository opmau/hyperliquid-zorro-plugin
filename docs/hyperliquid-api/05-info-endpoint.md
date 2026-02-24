<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/info-endpoint -->

### [hashtag](#pagination) Pagination

Responses that take a time range will only return 500 elements or distinct blocks of data. To query larger ranges, use the last returned timestamp as the next `startTime` for pagination.

### [hashtag](#perpetuals-vs-spot) Perpetuals vs Spot

The endpoints in this section as well as websocket subscriptions work for both Perpetuals and Spot. For perpetuals `coin` is the name returned in the `meta` response. For Spot, coin should be `PURR/USDC` for PURR, and `@{index}` e.g. `@1` for all other spot tokens where index is the index of the spot pair in the `universe` field of the `spotMeta` response. For example, the spot index for HYPE on mainnet is `@107` because the token index of HYPE is 150 and the spot pair `@107` has tokens `[150, 0]`. Note that some assets may be remapped on user interfaces. For example, `BTC/USDC` on app.hyperliquid.xyz corresponds to `UBTC/USDC` on mainnet HyperCore. The L1 name on the [token details pagearrow-up-right](https://app.hyperliquid.xyz/explorer/token/0x8f254b963e8468305d409b33aa137c67) can be used to detect remappings.

### [hashtag](#user-address) User address

To query the account data associated with a master or sub-account, you must pass in the actual address of that account. A common pitfall is to use an agent wallet's address which leads to an empty result.

## [hashtag](#retrieve-mids-for-all-coins) Retrieve mids for all coins

`POST` `https://api.hyperliquid.xyz/info`

Note that if the book is empty, the last trade price will be used as a fallback

#### [hashtag](#headers) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body) Request Body

Name

Type

Description

type\*

String

"allMids"

dex

String

Perp dex name. Defaults to the empty string which represents the first perp dex. Spot mids are only included with the first perp dex..

200: OK Successful Response

## [hashtag](#retrieve-a-users-open-orders) Retrieve a user's open orders

`POST` `https://api.hyperliquid.xyz/info`

See a user's open orders

#### [hashtag](#headers-1) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-1) Request Body

Name

Type

Description

type\*

String

"openOrders"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

dex

String

Perp dex name. Defaults to the empty string which represents the first perp dex. Spot open orders are only included with the first perp dex.

200: OK Successful R

## [hashtag](#retrieve-a-users-open-orders-with-additional-frontend-info) Retrieve a user's open orders with additional frontend info

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-2) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-2) Request Body

Name

Type

Description

type\*

String

"frontendOpenOrders"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

dex

String

Perp dex name. Defaults to the empty string which represents the first perp dex. Spot open orders are only included with the first perp dex.

200: OK

## [hashtag](#retrieve-a-users-fills) Retrieve a user's fills

`POST` `https://api.hyperliquid.xyz/info`

Returns at most 2000 most recent fills

#### [hashtag](#headers-3) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-3) Request Body

Name

Type

Description

type\*

String

"userFills"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

aggregateByTime

bool

When true, partial fills are combined when a crossing order gets filled by multiple different resting orders. Resting orders filled by multiple crossing orders are only aggregated if in the same block.

200: OK

## [hashtag](#retrieve-a-users-fills-by-time) Retrieve a user's fills by time

`POST` `https://api.hyperliquid.xyz/info`

Returns at most 2000 fills per response and only the 10000 most recent fills are available

#### [hashtag](#headers-4) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-4) Request Body

Name

Type

Description

type\*

String

userFillsByTime

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

startTime\*

int

Start time in milliseconds, inclusive

endTime

int

End time in milliseconds, inclusive. Defaults to current time.

aggregateByTime

bool

When true, partial fills are combined when a crossing order gets filled by multiple different resting orders. Resting orders filled by multiple crossing orders are only aggregated if in the same block.

200: OK Number of fills is limited to 2000

## [hashtag](#query-user-rate-limits) Query user rate limits

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#request-body-5) Request Body

Name

Type

Description

user

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000

type

String

userRateLimit

200: OK A successful response

## [hashtag](#query-order-status-by-oid-or-cloid) Query order status by oid or cloid

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#request-body-6) Request Body

Name

Type

Description

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

type\*

String

"orderStatus"

oid\*

uint64 or string

Either u64 representing the order id or 16-byte hex string representing the client order id

The <status> string returned has the following possible values:

Order status

Explanation

open

Placed successfully

filled

Filled

canceled

Canceled by user

triggered

Trigger order triggered

rejected

Rejected at time of placement

marginCanceled

Canceled because insufficient margin to fill

vaultWithdrawalCanceled

Vaults only. Canceled due to a user's withdrawal from vault

openInterestCapCanceled

Canceled due to order being too aggressive when open interest was at cap

selfTradeCanceled

Canceled due to self-trade prevention

reduceOnlyCanceled

Canceled reduced-only order that does not reduce position

siblingFilledCanceled

TP/SL only. Canceled due to sibling ordering being filled

delistedCanceled

Canceled due to asset delisting

liquidatedCanceled

Canceled due to liquidation

scheduledCancel

API only. Canceled due to exceeding scheduled cancel deadline (dead man's switch)

tickRejected

Rejected due to invalid tick price

minTradeNtlRejected

Rejected due to order notional below minimum

perpMarginRejected

Rejected due to insufficient margin

reduceOnlyRejected

Rejected due to reduce only

badAloPxRejected

Rejected due to post-only immediate match

iocCancelRejected

Rejected due to IOC not able to match

badTriggerPxRejected

Rejected due to invalid TP/SL price

marketOrderNoLiquidityRejected

Rejected due to lack of liquidity for market order

positionIncreaseAtOpenInterestCapRejected

Rejected due to open interest cap

positionFlipAtOpenInterestCapRejected

Rejected due to open interest cap

tooAggressiveAtOpenInterestCapRejected

Rejected due to price too aggressive at open interest cap

openInterestIncreaseRejected

Rejected due to open interest cap

insufficientSpotBalanceRejected

Rejected due to insufficient spot balance

oracleRejected

Rejected due to price too far from oracle

perpMaxPositionRejected

Rejected due to exceeding margin tier limit at current leverage

200: OK A successful response

200: OK Missing Order

## [hashtag](#l2-book-snapshot) L2 book snapshot

`POST` `https://api.hyperliquid.xyz/info`

Returns at most 20 levels per side

**Headers**

Name

Value

Content-Type\*

"application/json"

**Body**

Name

Type

Description

type\*

String

"l2Book"

coin\*

String

coin

nSigFigs

Number

Optional field to aggregate levels to `nSigFigs` significant figures. Valid values are 2, 3, 4, 5, and `null`, which means full precision

mantissa

Number

Optional field to aggregate levels. This field is only allowed if nSigFigs is 5. Accepts values of 1, 2 or 5.

**Response**

200: OK

## [hashtag](#candle-snapshot) Candle snapshot

`POST` `https://api.hyperliquid.xyz/info`

Only the most recent 5000 candles are available

Supported intervals: "1m", "3m", "5m", "15m", "30m", "1h", "2h", "4h", "8h", "12h", "1d", "3d", "1w", "1M"

**Headers**

Name

Value

Content-Type\*

"application/json"

**Body**

Name

Type

Description

type\*

String

"candleSnapshot"

req\*

Object

{"coin": <coin>, "interval": "15m", "startTime": <epoch millis>, "endTime": <epoch millis>}
For HIP-3, you need to prefix with the dex name, e.g. "xyz:XYZ100"

**Response**

200: OK

## [hashtag](#check-builder-fee-approval) Check builder fee approval

`POST` `https://api.hyperliquid.xyz/info`

**Headers**

Name

Value

Content-Type\*

"application/json"

**Body**

Name

Type

Description

type\*

String

"maxBuilderFee"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

builder\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

**Response**

200: OK

## [hashtag](#retrieve-a-users-historical-orders) Retrieve a user's historical orders

`POST` `https://api.hyperliquid.xyz/info`

Returns at most 2000 most recent historical orders

#### [hashtag](#headers-5) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-7) Request Body

Name

Type

Description

type\*

String

"historicalOrders"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#retrieve-a-users-twap-slice-fills) Retrieve a user's TWAP slice fills

`POST` `https://api.hyperliquid.xyz/info`

Returns at most 2000 most recent TWAP slice fills

#### [hashtag](#headers-6) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-8) Request Body

Name

Type

Description

type\*

String

"userTwapSliceFills"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#retrieve-a-users-subaccounts) Retrieve a user's subaccounts

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-7) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-9) Request Body

Name

Type

Description

type\*

String

"subAccounts"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#retrieve-details-for-a-vault) Retrieve details for a vault

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-8) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-10) Request Body

Name

Type

Description

type\*

String

"vaultDetails"

vaultAddress\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

user

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#retrieve-a-users-vault-deposits) Retrieve a user's vault deposits

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-9) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-11) Request Body

Name

Type

Description

type\*

String

"userVaultEquities"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-role) Query a user's role

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-10) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-12) Request Body

Name

Type

Description

type\*

String

"userRole"

user\*

String

Address in 42-character hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

User

Agent

Vault

Subaccount

Missing

## [hashtag](#query-a-users-portfolio) Query a user's portfolio

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-11) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-13) Request Body

Name

Type

Description

type\*

String

"portfolio"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-referral-information) Query a user's referral information

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-12) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-14) Request Body

Name

Type

Description

type\*

String

"referral"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

Note that rewardHistory is for legacy rewards. Claimed rewards are now returned in nonFundingLedgerUpdate

## [hashtag](#query-a-users-fees) Query a user's fees

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-13) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-15) Request Body

Name

Type

Description

type\*

String

"userFees"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-staking-delegations) Query a user's staking delegations

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-14) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-16) Request Body

Name

Type

Description

type\*

String

"delegations"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-staking-summary) Query a user's staking summary

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-15) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-17) Request Body

Name

Type

Description

type\*

String

"delegatorSummary"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-staking-history) Query a user's staking history

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-16) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-18) Request Body

Name

Type

Description

type\*

String

"delegatorHistory"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-staking-rewards) Query a user's staking rewards

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-17) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-19) Request Body

Name

Type

Description

type\*

String

"delegatorRewards"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-a-users-hip-3-dex-abstraction-state) Query a user's HIP-3 DEX abstraction state

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-18) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-20) Request Body

Name

Type

Description

type\*

String

"userDexAbstraction"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-aligned-quote-token-status) Query aligned quote token status

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-19) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-21) Request Body

Name

Type

Description

type\*

String

"alignedQuoteTokenInfo"

token\*

Number

token index

200: OK

## [hashtag](#query-borrow-lend-user-state) Query borrow/lend user state

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-20) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-22) Request Body

Name

Type

Description

type\*

String

"borrowLendUserState"

user\*

String

hexadecimal format; e.g. 0x0000000000000000000000000000000000000000.

200: OK

## [hashtag](#query-borrow-lend-reserve-state) Query borrow/lend reserve state

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-21) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-23) Request Body

Name

Type

Description

type\*

String

"borrowLendReserveState"

token\*

Number

token index

200: OK

## [hashtag](#query-all-borrow-lend-reserve-states) Query all borrow/lend reserve states

`POST` `https://api.hyperliquid.xyz/info`

#### [hashtag](#headers-22) Headers

Name

Type

Description

Content-Type\*

String

"application/json"

#### [hashtag](#request-body-24) Request Body

Name

Type

Description

type\*

String

"allBorrowLendReserveStates"

200: OK

[PreviousNonces and API walletschevron-left](/hyperliquid-docs/for-developers/api/nonces-and-api-wallets)[NextPerpetualschevron-right](/hyperliquid-docs/for-developers/api/info-endpoint/perpetuals)

Last updated 24 days ago