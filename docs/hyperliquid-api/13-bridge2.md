<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/bridge2 -->

### [hashtag](#general-information) General Information

The bridge between Hyperliquid and Arbitrum: [https://arbiscan.io/address/0x2df1c51e09aecf9cacb7bc98cb1742757f163df7arrow-up-right](https://arbiscan.io/address/0x2df1c51e09aecf9cacb7bc98cb1742757f163df7)

The bridge code: [https://github.com/hyperliquid-dex/contracts/blob/master/Bridge2.solarrow-up-right](https://github.com/hyperliquid-dex/contracts/blob/master/Bridge2.sol)

### [hashtag](#deposit) Deposit

The deposit flow for the bridge is simple. The user sends native USDC to the bridge, and it is credited to the account that sent it in less than 1 minute. The minimum deposit amount is 5 USDC. If you send an amount less than this, it will not be credited and be lost forever.

### [hashtag](#withdraw) Withdraw

The withdrawal flow requires a user wallet signature on Hyperliquid only, and no Arbitrum transaction. The withdrawal from Arbitrum is handled entirely by validators, and the funds arrive in the user wallet in 3-4 minutes. This payload for signTypedData is

Copy

```
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
#[serde(deny_unknown_fields)]
pub(crate) struct WithdrawAction3 {
    pub(crate) signature_chain_id: U256,
    pub(crate) hyperliquid_chain: Chain,
    pub(crate) destination: String,
    pub(crate) amount: String,
    pub(crate) time: u64,
}

impl Eip712 for WithdrawAction3 {
    type Error = Eip712Error;

    fn domain(&self) -> StdResult<EIP712Domain, Self::Error> {
        Ok(eip_712_domain(self.signature_chain_id))
    }

    fn type_hash() -> StdResult<[u8; 32], Self::Error> {
        Ok(eip712::make_type_hash(
            format!("{HYPERLIQUID_EIP_PREFIX}Withdraw"),
            &[
                ("hyperliquidChain".to_string(), ParamType::String),
                ("destination".to_string(), ParamType::String),
                ("amount".to_string(), ParamType::String),
                ("time".to_string(), ParamType::Uint(64)),
            ],
        ))
    }

    fn struct_hash(&self) -> StdResult<[u8; 32], Self::Error> {
        let Self { signature_chain_id: _, hyperliquid_chain, destination, amount, time } = self;
        let items = vec![
            ethers::abi::Token::Uint(Self::type_hash()?.into()),
            encode_eip712_type(hyperliquid_chain.to_string().into_token()),
            encode_eip712_type(destination.clone().into_token()),
            encode_eip712_type(amount.clone().into_token()),
            encode_eip712_type(time.into_token()),
        ];
        Ok(keccak256(encode(&items)))
    }
}
```

Example signed Hyperliquid action:

### [hashtag](#deposit-with-permit) Deposit with permit

The bridge supports depositing on behalf of another user via the `batchedDepositWithPermit`function. Example code for how the user can sign the PermitPayload

[PreviousOptimizing latencychevron-left](/hyperliquid-docs/for-developers/api/optimizing-latency)[NextDeploying HIP-1 and HIP-2 assetschevron-right](/hyperliquid-docs/for-developers/api/deploying-hip-1-and-hip-2-assets)

Last updated 10 months ago