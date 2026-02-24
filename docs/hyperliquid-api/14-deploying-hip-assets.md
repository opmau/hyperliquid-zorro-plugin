<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/deploying-hip-1-and-hip-2-assets -->

The API for deploying HIP-1 and HIP-2 assets is a five-step process which involves sending the first 5 variants (the last two are optional) of the enum in the order below.

Copy

```
type SpotDeployAction =
  | {
      type: "spotDeploy";
      registerToken2: RegisterToken2;
    }
  | {
      type: "spotDeploy";
      userGenesis: UserGenesis;
    }
  | {
      type: "spotDeploy";
      genesis: Genesis;
    }
  | {
      type: "spotDeploy";
      registerSpot: RegisterSpot;
    }
  | {
      type: "spotDeploy";
      registerHyperliquidity: RegisterHyperliquidity;
    }
  | {
      type: "spotDeploy";
      setDeployerTradingFeeShare: SetDeployerTradingFeeShare;
    }
  | {
      type: "spotDeploy";
      enableQuoteToken: { token: number };
    }
  | {
      type: "spotDeploy";
      enableAlignedQuoteToken: { token: number };
    };

type RegisterToken2 = {
  spec: TokenSpec;
  maxGas: number;
  fullName?: string;
}

type TokenSpec = {
  name: string,
  szDecimals: number,
  weiDecimals: number,
}

/**
 * UserGenesis can be called multiple times
 * @param token - The token involved in the genesis.
 * @param userAndWei - A list of tuples of user address and genesis amount (wei).
 * @param existingTokenAndWei - A list of tuples of existing token and total genesis amount for holders of that token (wei).
 * @param blacklistUsers - A list of tuples of users and blacklist status (True if blacklist, False to remove existing blacklisted user).
 */
type UserGenesis = {
  token: number;
  userAndWei: Array<[string, string]>;
  existingTokenAndWei: Array<[number, string]>;
  blacklistUsers?: Array<[string, boolean]>;
}

/**
 * Genesis denotes the initial creation of a token with a maximum supply.
 * @param maxSupply - Checksum ensureing all calls to UserGenesis succeeded
 * @param noHyperliquidity - Set hyperliquidity balance to 0.
 */
type Genesis = {
  token: number;
  maxSupply: string;
  noHyperliquidity?: boolean;
}

/**
 * @param tokens - [base index, quote index]
 * This is also the action used to deploy pairs between an existing base and existing quote asset.
 * Deployments between pairs of existing assets follow an independent Dutch auction.
 * This auction's status is available from the `spotPairDeployAuctionStatus` info request.
 */
type RegisterSpot = {
  tokens: [number, number];
}

/**
 * @param spot - The spot index (different from base token index)
 * @param startPx - The starting price.
 * @param orderSz - The size of each order (float, not wei)
 * @param nOrders - The number of orders. If "noHyperliquidity" was set to True, then this must be 0.
 * @param nSeededLevels - The number of levels the deployer wishes to seed with usdc instead of tokens.
 */
type RegisterHyperliquidity = {
  spot: number;
  startPx: string;
  orderSz: string;
  nOrders: number;
  nSeededLevels?: number;
}

/**
 * This is an optional action that can be performed at any time after
 * RegisterToken2. While the fee share defaults to 100%, this action
 * can be resent multiple times as long as the fee share is not increasing.
 * @param token - The token
 * @param share - The deployer trading fee share. Range: ["0%", "100%"]. Examples: "0.012%", "99.4%"
 */
type SetDeployerTradingFeeShare {
  token: number;
  share: string;
}
```

[PreviousBridge2chevron-left](/hyperliquid-docs/for-developers/api/bridge2)[NextHIP-3 deployer actionschevron-right](/hyperliquid-docs/for-developers/api/hip-3-deployer-actions)

Last updated 2 months ago